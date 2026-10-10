#ifndef FOSU_ENGINE_SECTIONS_HIT_OBJECTS_H
#define FOSU_ENGINE_SECTIONS_HIT_OBJECTS_H

#include <algorithm>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/engine/hit_objects/object_types.h>
#include <fosu/engine/hit_objects/slider.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/format.h>

namespace fosu { namespace internal {
    // Parses one complete [HitObjects] section and publishes accepted objects.
    // Object-specific syntax is delegated after the common fields are decoded.

    // Everything after the "x,y,time,type,hitSound" prefix: slider params,
    // spinner/hold end times, or a trailing hit sample.
    FOSU_NOINLINE inline bool parse_hitobject_details(Beatmap& beatmap, HitObjectCounts& counts,
                                                      HitObject& object, const char* p, const char* end,
                                                      bool lazer_format, Client::Value client) {
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
                   parse_slider(beatmap, counts, object, p + 1, end, lazer_format, client);
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

    struct HitObjectPrefix {
        HitObject object;
        const char* rest; // After hitSound: empty, or starting with ','.
    };

    // The x,y,time,type,hitSound prefix of a line: signed, decimal, spaced or wide fields.
    inline bool parse_hitobject_prefix(const char* p, const char* line_end, bool lazer_format,
                                       Client::Value client, HitObjectPrefix& out) {
        float x;
        const char* next = parse_osu_float(p, line_end, x, coordinate_limit(client));
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        float y;
        next = parse_osu_float(p, line_end, y, coordinate_limit(client));
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

        fosu::i64 type;
        next = parse_osu_int(p, line_end, type);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        fosu::i64 hitsound;
        next = parse_osu_int(p, line_end, hitsound);
        if (next == p || (next < line_end && *next != ',')) {
            return false;
        }

        x = x < 0.0f ? 0.0f : (x > 512.0f ? 512.0f : x);
        y = y < 0.0f ? 0.0f : (y > 512.0f ? 512.0f : y);
        if (!lazer_format) {
            x = static_cast<float>(static_cast<fosu::i32>(x));
            y = static_cast<float>(static_cast<fosu::i32>(y));
        }

        out.object.x = x;
        out.object.y = y;
        out.object.type = static_cast<fosu::u32>(type);
        out.object.hitsound = static_cast<fosu::u32>(hitsound);
        out.object.time = time;
        out.object.end_time = 0;
        out.object.slider = HitObject::kNoSlider;
        out.object.new_combo = false;
        out.object.combo_skip = 0;
        out.object.hit_sample = nonstd::string_view();
        out.rest = next;
        return true;
    }

    // Anything else the scalar prefix parser rejects is malformed.
    inline bool parse_hitobject_line_scalar(Beatmap& beatmap, HitObjectCounts& counts, const char* p,
                                            const char* line_end, bool lazer_format, Client::Value client,
                                            HitObject& out) {
        HitObjectPrefix prefix;
        if (!parse_hitobject_prefix(p, line_end, lazer_format, client, prefix)) {
            return false;
        }
        ++beatmap.stats.slow_path_lines;
        if (!parse_hitobject_details(beatmap, counts, prefix.object, prefix.rest, line_end, lazer_format,
                                     client)) {
            return false;
        }
        out = prefix.object;
        return true;
    }

    // osu! (lazer) creates a circle once its prefix parses, and a spinner once its end time also
    // parses, before reading the hit sample. A line rejected only for its sample therefore still
    // counts as the previous object when the next one decides whether it starts a new combo.
    // `rest` follows hitSound.
    inline bool preceding_spinner_after_rejection(fosu::u32 type, bool preceding_was_spinner,
                                                  const char* rest, const char* line_end) {
        switch (classify_hitobject_kind(type)) {
        case HitObjectKind::Circle: return false;
        case HitObjectKind::Spinner: {
            if (rest == line_end || *rest != ',') {
                return preceding_was_spinner;
            }
            double end_time;
            const char* next = parse_osu_double(rest + 1, line_end, end_time);
            return (next != rest + 1 && (next == line_end || *next == ',')) || preceding_was_spinner;
        }
        default: return preceding_was_spinner;
        }
    }

    // osu!stable cannot load a map with a hit object it fails to read, but ignores one whose type
    // names no kind. Lazer skips both.
    inline void reject_hitobject(Client::Value client, HitObjectCounts& counts,
                                 bool& preceding_was_spinner, fosu::u32 type, const char* line,
                                 const char* rest, const char* line_end) {
        if (client == Client::Stable) {
            if (!counts.unloadable_line && classify_hitobject_kind(type) != HitObjectKind::Invalid) {
                counts.unloadable_line = line;
            }
        } else {
            preceding_was_spinner =
                preceding_spinner_after_rejection(type, preceding_was_spinner, rest, line_end);
        }
    }

    FOSU_NOINLINE inline void reject_hitobject_line(Beatmap& beatmap, HitObjectCounts& counts,
                                                    bool& preceding_was_spinner, const char* p,
                                                    const char* line_end, bool lazer_format,
                                                    Client::Value client) {
        ++beatmap.stats.malformed_lines;
        HitObjectPrefix prefix;
        if (parse_hitobject_prefix(p, line_end, lazer_format, client, prefix)) {
            reject_hitobject(client, counts, preceding_was_spinner, prefix.object.type, p, prefix.rest,
                             line_end);
        } else if (client == Client::Stable && !counts.unloadable_line) {
            counts.unloadable_line = p;
        }
    }

    // Lines osu!stable skips before reading a hit object: blank, comments, and those indented with
    // ' ' or '_'.
    inline bool ignored_hitobject_line(Client::Value client, const char* p, const char* line_end) {
        if (client == Client::Stable && (*p == ' ' || *p == '_')) {
            return true;
        }
        return ignored_line(p, line_end);
    }

    // Interpret a successfully decoded record of `kind` before publishing it to the arena. The
    // preceding object is in source order, including across repeated HitObjects sections.
    inline HitObject normalize_hitobject(HitObject object, HitObjectKind::Value kind, size_t preceding_count,
                                         bool preceding_was_spinner, int offset) {
        const bool explicit_combo = (object.type & 4) != 0;
        object.time += offset;
        object.new_combo = false;
        object.combo_skip = 0;
        if (kind == HitObjectKind::Circle || kind == HitObjectKind::Slider) {
            object.new_combo = !preceding_count || explicit_combo || preceding_was_spinner;
            object.combo_skip = explicit_combo ? static_cast<fosu::u8>((object.type >> 4) & 7) : 0;
            object.end_time = kind == HitObjectKind::Circle ? object.time : 0;
        } else if (kind == HitObjectKind::Spinner) {
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

    inline const char* parse_hitobjects_section(Beatmap& beatmap, HitObjectCounts& counts, const char* p,
                                                const char* file_end, const ParseContext& context) {
        const bool lazer_format = context.format.lazer_format;
        const int time_offset = context.format.time_offset;
        const Client::Value client = context.client;
        bool preceding_was_spinner = counts.preceding_was_spinner;
        while (p < file_end) {
            const char c = *p;
            if (c == '\r' || c == '\n') {
                ++p;
                continue;
            }
            const char* line_end = find_line_end(p, file_end);
            if (c == '[' && section_header_line(p, line_end, client)) {
                break;
            }
            if (!ignored_hitobject_line(client, p, line_end)) {
                HitObject object;
                if (parse_hitobject_line_scalar(beatmap, counts, p, line_end, lazer_format, client, object)) {
                    const size_t count = counts.objects++;
                    const HitObjectKind::Value kind = classify_hitobject_kind(object.type);
                    beatmap.hit_objects[count] =
                        normalize_hitobject(object, kind, count, preceding_was_spinner, time_offset);
                    preceding_was_spinner = kind == HitObjectKind::Spinner;
                } else {
                    reject_hitobject_line(beatmap, counts, preceding_was_spinner, p, line_end, lazer_format,
                                          client);
                }
            }
            p = after_line_ending(line_end, file_end);
        }
        counts.preceding_was_spinner = preceding_was_spinner;
        return p;
    }
}} // namespace fosu::internal

#endif
