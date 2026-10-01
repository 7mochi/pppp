#ifndef FOSU_SLIDER_GEOMETRY_H
#define FOSU_SLIDER_GEOMETRY_H

// Curve approximation adapted from osu-framework (2026.807.0) and osu!.
// Copyright (c) ppy Pty Ltd <contact@ppy.sh>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// The official decoder computes curve geometry with separate single-precision operations:
// the build must not contract a*b+c into fused multiply-adds (pppp's CMake sets
// -ffp-contract=off for every consumer; the upstream pragmas are compiler extensions).

#include <algorithm>
#include <cmath>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/compiler.h>

namespace fosu {
    namespace internal {

        const double kPi = 3.14159265358979323846;
        const int kMaxInt = 2147483647;
        const int kMinInt = -2147483647 - 1;

        // osu! computes curve geometry in single precision, relative to the slider head.
        typedef PathPoint CurvePoint;

        // Duration only needs the distance and final edge, not a retained path array.
        struct CurveDistance {
            double length;
            // Length removed by the Catmull optimisation (osu! SliderPath.optimisedLength). It is
            // still part of the calculated length and is front-loaded into the cumulative lengths.
            double optimised_length;
            CurvePoint last, previous;
            size_t count;
            bool first_in_segment;

            CurveDistance()
                : length(0),
                  optimised_length(0),
                  last(CurvePoint::zero()),
                  previous(CurvePoint::zero()),
                  count(0),
                  first_in_segment(false) {}

            void begin_segment() { first_in_segment = true; }

            void append(CurvePoint point) {
                if (first_in_segment && count && last == point) {
                    first_in_segment = false;
                    return;
                }
                first_in_segment = false;
                if (count) {
                    length += (point - last).length();
                }
                previous = last;
                last = point;
                ++count;
            }
        };

        inline void subdivide_bezier(Span<const CurvePoint> points, CurvePoint* left, CurvePoint* right,
                                     CurvePoint* midpoints) {
            const size_t count = points.size();
            if (count == 4) {
                const CurvePoint a = (points[0] + points[1]) * 0.5f;
                const CurvePoint b = (points[1] + points[2]) * 0.5f;
                const CurvePoint c = (points[2] + points[3]) * 0.5f;
                const CurvePoint d = (a + b) * 0.5f;
                const CurvePoint e = (b + c) * 0.5f;
                const CurvePoint f = (d + e) * 0.5f;
                left[0] = points[0];
                left[1] = a;
                left[2] = d;
                left[3] = f;
                right[0] = f;
                right[1] = e;
                right[2] = c;
                right[3] = points[3];
                return;
            }
            std::copy(points.begin(), points.end(), midpoints);
            for (size_t i = 0; i < count; ++i) {
                left[i] = midpoints[0];
                right[count - i - 1] = midpoints[count - i - 1];
                for (size_t j = 0; j < count - i - 1; ++j) {
                    midpoints[j] = (midpoints[j] + midpoints[j + 1]) * 0.5f;
                }
            }
        }

        template <typename Curve>
        inline bool bezier_distance(Span<const CurvePoint> points, Curve& distance, Arena* arena);

        template <typename Curve>
        inline bool bspline_distance(Span<const CurvePoint> points, fosu_uint32 requested_degree,
                                     Curve& distance, Arena* arena) {
            if (points.size() < 2) {
                for (size_t i = 0; i < points.size(); ++i) {
                    distance.append(points[i]);
                }
                return true;
            }
            const size_t point_count = points.size() - 1;
            const size_t degree = std::min<size_t>(requested_degree, point_count);
            if (!degree) {
                return false;
            }
            if (degree == point_count) {
                return bezier_distance(points, distance, arena);
            }

            CurvePoint* work = arena_push_array<CurvePoint>(arena, points.size());
            CurvePoint* bezier = arena_push_array<CurvePoint>(arena, degree + 1);
            if (!work || !bezier) {
                return false;
            }
            std::copy(points.begin(), points.end(), work);
            for (size_t i = 0; i < point_count - degree; ++i) {
                bezier[0] = work[i];
                for (size_t j = 0; j < degree - 1; ++j) {
                    bezier[j + 1] = work[i + 1];
                    for (size_t k = 1; k < degree - j; ++k) {
                        const size_t weight = std::min(k, point_count - degree - i);
                        work[i + k] = (work[i + k] * static_cast<float>(weight) + work[i + k + 1]) /
                                      static_cast<float>(weight + 1);
                    }
                }
                bezier[degree] = work[i + 1];
                distance.begin_segment();
                if (!bezier_distance(Span<const CurvePoint>(bezier, degree + 1), distance, arena)) {
                    return false;
                }
            }
            distance.begin_segment();
            return bezier_distance(Span<const CurvePoint>(work + point_count - degree, degree + 1), distance,
                                   arena);
        }

