#ifndef FOSU_ENGINE_HIT_OBJECTS_SLIDER_H
#define FOSU_ENGINE_HIT_OBJECTS_SLIDER_H

// Parses the slider-specific portion of one hit object and publishes its
// Slider, control points and curve segments into the Beatmap. Section
// iteration and common hit-object fields belong to sections/hit_objects.h.

#include <algorithm>
#include <cstddef>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/compiler.h>
#include <fosu/engine/hit_objects/samples.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/primitives/packed_digits.h>
#include <fosu/enums.h>

namespace fosu { namespace internal {

    inline bool parse_curve_type(char value, CurveType::Value& out) {
        switch (value) {
        case 'B': out = CurveType::Bezier; return true;
        case 'C': out = CurveType::Catmull; return true;
        case 'L': out = CurveType::Linear; return true;
        case 'P': out = CurveType::PerfectCurve; return true;
        default: return false;
        }
    }

    // An explicit lazer B-spline degree ("B2"): parses the digits after the type letter.
    inline bool parse_curve_degree(const char*& p, const char* end, bool& has_degree, fosu_uint32& degree) {
        has_degree = false;
        degree = 0;
        if (p < end && is_digit(*p)) {
            fosu_int64 value;
            const char* next = parse_osu_int(p, end, value);
            if (next == p || value <= 0 || value > static_cast<fosu_int64>(kUint32Max)) {
                return false;
            }
            has_degree = true;
            degree = static_cast<fosu_uint32>(value);
            p = next;
        }
        return true;
    }

    inline CurveSegment make_curve_segment(CurveType::Value type, bool has_degree, fosu_uint32 degree,
                                           size_t point_begin, size_t point_count) {
        CurveSegment segment;
        segment.type = type;
        segment.has_degree = has_degree;
        segment.degree = degree;
        segment.point_begin = static_cast<fosu_uint32>(point_begin);
        segment.point_count = static_cast<fosu_uint32>(point_count);
        return segment;
    }

