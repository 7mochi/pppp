#ifndef FOSU_SLIDER_PATH_H
#define FOSU_SLIDER_PATH_H

#include <algorithm>
#include <cmath>

#include <fosu/nonstd_config.h>

// Keep the official implementation's separate single-precision operations: the build must not
// contract a*b+c into fused multiply-adds (pppp's CMake sets -ffp-contract=off; the upstream
// pragmas are compiler extensions).

namespace fosu {
    // Subpixel coordinates relative to the slider head, in osu! playfield pixels.
    struct PathPoint {
        float x, y;

        PathPoint operator+(PathPoint b) const { return make(x + b.x, y + b.y); }
        PathPoint operator-(PathPoint b) const { return make(x - b.x, y - b.y); }
        PathPoint operator*(float scale) const { return make(x * scale, y * scale); }
        PathPoint operator/(float scale) const { return make(x / scale, y / scale); }
        float squared_length() const { return x * x + y * y; }
        float length() const { return std::sqrt(squared_length()); }

        static PathPoint make(float px, float py) {
            PathPoint p;
            p.x = px;
            p.y = py;
            return p;
        }
        static PathPoint zero() { return make(0.0f, 0.0f); }
    };

    inline bool operator==(PathPoint a, PathPoint b) { return a.x == b.x && a.y == b.y; }
    inline bool operator!=(PathPoint a, PathPoint b) { return !(a == b); }

    struct SliderPath {
        nonstd::span<PathPoint> points;
        nonstd::span<double> cumulative_lengths;
        // Empty unless the path has a Catmull segment: the same path without the decimation that
        // SliderPath.OptimiseCatmull applies. Only the osu! ruleset's Slider turns that on, so
        // taiko, catch and mania want these instead.
        nonstd::span<PathPoint> undecimated_points;
        nonstd::span<double> undecimated_cumulative_lengths;

        double distance() const { return cumulative_lengths.empty() ? 0 : cumulative_lengths.back(); }
    };

    // A pure query: no approximation, allocation, or cached mutation. Progress is
    // clamped to [0, 1]; add the hit object's x/y to obtain playfield coordinates.
    inline PathPoint slider_position_at(const SliderPath& path, double progress) {
        if (path.points.empty()) {
            return PathPoint::zero();
        }
        const nonstd::span<const double> lengths = path.cumulative_lengths;
        const double clamped = progress < 0.0 ? 0.0 : (progress > 1.0 ? 1.0 : progress);
        const double distance = clamped * path.distance();
        // The first length not below distance. Heads, tails and repeats sit at the
        // path's ends, so search only between them.
        size_t i = 0;
        if (progress >= 1 && !lengths.empty()) {
            i = lengths.size() - 1;
            while (i && lengths[i - 1] >= distance) {
                --i;
            }
        } else if (progress > 0) {
            // std::lower_bound, choosing each half with a conditional move: where a
            // tick falls along the path is unpredictable, so a branch per step
            // mispredicts about half the time.
            const double* first = lengths.data();
            size_t count = lengths.size();
            while (count > 1) {
                const size_t half = count / 2;
                first = first[half] < distance ? first + half : first;
                count -= half;
            }
            i = static_cast<size_t>(first - lengths.data()) + (count && *first < distance);
        }
        if (!i) {
            return path.points.front();
        }
        if (i >= path.points.size()) {
            return path.points.back();
        }
        const double start = lengths[i - 1];
        const double length = lengths[i] - start;
        if (std::fabs(length) < 1e-7) {
            return path.points[i - 1];
        }
        return path.points[i - 1] +
               (path.points[i] - path.points[i - 1]) * static_cast<float>((distance - start) / length);
    }
} // namespace fosu

#endif