        // The vertices of a flat subdivision, read across the left and right halves.
        inline CurvePoint bezier_half_vertex(const CurvePoint* left, const CurvePoint* right, size_t count,
                                             size_t index) {
            return index < count ? left[index] : right[index - count + 1];
        }

        template <typename Curve>
        inline bool bezier_distance(Span<const CurvePoint> points, Curve& distance, Arena* arena) {
            const size_t count = points.size();
            CurvePoint* work = arena_push_array<CurvePoint>(arena, count * 4);
            if (!work) {
                return false;
            }
            CurvePoint* left = work + count;
            CurvePoint* right = left + count;
            CurvePoint* midpoints = right + count;
            std::copy(points.begin(), points.end(), work);

            // Depth-first subdivision, with reusable arena buffers for pending right halves.
            // Float coordinates converge long before 64 subdivisions.
            const size_t max_depth = 64;
            CurvePoint* pending[64] = {0};
            size_t depth = 0;
            for (;;) {
                bool flat = true;
                for (size_t i = 1; i + 1 < count; ++i) {
                    if ((work[i - 1] - work[i] * 2 + work[i + 1]).squared_length() > 0.25f) {
                        flat = false;
                        break;
                    }
                }
                subdivide_bezier(Span<const CurvePoint>(work, count), left, right, midpoints);
                if (flat) {
                    distance.append(work[0]);
                    for (size_t i = 1; i + 1 < count; ++i) {
                        distance.append((bezier_half_vertex(left, right, count, 2 * i - 1) +
                                         bezier_half_vertex(left, right, count, 2 * i) * 2 +
                                         bezier_half_vertex(left, right, count, 2 * i + 1)) *
                                        0.25f);
                    }
                    if (!depth) {
                        break;
                    }
                    std::copy(pending[depth - 1], pending[depth - 1] + count, work);
                    --depth;
                } else {
                    if (depth == max_depth) {
                        return false;
                    }
                    if (!pending[depth]) {
                        pending[depth] = arena_push_array<CurvePoint>(arena, count);
                    }
                    if (!pending[depth]) {
                        return false;
                    }
                    std::copy(right, right + count, pending[depth]);
                    ++depth;
                    std::copy(left, left + count, work);
                }
            }
            distance.append(points.back());
            return true;
        }

        inline CurvePoint catmull_point(CurvePoint a, CurvePoint b, CurvePoint c, CurvePoint d, float t) {
            const float t2 = t * t, t3 = t * t2;
            return (b * 2 + (c - a) * t + (a * 2 - b * 5 + c * 4 - d) * t2 + (b * 3 - a - c * 3 + d) * t3) *
                   0.5f;
        }

