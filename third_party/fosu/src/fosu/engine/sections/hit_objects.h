#ifndef FOSU_ENGINE_SECTIONS_HIT_OBJECTS_H
#define FOSU_ENGINE_SECTIONS_HIT_OBJECTS_H

#include <algorithm>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/engine/hit_objects/object_types.h>
#include <fosu/engine/hit_objects/slider.h>
#include <fosu/engine/parsing/lines.h>

namespace fosu { namespace internal {

    // Parses one complete [HitObjects] section and publishes accepted objects.
    // Object-specific syntax is delegated after the common fields are decoded.

    // Everything after the "x,y,time,type,hitSound" prefix: slider params,
    // spinner/hold end times, or a trailing hit sample.
    FOSU_NOINLINE inline bool parse_hitobject_details(Beatmap& beatmap, size_t& slider_count,
                                                      size_t& slider_segment_count,
                                                      size_t& slider_point_count, HitObject& object,
                                                      const char* p, const char* end) {
        switch (classify_hitobject_kind(object.type)) {
        case HitObjectKind::Circle: {
            CircleDetails details;
            if (!parse_circle_details(p, end, details)) {
                return false;
            }
            object.end_time = 0;
            object.hit_sample = details.hit_sample;
            return true;
        }
        case HitObjectKind::Slider:
            return p < end && *p == ',' &&
                   parse_slider(beatmap, slider_count, slider_segment_count, slider_point_count, object,
                                p + 1, end);
        case HitObjectKind::Spinner: {
            TimedHitObjectDetails details;
            if (!parse_spinner_details(p, end, details)) {
                return false;
            }
            object.end_time = details.end_time;
            object.hit_sample = details.hit_sample;
            return true;
        }
        case HitObjectKind::Hold: {
            TimedHitObjectDetails details;
            if (!parse_hold_details(object.time, p, end, details)) {
                return false;
            }
            object.end_time = details.end_time;
            object.hit_sample = details.hit_sample;
            return true;
        }
        case HitObjectKind::Invalid: return false;
        }
        return false;
    }

    // The prefix parser handles signed, decimal, spaced or wide fields (upstream's SIMD prefix
    // decoder only took the common editor shapes); anything else is malformed.
    inline bool parse_hitobject_line_scalar(Beatmap& beatmap, size_t& slider_count,
                                            size_t& slider_segment_count, size_t& slider_point_count,
                                            const char* p, const char* line_end, HitObject& object) {
        float x;
        const char* next = parse_osu_float(p, line_end, x, 131072);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        float y;
        next = parse_osu_float(p, line_end, y, 131072);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        double time;
        next = parse_osu_double(p, line_end, time);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        fosu_int64 type;
        next = parse_osu_int(p, line_end, type);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        fosu_int64 hitsound;
        next = parse_osu_int(p, line_end, hitsound);
        if (next == p || (next < line_end && *next != ',')) {
            return false;
        }

        if (beatmap.format_version < 128) {
            x = static_cast<float>(static_cast<fosu_int32>(x));
            y = static_cast<float>(static_cast<fosu_int32>(y));
        }

        ++beatmap.stats.slow_path_lines;
        object.x = x;
        object.y = y;
        object.type = static_cast<fosu_uint32>(type);
        object.hitsound = static_cast<fosu_uint32>(hitsound);
        object.time = time;
        object.end_time = 0;
        object.slider = HitObject::kNoSlider;
        object.new_combo = false;
        object.combo_skip = 0;
        object.hit_sample = StringView();
        return parse_hitobject_details(beatmap, slider_count, slider_segment_count, slider_point_count,
                                       object, next, line_end);
    }

    // Interpret a successfully decoded record before publishing it to the arena.
    // The preceding accepted object is still in source order, including across
    // repeated HitObjects sections. No separate state crosses the engine boundary.
    inline HitObject normalize_hitobject(HitObject object, size_t preceding_count, bool preceding_was_spinner,
                                         int offset) {
        const bool explicit_combo = (object.type & 4) != 0;
        object.time += offset;
        object.new_combo = false;
        object.combo_skip = 0;
        if (object.is_circle() || object.is_slider()) {
            object.new_combo = !preceding_count || explicit_combo || preceding_was_spinner;
            object.combo_skip = explicit_combo ? static_cast<fosu_uint8>((object.type >> 4) & 7) : 0;
            object.end_time = object.is_circle() ? object.time : 0;
        } else if (object.is_spinner()) {
            object.new_combo = explicit_combo;
            object.x = 256;
            object.y = 192;
            object.end_time = std::max(object.time, object.end_time + offset);
        } else {
            // Legacy holds clamp against the offset start before offsetting the end.
            object.end_time = std::max(object.time, object.end_time) + offset;
        }
        return object;
    }

    inline const char* parse_hitobjects_section(Beatmap& beatmap, size_t& hit_object_count,
                                                size_t& slider_count, size_t& slider_segment_count,
                                                size_t& slider_point_count, const char* p,
                                                const char* file_end, int time_offset = 0) {
        while (p < file_end) {
            const char c = *p;
            if (c == '\r' || c == '\n') {
                ++p;
                continue;
            }
            const char* line_end = find_line_end(p, file_end);
            if (c == '[' && section_header_line(p, line_end)) {
                break;
            }
            const char* following_line = after_line_ending(line_end, file_end);
            if (!ignored_line(p, line_end)) {
                HitObject object;
                if (parse_hitobject_line_scalar(beatmap, slider_count, slider_segment_count,
                                                slider_point_count, p, line_end, object)) {
                    const bool preceding_was_spinner =
                        hit_object_count && (object.is_circle() || object.is_slider()) &&
                        !(object.type & 4) &&
                        classify_hitobject_kind(beatmap.hit_objects[hit_object_count - 1].type) ==
                            HitObjectKind::Spinner;
                    beatmap.hit_objects[hit_object_count] =
                        normalize_hitobject(object, hit_object_count, preceding_was_spinner, time_offset);
                    ++hit_object_count;
                } else {
                    ++beatmap.stats.malformed_lines;
                }
            }
            p = following_line;
        }
        return p;
    }

}} // namespace fosu::internal

#endif
