#ifndef FOSU_ENGINE_SECTIONS_EDITOR_H
#define FOSU_ENGINE_SECTIONS_EDITOR_H

#include <algorithm>
#include <fosu/engine/parsing/field_values.h>

#include <fosu/engine/parsing/key_value.h>

namespace fosu { namespace internal {

    inline size_t set_default_velocity_presets(Beatmap& beatmap) {
        beatmap.velocity_presets[0] = 0.75;
        beatmap.velocity_presets[1] = 1;
        beatmap.velocity_presets[2] = 1.5;
        return 3;
    }

    inline Optional<double> parse_editor_scale(StringView input) {
        const Optional<double> value = parse_field_double(input);
        if (value.has_value()) {
            return Optional<double>(std::max(0.0, *value));
        }
        return Optional<double>();
    }

    inline Optional<fosu_int32> parse_beat_divisor(StringView input) {
        const Optional<fosu_int32> value = parse_field_integer(input);
        if (value.has_value()) {
            return Optional<fosu_int32>(clamp_value(*value, 1, 64));
        }
        return Optional<fosu_int32>();
    }

    extern const StringLookup<FieldParser> kEditorFields;

    inline bool parse_velocity_presets(Beatmap& beatmap, size_t& count, StringView input) {
        size_t parsed = 0;
        const char* p = input.data();
        const char* end = p + input.size();
        while (p < end) {
            const char* comma = find_byte(',', p, end);
            const Optional<double> value =
                parse_field_double(trim_field(StringView(p, static_cast<size_t>(comma - p))));
            if (value.has_value() && parsed == beatmap.velocity_presets.size()) {
                return false;
            }
            if (value.has_value()) {
                beatmap.velocity_presets[parsed++] = *value;
            }
            p = comma == end ? end : comma + 1;
        }
        count = parsed;
        return true;
    }

    struct EditorSectionLine {
        Beatmap* beatmap;
        size_t* velocity_preset_count;
        bool* velocity_presets_seen;
        void operator()(StringView line) {
            KeyValue field;
            if (!split_key_value(line, field)) {
                return;
            }
            if (field.key == StringView("VelocityPresets")) {
                *velocity_presets_seen = true;
                if (!parse_velocity_presets(*beatmap, *velocity_preset_count, field.value)) {
                    ++beatmap->stats.malformed_lines;
                }
                return;
            }
            parse_key_value(*beatmap, kEditorFields, field);
        }
    };

    inline const char* parse_editor_section(Beatmap& beatmap, size_t& velocity_preset_count,
                                            bool& velocity_presets_seen, const char* p, const char* end) {
        EditorSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.velocity_preset_count = &velocity_preset_count;
        parse_line.velocity_presets_seen = &velocity_presets_seen;
        return for_each_section_line(p, end, parse_line);
    }

}} // namespace fosu::internal

#endif