        // PathApproximator.CatmullToPiecewiseLinear, optionally followed by the decimation that
        // SliderPath.OptimiseCatmull performs. Only the osu! ruleset's Slider turns that on; taiko,
        // catch and mania read the undecimated path, so `optimise` selects between them. When it is
        // on, only the
        // first vertex, vertices more than 6px from the last kept one, the last vertex of every
        // knot segment (100 samples) and the final vertex are kept. The curve length that was
        // dropped between kept vertices (minus the chord) is accumulated in optimised_length.
        template <typename Curve>
        inline void catmull_distance(Span<const CurvePoint> points, Curve& distance, bool optimise = true) {
            const int catmull_detail = 50;
            const int catmull_segment_length = catmull_detail * 2;
            const size_t total = (points.size() - 1) * catmull_segment_length;

            bool has_last_start = false;
            CurvePoint last_start = CurvePoint::zero(), previous = CurvePoint::zero();
            double length_removed_since_start = 0;
            size_t index = 0;

            for (size_t i = 0; i + 1 < points.size(); ++i) {
                const CurvePoint a = points[i ? i - 1 : i], b = points[i], c = points[i + 1];
                const CurvePoint d = i + 2 < points.size() ? points[i + 2] : c * 2 - b;
                for (int sample = 0; sample < catmull_segment_length; ++sample, ++index) {
                    const float t = static_cast<float>((sample + 1) / 2) / catmull_detail;
                    const CurvePoint point = catmull_point(a, b, c, d, t);

                    if (!optimise) {
                        distance.append(point);
                        continue;
                    }

                    if (!has_last_start) {
                        distance.append(point);
                        last_start = point;
                        has_last_start = true;
                        previous = point;
                        continue;
                    }

                    const float dist_from_start = (point - last_start).length();
                    length_removed_since_start += (point - previous).length();

                    if (dist_from_start > 6 || (index + 1) % catmull_segment_length == 0 ||
                        index == total - 1) {
                        distance.append(point);
                        distance.optimised_length += length_removed_since_start - dist_from_start;
                        has_last_start = false;
                        length_removed_since_start = 0;
                    }
                    previous = point;
                }
            }
        }

        template <typename Curve>
        inline bool circular_arc_distance(Span<const CurvePoint> points, Curve& distance) {
            const CurvePoint a = points[0], b = points[1], c = points[2];
            // osu.Framework CircularArcProperties: a degenerate (collinear) triangle is not a
            // valid arc; the caller then falls back to the bezier approximation. Without this
            // guard the circumcentre is infinite and the vertices come out as NaN.
            const float collinearity = (b.y - a.y) * (c.x - a.x) - (b.x - a.x) * (c.y - a.y);
            if (std::fabs(collinearity) <= 1e-3f) {
                return false;
            }
            const float divisor = 2 * (a.x * (b - c).y + b.x * (c - a).y + c.x * (a - b).y);
            const CurvePoint centre =
                CurvePoint::make(a.squared_length() * (b - c).y + b.squared_length() * (c - a).y +
                                     c.squared_length() * (a - b).y,
                                 a.squared_length() * (c - b).x + b.squared_length() * (a - c).x +
                                     c.squared_length() * (b - a).x) /
                divisor;
            const CurvePoint first = a - centre, last = c - centre;
            const float radius = first.length();
            const double start = std::atan2(static_cast<double>(first.y), static_cast<double>(first.x));
            double end = std::atan2(static_cast<double>(last.y), static_cast<double>(last.x));
            const double circle = 2 * kPi;
            while (end < start) {
                end += circle;
            }
            double range = end - start, direction = 1;
            if ((c - a).y * (b - a).x - (c - a).x * (b - a).y < 0) {
                direction = -1;
                range = circle - range;
            }
            // osu.Game SliderPath / osu.Framework PathApproximator:
            //   amount = 2 * r <= 0.1f ? 2 : Math.Max(2, (int)Math.Ceiling(range / (2 * Math.Acos(1 - 0.1f /
            //   r))))
            // For a huge radius `1 - 0.1f / r` is exactly 1f, the divisor is 0 and the quotient is
            // infinite (or NaN); .NET 8 on x64 converts that (and any out-of-range value) to
            // int.MinValue, which Math.Max clamps to 2 — a two-vertex chord, not a bezier fallback.
            int amount;
            if (2 * radius <= 0.1f) {
                amount = 2;
            } else {
                const double raw = std::ceil(range / (2 * std::acos(static_cast<double>(1 - 0.1f / radius))));
                const int converted =
                    (raw >= -2147483648.0 && raw < 2147483648.0) ? static_cast<int>(raw) : kMinInt;
                amount = std::max(2, converted);
            }
            // SliderPath.calculateSubPath: 1000 or more sub-points fall back to a bezier approximation.
            if (amount >= 1000) {
                return false;
            }
            for (int i = 0; i < amount; ++i) {
                const double theta = start + direction * (static_cast<double>(i) / (amount - 1)) * range;
                distance.append(centre + CurvePoint::make(static_cast<float>(std::cos(theta)),
                                                          static_cast<float>(std::sin(theta))) *
                                             radius);
            }
            return true;
        }

