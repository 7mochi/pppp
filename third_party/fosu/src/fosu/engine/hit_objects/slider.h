#ifndef FOSU_ENGINE_HIT_OBJECTS_SLIDER_H
#define FOSU_ENGINE_HIT_OBJECTS_SLIDER_H

// Parses the slider-specific portion of one hit object and publishes its
// Slider, control points and curve segments into the Beatmap. Section
// iteration and common hit-object fields belong to sections/hit_objects.h.

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>

#include <fosu/beatmap.h>
#include <fosu/compiler.h>
#include <fosu/engine/hit_objects/samples.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/engine/primitives/packed_digits.h>
#include <fosu/enums.h>
#include <fosu/parse_options.h>

namespace fosu { namespace internal {
    // Running counters for one [HitObjects] section (upstream: HitObjectCounts).
    struct HitObjectCounts {
        size_t objects;
        size_t sliders;
        size_t slider_segments;
        size_t slider_points;
        // Whether osu!'s previous object, in source order, is a spinner. It can be a rejected line:
        // see preceding_spinner_after_rejection.
        bool preceding_was_spinner;
        // The first line that makes osu!stable refuse the map, if any.
        const char* unloadable_line;

        HitObjectCounts()
            : objects(0),
              sliders(0),
              slider_segments(0),
              slider_points(0),
              preceding_was_spinner(false),
              unloadable_line(0) {}
    };

    // Lazer bounds coordinates and slider lengths by +-131072. stable reads any finite length, and
    // fosu keeps its coordinates where the int32 truncation of a control point stays defined.
    inline float coordinate_limit(Client::Value client) {
        return client == Client::Lazer ? 131072.0f : 2147483520.0f;
    }
    inline double slider_length_limit(Client::Value client) {
        return client == Client::Lazer ? 131072.0 : std::numeric_limits<double>::max();
    }

    // Whether the token after the '|' at `p` is a single character.
    inline bool one_character_token(const char* p, const char* end) {
        if (p + 1 >= end || p[1] == '|' || p[1] == ',') {
            return false;
        }
        return p + 2 == end || p[2] == '|' || p[2] == ',';
    }

    inline bool is_ascii_letter(char value) {
        return static_cast<fosu::u8>((value | 0x20) - 'a') < 26;
    }

    inline bool parse_curve_type(char value, CurveType::Value& out) {
        switch (value) {
        case 'B': out = CurveType::Bezier; return true;
        case 'C': out = CurveType::Catmull; return true;
        case 'L': out = CurveType::Linear; return true;
        case 'P': out = CurveType::PerfectCurve; return true;
        default: return false;
        }
    }

    inline CurveSegment make_curve_segment(CurveType::Value type, bool has_degree, fosu::u32 degree,
                                           size_t point_begin, size_t point_count) {
        CurveSegment segment;
        segment.type = type;
        segment.has_degree = has_degree;
        segment.degree = degree;
        segment.point_begin = static_cast<fosu::u32>(point_begin);
        segment.point_count = static_cast<fosu::u32>(point_count);
        return segment;
    }

    struct ParsedSliderPoint {
        SliderPoint point;
        const char* end;
    };

    inline bool parse_slider_point(const char* p, const char* end, bool lazer_format, float limit,
                                   ParsedSliderPoint& out) {
        const char* coordinate = p + 1;
        float x;
        fosu::u32 digits = digit_run8(coordinate);
        if (digits - 1 <= 3 && digits <= static_cast<size_t>(end - coordinate) &&
            coordinate[digits] == ':') {
            x = static_cast<float>(swar_parse_u32(coordinate, digits));
            coordinate += digits;
        } else {
            const char* next = parse_osu_float(coordinate, end, x, limit);
            if (next == coordinate || next >= end || *next != ':') {
                return false;
            }
            if (!lazer_format) {
                x = static_cast<float>(static_cast<fosu::i32>(x));
            }
            coordinate = next;
        }

        ++coordinate;
        float y;
        digits = digit_run8(coordinate);
        if (digits - 1 <= 3 && digits <= static_cast<size_t>(end - coordinate) &&
            (coordinate[digits] == ':' || coordinate[digits] == '|' || coordinate[digits] == ',')) {
            y = static_cast<float>(swar_parse_u32(coordinate, digits));
            coordinate += digits;
        } else {
            const char* next = parse_osu_float(coordinate, end, y, limit);
            if (next == coordinate) {
                return false;
            }
            if (!lazer_format) {
                y = static_cast<float>(static_cast<fosu::i32>(y));
            }
            coordinate = next;
        }

        // osu! reads only the first two ':'-separated values of a point.
        if (coordinate < end && *coordinate == ':') {
            while (coordinate < end && *coordinate != '|' && *coordinate != ',') {
                ++coordinate;
            }
        }
        out.point.x = x;
        out.point.y = y;
        out.end = coordinate;
        return true;
    }