    // Slider params after "type,hitSound,":
    //   curveType|x:y|x:y...,slides,length[,edgeSounds,edgeSets][,hitSample]
    //
    // This is one parse transaction. Control points and completed lazer segments
    // are written into their arena arrays as they are accepted. Failures roll both
    // arrays back so rejected sliders leave no published data behind.
    FOSU_NOINLINE inline bool parse_slider(Beatmap& beatmap, size_t& slider_count,
                                           size_t& slider_segment_count, size_t& slider_point_count,
                                           HitObject& object, const char* p, const char* end) {
        if (p >= end) {
            return false;
        }

        const bool lazer = beatmap.format_version >= 128;
        const size_t slider_point_begin = slider_point_count;
        const size_t slider_segment_begin = slider_segment_count;

        CurveType::Value first_curve_type;
        if (!parse_curve_type(*p++, first_curve_type)) {
            return false;
        }

        bool first_curve_has_degree = false;
        fosu_uint32 first_curve_degree = 0;
        if (first_curve_type == CurveType::Bezier &&
            !parse_curve_degree(p, end, first_curve_has_degree, first_curve_degree)) {
            return false;
        }

        CurveType::Value current_curve_type = first_curve_type;
        bool current_curve_has_degree = first_curve_has_degree;
        fosu_uint32 current_curve_degree = first_curve_degree;
        size_t segment_point_begin = slider_point_count;
        bool has_explicit_segments = false;

        while (p < end && *p == '|') {
            const bool starts_segment =
                lazer && p + 1 < end && (p[1] == 'B' || p[1] == 'C' || p[1] == 'L' || p[1] == 'P');

            CurveType::Value next_curve_type = current_curve_type;
            bool next_curve_has_degree = current_curve_has_degree;
            fosu_uint32 next_curve_degree = current_curve_degree;
            if (starts_segment) {
                ++p;
                if (!parse_curve_type(*p++, next_curve_type)) {
                    slider_point_count = slider_point_begin;
                    slider_segment_count = slider_segment_begin;
                    return false;
                }
                next_curve_has_degree = false;
                next_curve_degree = 0;
                if (next_curve_type == CurveType::Bezier &&
                    !parse_curve_degree(p, end, next_curve_has_degree, next_curve_degree)) {
                    slider_point_count = slider_point_begin;
                    slider_segment_count = slider_segment_begin;
                    return false;
                }
            }

            if (p >= end || *p != '|') {
                slider_point_count = slider_point_begin;
                slider_segment_count = slider_segment_begin;
                return false;
            }

            const char* coordinate = p + 1;
            float x;
            fosu_uint32 digits = digit_run8(coordinate);
            if (digits - 1 <= 3 && digits <= static_cast<size_t>(end - coordinate) &&
                coordinate[digits] == ':') {
                x = static_cast<float>(swar_parse_u32(coordinate, digits));
                coordinate += digits;
            } else {
                const char* next = parse_osu_float(coordinate, end, x, 131072);
                if (next == coordinate || next >= end || *next != ':') {
                    slider_point_count = slider_point_begin;
                    slider_segment_count = slider_segment_begin;
                    return false;
                }
                if (!lazer) {
                    x = static_cast<float>(static_cast<fosu_int32>(x));
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
                const char* next = parse_osu_float(coordinate, end, y, 131072);
                if (next == coordinate) {
                    slider_point_count = slider_point_begin;
                    slider_segment_count = slider_segment_begin;
                    return false;
                }
                if (!lazer) {
                    y = static_cast<float>(static_cast<fosu_int32>(y));
                }
                coordinate = next;
            }

            SliderPoint point;
            point.x = x;
            point.y = y;
            beatmap.slider_points[slider_point_count++] = point;
            p = coordinate;

            if (starts_segment) {
                if (slider_segment_count == beatmap.slider_segments.size()) {
                    slider_point_count = slider_point_begin;
                    slider_segment_count = slider_segment_begin;
                    return false;
                }
                beatmap.slider_segments[slider_segment_count++] = make_curve_segment(
                    current_curve_type, current_curve_has_degree, current_curve_degree,
                    segment_point_begin - slider_point_begin, slider_point_count - segment_point_begin);
                has_explicit_segments = true;
                segment_point_begin = slider_point_count - 1;
                current_curve_type = next_curve_type;
                current_curve_has_degree = next_curve_has_degree;
                current_curve_degree = next_curve_degree;
            }
        }

        // Everything after the point list is positional.
        if (p >= end || *p != ',') {
            slider_point_count = slider_point_begin;
            slider_segment_count = slider_segment_begin;
            return false;
        }
        ++p;

        fosu_int32 slides;
        const fosu_uint32 first_slide_digit = static_cast<fosu_uint8>(p[0] - '0');
        const fosu_uint32 second_slide_digit = static_cast<fosu_uint8>(p[1] - '0');
        if (first_slide_digit <= 9 && p[1] == ',') {
            slides = static_cast<fosu_int32>(first_slide_digit);
            ++p;
        } else if (first_slide_digit <= 9 && second_slide_digit <= 9 && p[2] == ',') {
            slides = static_cast<fosu_int32>(first_slide_digit * 10 + second_slide_digit);
            p += 2;
        } else {
            const fosu_uint32 digits = digit_run8(p);
            if (digits - 1 <= 6) {
                slides = static_cast<fosu_int32>(swar_parse_u64(p, digits));
                p = skip_numeric_space(p + digits, end);
            } else {
                fosu_int64 parsed_slides;
                const char* next = parse_osu_int(p, end, parsed_slides);
                if (next == p) {
                    slider_point_count = slider_point_begin;
                    slider_segment_count = slider_segment_begin;
                    return false;
                }
                slides = clamp_i32(parsed_slides);
                p = next;
            }
        }
        if (slides > 9000 || (p < end && *p != ',')) {
            slider_point_count = slider_point_begin;
            slider_segment_count = slider_segment_begin;
            return false;
        }

        double length = 0;
        if (p < end) {
            const char* length_begin = p + 1;
            const char* next = parse_osu_double(length_begin, end, length, 131072);
            if (next != length_begin) {
                next = skip_numeric_space(next, end);
            }
            if (next == length_begin || (next < end && *next != ',')) {
                slider_point_count = slider_point_begin;
                slider_segment_count = slider_segment_begin;
                return false;
            }
            p = next;
        }

        StringView sound_fields[3];
        if (p < end) {
            const char* sound_begin = p + 1;
            for (int field = 0; field < 3; ++field) {
                const char* comma = find_byte(',', sound_begin, end);
                sound_fields[field] = StringView(sound_begin, static_cast<size_t>(comma - sound_begin));
                if (comma == end) {
                    break;
                }
                sound_begin = comma + 1;
            }
        }

        const StringView edge_sounds = sound_fields[0];
        const StringView edge_sets = sound_fields[1];
        const StringView hit_sample = sound_fields[2];
        if (!valid_sample(hit_sample, true) || !valid_edge_sets(edge_sets, slides)) {
            slider_point_count = slider_point_begin;
            slider_segment_count = slider_segment_begin;
            return false;
        }

        if (lazer && (has_explicit_segments || first_curve_has_degree)) {
            if (slider_segment_count == beatmap.slider_segments.size()) {
                slider_point_count = slider_point_begin;
                slider_segment_count = slider_segment_begin;
                return false;
            }
            beatmap.slider_segments[slider_segment_count++] = make_curve_segment(
                current_curve_type, current_curve_has_degree, current_curve_degree,
                segment_point_begin - slider_point_begin, slider_point_count - segment_point_begin);
        }

        Slider& slider = beatmap.sliders[slider_count];
        slider.point_begin = static_cast<fosu_uint32>(slider_point_begin);
        slider.point_count = static_cast<fosu_uint32>(slider_point_count - slider_point_begin);
        slider.segment_begin = static_cast<fosu_uint32>(slider_segment_begin);
        slider.segment_count =
            lazer ? static_cast<fosu_uint32>(slider_segment_count - slider_segment_begin) : 0;
        slider.slides = std::max(1, slides);
        slider.curve_type = first_curve_type;
        slider.length = std::max(0.0, length);
        slider.edge_sounds = edge_sounds;
        slider.edge_sets = edge_sets;
        object.hit_sample = hit_sample;
        object.slider = static_cast<fosu_uint32>(slider_count++);
        return true;
    }

}} // namespace fosu::internal

#endif