        template <typename Curve>
        inline bool approximate_curve_segment(Span<const CurvePoint> points, CurveType::Value type,
                                              bool has_degree, fosu_uint32 degree, Curve& segment,
                                              Arena* arena, bool optimise_catmull = true) {
            if (points.size() == 1 || type == CurveType::Linear ||
                (points.size() == 2 && type != CurveType::Catmull)) {
                for (size_t i = 0; i < points.size(); ++i) {
                    segment.append(points[i]);
                }
            } else if (type == CurveType::Catmull) {
                catmull_distance(points, segment, optimise_catmull);
            } else if (type != CurveType::PerfectCurve || points.size() != 3 ||
                       !circular_arc_distance(points, segment)) {
                const fosu_uint32 spline_degree =
                    has_degree ? degree : static_cast<fosu_uint32>(points.size() - 1);
                if (!bspline_distance(points, spline_degree, segment, arena)) {
                    return false;
                }
            }
            return true;
        }

        template <typename Curve>
        inline bool approximate_curve_segment(Span<const CurvePoint> points, CurveType::Value type,
                                              Curve& segment, Arena* arena, bool optimise_catmull = true) {
            return approximate_curve_segment(points, type, false, 0, segment, arena, optimise_catmull);
        }

        inline bool curve_segment_distance(Span<const CurvePoint> points, CurveType::Value type,
                                           CurveDistance& distance, Arena* arena) {
            CurveDistance segment;
            segment.length = distance.length;
            if (!approximate_curve_segment(points, type, segment, arena)) {
                return false;
            }
            // Adjacent segments share their first vertex. The last edge is unchanged
            // unless the new segment consists solely of that shared vertex.
            if (segment.count > 1 || !distance.count) {
                distance.previous = segment.previous;
                distance.last = segment.last;
                distance.count += segment.count;
            }
            distance.length = segment.length;
            return true;
        }

        inline CurveType::Value legacy_curve_type(Span<const CurvePoint> points, CurveType::Value type) {
            if (type != CurveType::PerfectCurve) {
                return type;
            }
            if (points.size() != 3) {
                return CurveType::Bezier;
            }
            const float cross = (points[1].y - points[0].y) * (points[2].x - points[0].x) -
                                (points[1].x - points[0].x) * (points[2].y - points[0].y);
            return std::fabs(cross) < 1e-3f ? CurveType::Linear : type;
        }

        inline CurveType::Value lazer_curve_type(Span<const CurvePoint> points, CurveType::Value type) {
            return type == CurveType::PerfectCurve && points.size() > 3 ? CurveType::Bezier : type;
        }

        inline bool calculate_legacy_slider_distance(Span<const CurvePoint> points, CurveType::Value type,
                                                     CurveDistance& distance, Arena* arena) {
            type = legacy_curve_type(points, type);
            size_t begin = 0;
            for (size_t i = 1; i < points.size(); ++i) {
                if (points[i] != points[i - 1] || i == points.size() - 1 ||
                    (type == CurveType::Catmull && i > 1)) {
                    continue;
                }
                if (i - begin > 1 &&
                    !curve_segment_distance(points.subspan(begin, i - begin), type, distance, arena)) {
                    return false;
                }
                begin = i;
            }
            return curve_segment_distance(points.subspan(begin), type, distance, arena);
        }