    // Slider params after "type,hitSound,":
    //   curveType|x:y|x:y...,slides,length[,edgeSounds,edgeSets][,hitSample]
    //
    // This is one parse transaction. Control points and completed lazer segments are written into
    // their arena arrays as they are accepted. Failures roll both arrays back so rejected sliders
    // leave no published data behind.
    //
    // Lazer starts a segment at each curve type in the point list; before v128 it later applies
    // legacy rules to each one. stable instead reads a one-character token as the curve type of the
    // whole slider, the last one winning, and ignores one naming no type, in every format; fosu
    // follows it in stable mode. Lazer would also read a longer token starting with a letter as a
    // type, which fosu rejects.
    FOSU_NOINLINE inline bool parse_slider(Beatmap& beatmap, HitObjectCounts& counts, HitObject& object,
                                           const char* p, const char* end, bool lazer_format,
                                           Client::Value client) {
        if (p >= end) {
            return false;
        }

        const size_t slider_point_begin = counts.slider_points;
        const size_t slider_segment_begin = counts.slider_segments;
        const bool segments = client == Client::Lazer;
        const float limit = coordinate_limit(client);

        const char first_token = *p++;
        CurveType::Value known_curve_type;
        const bool known = parse_curve_type(first_token, known_curve_type);
        if (!known && !is_ascii_letter(first_token)) {
            // Lazer reads any other character as a point, which fails.
            if (client == Client::Lazer) {
                return false;
            }
            // stable ignores it as a marker, unless the token is empty.
            if (first_token == '|' || first_token == ',') {
                return false;
            }
        }
        // Both clients read a letter other than B, C, L or P as Catmull, which is also stable's
        // type when it ignores a marker.
        const CurveType::Value first_curve_type = known ? known_curve_type : CurveType::Catmull;

        bool first_curve_has_degree = false;
        fosu::u32 first_curve_degree = 0;
        if (first_curve_type == CurveType::Bezier && p < end && is_digit(*p)) {
            // B-spline degrees belong to lazer's v128 format. Otherwise the line is rejected:
            // stable cannot load it, and lazer before v128 (unlike us) reads a B-spline.
            if (!lazer_format) {
                return false;
            }
            fosu::i64 degree;
            const char* next = parse_osu_int(p, end, degree);
            if (next == p || degree <= 0 || degree > static_cast<fosu::i64>(kUint32Max)) {
                return false;
            }
            first_curve_has_degree = true;
            first_curve_degree = static_cast<fosu::u32>(degree);
            p = next;
        }

        CurveType::Value current_curve_type = first_curve_type;
        bool current_curve_has_degree = first_curve_has_degree;
        fosu::u32 current_curve_degree = first_curve_degree;
        size_t segment_point_begin = counts.slider_points;
        bool has_explicit_segments = false;

        while (p < end && *p == '|') {
            if (!segments && one_character_token(p, end)) {
                CurveType::Value type;
                if (parse_curve_type(p[1], type)) {
                    current_curve_type = type;
                }
                p += 2;
                continue;
            }

            const bool starts_segment = segments && p + 1 < end && is_ascii_letter(p[1]);

            CurveType::Value next_curve_type = current_curve_type;
            bool next_curve_has_degree = current_curve_has_degree;
            fosu::u32 next_curve_degree = current_curve_degree;
            if (starts_segment) {
                ++p;
                CurveType::Value parsed;
                next_curve_type = parse_curve_type(*p, parsed) ? parsed : CurveType::Catmull;
                ++p;
                next_curve_has_degree = false;
                next_curve_degree = 0;
                // As for the first type, fosu reads B-spline degrees only in lazer's v128 format.
                if (lazer_format && next_curve_type == CurveType::Bezier && p < end && is_digit(*p)) {
                    fosu::i64 degree;
                    const char* next = parse_osu_int(p, end, degree);
                    if (next == p || degree <= 0 || degree > static_cast<fosu::i64>(kUint32Max)) {
                        counts.slider_points = slider_point_begin;
                        counts.slider_segments = slider_segment_begin;
                        return false;
                    }
                    next_curve_has_degree = true;
                    next_curve_degree = static_cast<fosu::u32>(degree);
                    p = next;
                }
            }

            if (p >= end || *p != '|') {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }

            ParsedSliderPoint point;
            if (!parse_slider_point(p, end, lazer_format, limit, point)) {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }
            beatmap.slider_points[counts.slider_points++] = point.point;
            p = point.end;

            if (starts_segment) {
                if (counts.slider_segments == beatmap.slider_segments.size()) {
                    counts.slider_points = slider_point_begin;
                    counts.slider_segments = slider_segment_begin;
                    return false;
                }
                beatmap.slider_segments[counts.slider_segments++] =
                    make_curve_segment(current_curve_type, current_curve_has_degree, current_curve_degree,
                                       segment_point_begin - slider_point_begin,
                                       counts.slider_points - segment_point_begin);
                has_explicit_segments = true;
                segment_point_begin = counts.slider_points - 1;
                current_curve_type = next_curve_type;
                current_curve_has_degree = next_curve_has_degree;
                current_curve_degree = next_curve_degree;
            }
        }

        // Everything after the point list is positional.
        if (p >= end || *p != ',') {
            counts.slider_points = slider_point_begin;
            counts.slider_segments = slider_segment_begin;
            return false;
        }
        ++p;

        fosu::i32 slides;
        const fosu::u32 first_slide_digit = static_cast<fosu::u8>(p[0] - '0');
        if (first_slide_digit <= 9 && p[1] == ',') {
            slides = static_cast<fosu::i32>(first_slide_digit);
            ++p;
        } else {
            const fosu::u32 digits = digit_run8(p);
            if (digits - 1 <= 6) {
                slides = static_cast<fosu::i32>(swar_parse_u64(p, digits));
                p = skip_numeric_space(p + digits, end);
            } else {
                fosu::i64 parsed_slides;
                const char* next = parse_osu_int(p, end, parsed_slides);
                if (next == p) {
                    counts.slider_points = slider_point_begin;
                    counts.slider_segments = slider_segment_begin;
                    return false;
                }
                slides = clamp_i32(parsed_slides);
                p = next;
            }
        }
        if (slides > 9000 || (p < end && *p != ',')) {
            counts.slider_points = slider_point_begin;
            counts.slider_segments = slider_segment_begin;
            return false;
        }

        double length = 0;
        if (p < end) {
            const char* length_begin = p + 1;
            const char* next = parse_osu_double(length_begin, end, length, slider_length_limit(client));
            if (next == length_begin || (next < end && *next != ',')) {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }
            p = next;
        }

        nonstd::string_view sound_fields[3];
        if (p < end) {
            const char* sound_begin = p + 1;
            for (int field = 0; field < 3; ++field) {
                const char* comma = find_byte(',', sound_begin, end);
                sound_fields[field] = nonstd::string_view(sound_begin, static_cast<size_t>(comma - sound_begin));
                if (comma == end) {
                    break;
                }
                sound_begin = comma + 1;
            }
        }

        const nonstd::string_view edge_sounds = sound_fields[0];
        const nonstd::string_view edge_sets = sound_fields[1];
        const nonstd::string_view hit_sample = sound_fields[2];
        if (!valid_sample(hit_sample, true) || !valid_edge_sets(edge_sets, slides)) {
            counts.slider_points = slider_point_begin;
            counts.slider_segments = slider_segment_begin;
            return false;
        }

        if (segments && (has_explicit_segments || first_curve_has_degree)) {
            if (counts.slider_segments == beatmap.slider_segments.size()) {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }
            beatmap.slider_segments[counts.slider_segments++] =
                make_curve_segment(current_curve_type, current_curve_has_degree, current_curve_degree,
                                   segment_point_begin - slider_point_begin,
                                   counts.slider_points - segment_point_begin);
        }

        Slider& slider = beatmap.sliders[counts.sliders];
        slider.point_begin = static_cast<fosu::u32>(slider_point_begin);
        slider.point_count = static_cast<fosu::u32>(counts.slider_points - slider_point_begin);
        slider.segment_begin = static_cast<fosu::u32>(slider_segment_begin);
        slider.segment_count =
            segments ? static_cast<fosu::u32>(counts.slider_segments - slider_segment_begin) : 0;
        slider.slides = std::max(1, slides);
        slider.curve_type = segments ? first_curve_type : current_curve_type;
        slider.length = std::max(0.0, length);
        slider.edge_sounds = edge_sounds;
        slider.edge_sets = edge_sets;
        object.hit_sample = hit_sample;
        object.slider = static_cast<fosu::u32>(counts.sliders++);
        return true;
    }
}} // namespace fosu::internal

#endif