        inline bool calculate_lazer_slider_distance(Span<const CurvePoint> points,
                                                    Span<const CurveSegment> segments, CurveType::Value type,
                                                    CurveDistance& distance, Arena* arena) {
            if (segments.empty()) {
                type = lazer_curve_type(points, type);
                size_t begin = 0;
                for (size_t i = 1; i < points.size(); ++i) {
                    if (points[i] != points[i - 1] || i == points.size() - 1) {
                        continue;
                    }
                    if (i - begin > 1 &&
                        !curve_segment_distance(points.subspan(begin, i - begin), type, distance, arena)) {
                        return false;
                    }
                    begin = i;
                }
                return curve_segment_distance(points.subspan(begin), type, distance, arena);
            }

            for (size_t i = 0; i < segments.size(); ++i) {
                const CurveSegment& source = segments[i];
                const size_t start = source.point_begin + (i == 0 ? 0 : 1);
                const size_t count = source.point_count + (i == 0 ? 1 : 0);
                const Span<const CurvePoint> segment_points = points.subspan(start, count);
                const CurveType::Value segment_type = lazer_curve_type(segment_points, source.type);
                distance.begin_segment();
                if (!approximate_curve_segment(segment_points, segment_type, source.has_degree, source.degree,
                                               distance, arena)) {
                    return false;
                }
            }
            return true;
        }

        // Control points relative to the slider head, in single precision, as lazer stores them.
        inline CurvePoint* relative_control_points(const HitObject& object,
                                                   Span<const SliderPoint> control_points, Arena* arena) {
            CurvePoint* points = arena_push_array<CurvePoint>(arena, control_points.size() + 1);
            if (!points) {
                return 0;
            }
            points[0] = CurvePoint::zero();
            for (size_t i = 0; i < control_points.size(); ++i) {
                points[i + 1] = CurvePoint::make(static_cast<float>(control_points[i].x - object.x),
                                                 static_cast<float>(control_points[i].y - object.y));
            }
            return points;
        }

        inline Result<double> slider_distance(const HitObject& object, const Slider& slider,
                                              Span<const SliderPoint> control_points,
                                              Span<const CurveSegment> segments, Arena* arena,
                                              bool lazer_format) {
            // A non-degenerate final linear edge can always reach the declared length.
            // No approximation or scratch allocation is needed to establish its distance.
            if (slider.length > 0 && !control_points.empty()) {
                const SliderPoint last = control_points.back();
                SliderPoint previous;
                if (control_points.size() > 1) {
                    previous = control_points[control_points.size() - 2];
                } else {
                    previous.x = object.x;
                    previous.y = object.y;
                }
                if (last.x != previous.x || last.y != previous.y) {
                    return slider.length;
                }
            }
            const TempArena temp(arena);
            CurvePoint* points = relative_control_points(object, control_points, arena);
            if (!points) {
                return Error(ErrorCode::AllocationFailure);
            }
            const size_t count = control_points.size() + 1;
            CurveDistance distance;
            const Span<const CurvePoint> relative_points(points, count);
            if (lazer_format) {
                if (!calculate_lazer_slider_distance(relative_points, segments, slider.curve_type, distance,
                                                     arena)) {
                    return Error(ErrorCode::AllocationFailure);
                }
            } else {
                if (!calculate_legacy_slider_distance(relative_points, slider.curve_type, distance, arena)) {
                    return Error(ErrorCode::AllocationFailure);
                }
            }
            // A missing/zero declared length uses the natural path. Otherwise osu! trims
            // or extends it, except when a duplicate final vertex prevents extension.
            if (slider.length > 0 && distance.count > 1 &&
                !(distance.last == distance.previous && slider.length > distance.length)) {
                return slider.length;
            }
            return distance.length;
        }

        struct CurveVertexChunk {
            static const size_t capacity = 64;

            CurveVertexChunk* next;
            size_t count;
            CurvePoint points[capacity];
        };

        struct CurveVertices {
            Arena* arena;
            CurveVertexChunk* first;
            CurveVertexChunk* last;
            size_t count;
            double optimised_length; // see catmull_distance
            CurvePoint last_point;
            bool first_in_segment;
            bool failed;

            explicit CurveVertices(Arena* scratch)
                : arena(scratch),
                  first(0),
                  last(0),
                  count(0),
                  optimised_length(0),
                  last_point(CurvePoint::zero()),
                  first_in_segment(true),
                  failed(false) {}

            void begin_segment() { first_in_segment = true; }

            void append(CurvePoint point) {
                const bool shared = first_in_segment && count && last_point == point;
                first_in_segment = false;
                if (shared || failed) {
                    return;
                }
                if (!last || last->count == CurveVertexChunk::capacity) {
                    CurveVertexChunk* chunk = arena_push_array<CurveVertexChunk>(arena, 1);
                    if (!chunk) {
                        failed = true;
                        return;
                    }
                    chunk->next = 0;
                    chunk->count = 0;
                    if (last) {
                        last->next = chunk;
                    } else {
                        first = chunk;
                    }
                    last = chunk;
                }
                last->points[last->count++] = point;
                last_point = point;
                ++count;
            }

            void copy_to(CurvePoint* destination) const {
                for (const CurveVertexChunk* chunk = first; chunk; chunk = chunk->next) {
                    std::memcpy(destination, chunk->points, chunk->count * sizeof(CurvePoint));
                    destination += chunk->count;
                }
            }
        };

        inline bool calculate_legacy_slider_curve(Span<const CurvePoint> points, CurveType::Value type,
                                                  CurveVertices& curve, Arena* arena,
                                                  bool optimise_catmull = true) {
            type = legacy_curve_type(points, type);
            size_t begin = 0;
            for (size_t i = 1; i <= points.size(); ++i) {
                if (i < points.size() && (points[i] != points[i - 1] || i == points.size() - 1 ||
                                          (type == CurveType::Catmull && i > 1))) {
                    continue;
                }
                if (i == points.size() || i - begin > 1) {
                    curve.first_in_segment = i - begin > 1;
                    if (!approximate_curve_segment(points.subspan(begin, i - begin), type, curve, arena,
                                                   optimise_catmull) ||
                        curve.failed) {
                        return false;
                    }
                }
                begin = i;
            }
            return true;
        }

        inline bool calculate_lazer_slider_curve(Span<const CurvePoint> points,
                                                 Span<const CurveSegment> segments, CurveType::Value type,
                                                 CurveVertices& curve, Arena* arena,
                                                 bool optimise_catmull = true) {
            if (segments.empty()) {
                type = lazer_curve_type(points, type);
                size_t begin = 0;
                for (size_t i = 1; i <= points.size(); ++i) {
                    if (i < points.size() && (points[i] != points[i - 1] || i == points.size() - 1)) {
                        continue;
                    }
                    if (i == points.size() || i - begin > 1) {
                        curve.first_in_segment = i - begin > 1;
                        if (!approximate_curve_segment(points.subspan(begin, i - begin), type, curve, arena,
                                                       optimise_catmull) ||
                            curve.failed) {
                            return false;
                        }
                    }
                    begin = i;
                }
                return true;
            }

            for (size_t i = 0; i < segments.size(); ++i) {
                const CurveSegment& source = segments[i];
                const size_t start = source.point_begin + (i == 0 ? 0 : 1);
                const size_t count = source.point_count + (i == 0 ? 1 : 0);
                const Span<const CurvePoint> segment_points = points.subspan(start, count);
                const CurveType::Value segment_type = lazer_curve_type(segment_points, source.type);
                curve.begin_segment();
                if (!approximate_curve_segment(segment_points, segment_type, source.has_degree, source.degree,
                                               curve, arena, optimise_catmull) ||
                    curve.failed) {
                    return false;
                }
            }
            return true;
        }

        // Builds the path of `slider` from control points already relative to the head (`points[0]`
        // is the head itself). `type` is the slider's declared curve type unless the caller has
        // already resolved the legacy collinear-perfect-curve rule.
        inline Result<SliderPath>
        calculate_slider_path_from_relative(const Slider& slider, Span<const CurvePoint> points,
                                            Span<const CurveSegment> segments, CurveType::Value type,
                                            Arena* result_arena, Arena* scratch_arena, bool lazer_format,
                                            bool optimise_catmull = true) {
            const TempArena work(scratch_arena);
            CurveVertices curve(scratch_arena);
            // The first typed control point is itself a one-vertex segment in osu!.
            // Circular approximation can produce a slightly different first vertex.
            if (points.size() > 1 && points[0] != points[1]) {
                curve.append(points[0]);
            }
            if (lazer_format) {
                if (!calculate_lazer_slider_curve(points, segments, type, curve, scratch_arena,
                                                  optimise_catmull)) {
                    return Error(ErrorCode::AllocationFailure);
                }
            } else {
                if (!calculate_legacy_slider_curve(points, type, curve, scratch_arena, optimise_catmull)) {
                    return Error(ErrorCode::AllocationFailure);
                }
            }
            PathPoint* output = arena_push_array<PathPoint>(result_arena, curve.count);
            double* lengths = arena_push_array<double>(result_arena, curve.count);
            if (!output || !lengths) {
                return Error(ErrorCode::AllocationFailure);
            }
            curve.copy_to(output);
            // SliderPath.calculateLength: the running length starts at the optimised-out Catmull
            // length, so the cumulative lengths (but not the first, which stays 0) carry it.
            lengths[0] = 0;
            double calculated_length = curve.optimised_length;
            for (size_t i = 1; i < curve.count; ++i) {
                calculated_length += (output[i] - output[i - 1]).length();
                lengths[i] = calculated_length;
            }
            size_t end = curve.count - 1;
            const double expected = slider.length;
            if (expected > 0 && end && expected != calculated_length &&
                !(expected > calculated_length && output[end] == output[end - 1])) {
                while (end > 1 && lengths[end - 1] >= expected) {
                    --end;
                }
                const PathPoint edge = output[end] - output[end - 1];
                // osuTK Vector2.Normalized(): scale by the reciprocal length in float
                const float scale = 1.0f / edge.length();
                output[end] =
                    output[end - 1] + (edge * scale) * static_cast<float>(expected - lengths[end - 1]);
                lengths[end] = expected;
            }
            SliderPath result;
            result.points = Span<PathPoint>(output, end + 1);
            result.cumulative_lengths = Span<double>(lengths, end + 1);
            return result;
        }

        inline Result<SliderPath> calculate_slider_path(const HitObject& object, const Slider& slider,
                                                        Span<const SliderPoint> control_points,
                                                        Span<const CurveSegment> segments,
                                                        Arena* result_arena, Arena* scratch_arena,
                                                        bool lazer_format, bool optimise_catmull = true) {
            const TempArena work(scratch_arena);
            CurvePoint* points = relative_control_points(object, control_points, scratch_arena);
            if (!points) {
                return Error(ErrorCode::AllocationFailure);
            }
            return calculate_slider_path_from_relative(
                slider, Span<const CurvePoint>(points, control_points.size() + 1), segments,
                slider.curve_type, result_arena, scratch_arena, lazer_format, optimise_catmull);
        }

        inline bool has_catmull_segment(const Slider& slider, Span<const CurveSegment> segments) {
            if (segments.empty()) {
                return slider.curve_type == CurveType::Catmull;
            }
            for (size_t i = 0; i < segments.size(); ++i) {
                if (segments[i].type == CurveType::Catmull) {
                    return true;
                }
            }
            return false;
        }

        inline bool set_slider_paths(Beatmap& map, Arena* result_arena, Arena* scratch_arena) {
            if (map.sliders.empty()) {
                return true;
            }
            SliderPath* paths = arena_push_array<SliderPath>(result_arena, map.sliders.size());
            if (!paths) {
                return false;
            }
            bool success = true;
            for (size_t i = 0; i < map.hit_objects.size(); ++i) {
                const HitObject& object = map.hit_objects[i];
                if (object.slider == HitObject::kNoSlider) {
                    continue;
                }
                const Slider& slider = map.sliders[object.slider];
                const Span<const SliderPoint> control_points =
                    map.slider_points.subspan(slider.point_begin, slider.point_count);
                const Span<const CurveSegment> segments =
                    map.slider_segments.subspan(slider.segment_begin, slider.segment_count);
                const Result<SliderPath> path =
                    calculate_slider_path(object, slider, control_points, segments, result_arena,
                                          scratch_arena, map.format_version >= 128);
                if (path.failed()) {
                    success = false;
                    break;
                }
                paths[object.slider] = path.value();

                // Only the osu! ruleset's Slider sets SliderPath.OptimiseCatmull; every other
                // ruleset reads the undecimated path, so a Catmull slider carries both.
                if (has_catmull_segment(slider, segments)) {
                    const Result<SliderPath> undecimated =
                        calculate_slider_path(object, slider, control_points, segments, result_arena,
                                              scratch_arena, map.format_version >= 128, false);
                    if (undecimated.failed()) {
                        success = false;
                        break;
                    }
                    paths[object.slider].undecimated_points = undecimated.value().points;
                    paths[object.slider].undecimated_cumulative_lengths =
                        undecimated.value().cumulative_lengths;
                }
            }
            if (success) {
                map.slider_paths = Span<SliderPath>(paths, map.sliders.size());
            }
            return success;
        }

    } // namespace internal

    // Recomputes the path of the slider owned by `map.hit_objects[hit_object_index]` with its
    // control points replaced by `relative_points` (relative to the slider head, single precision —
    // exactly what lazer's PathControlPoint.Position holds; `count` must be the slider's
    // point_count). Segment structure, declared length and curve type are kept; for legacy beatmaps
    // the collinear-perfect-curve -> linear rule is decided from the ORIGINAL control points (the
    // unstacked ones when stacking ran), as lazer does at parse time. The path is allocated in
    // `result_arena`; `scratch_arena` is restored on return.
    inline Result<SliderPath> slider_path_from_relative_points(const Beatmap& map, size_t hit_object_index,
                                                               const PathPoint* relative_points, size_t count,
                                                               Arena* result_arena, Arena* scratch_arena) {
        if (hit_object_index >= map.hit_objects.size() || (count && !relative_points)) {
            return Error(ErrorCode::InvalidInput);
        }
        const HitObject& owner = map.hit_objects[hit_object_index];
        if (!owner.is_slider() || owner.slider >= map.sliders.size()) {
            return Error(ErrorCode::InvalidInput);
        }
        const Slider& slider = map.sliders[owner.slider];
        if (count != slider.point_count) {
            return Error(ErrorCode::InvalidInput);
        }
        const bool lazer_format = map.format_version >= 128;

        const TempArena work(scratch_arena);
        internal::CurvePoint* points = arena_push_array<internal::CurvePoint>(scratch_arena, count + 1);
        if (!points) {
            return Error(ErrorCode::AllocationFailure);
        }

        // ConvertHitObjectParser fixes the path type when the file is read: a legacy perfect curve
        // with three collinear points becomes linear. That decision must not be re-taken on the
        // transformed points (a rotated straight line is no longer exactly collinear in float).
        CurveType::Value curve_type = slider.curve_type;
        if (!lazer_format && curve_type == CurveType::PerfectCurve) {
            const Span<const SliderPoint> original =
                map.raw_slider_points.empty() ? map.slider_points : map.raw_slider_points;
            PathPoint head;
            if (map.raw_positions.empty()) {
                head.x = owner.x;
                head.y = owner.y;
            } else {
                head = map.raw_positions[hit_object_index];
            }
            points[0] = PathPoint::zero();
            for (size_t i = 0; i < count; ++i) {
                points[i + 1] = PathPoint::make(original[slider.point_begin + i].x - head.x,
                                                original[slider.point_begin + i].y - head.y);
            }
            curve_type =
                internal::legacy_curve_type(Span<const internal::CurvePoint>(points, count + 1), curve_type);
        }

        points[0] = PathPoint::zero();
        for (size_t i = 0; i < count; ++i) {
            points[i + 1] = relative_points[i];
        }
        const Span<const CurveSegment> segments =
            map.slider_segments.subspan(slider.segment_begin, slider.segment_count);
        return internal::calculate_slider_path_from_relative(
            slider, Span<const internal::CurvePoint>(points, count + 1), segments, curve_type, result_arena,
            scratch_arena, lazer_format);
    }

} // namespace fosu

#endif
