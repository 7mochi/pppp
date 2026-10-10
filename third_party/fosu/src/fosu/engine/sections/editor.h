#ifndef FOSU_ENGINE_SECTIONS_EDITOR_H
#define FOSU_ENGINE_SECTIONS_EDITOR_H

#include <algorithm>
#include <limits>

#include <fosu/engine/parsing/field_values.h>

#include <fosu/engine/parsing/key_value.h>

namespace fosu { namespace internal {
    inline size_t set_default_velocity_presets(Beatmap& beatmap) {
        beatmap.velocity_presets[0] = 0.75;
        beatmap.velocity_presets[1] = 1;
        beatmap.velocity_presets[2] = 1.5;
        return 3;
    }

    inline nonstd::optional<double> parse_editor_scale(nonstd::string_view input) {
        const nonstd::optional<double> value = parse_field_double(input);
        if (value.has_value()) {
            return nonstd::optional<double>(std::max(0.0, *value));
        }
        return nonstd::optional<double>();
    }

    inline nonstd::optional<fosu::i32> parse_beat_divisor(nonstd::string_view input) {
        const nonstd::optional<fosu::i32> value = parse_field_integer(input);
        if (value.has_value()) {
            return nonstd::optional<fosu::i32>(clamp_value(*value, 1, 64));
        }
        return nonstd::optional<fosu::i32>();
    }

    extern const StringLookup<FieldParser> kEditorFields;

    inline bool preset_word_equals(const char* p, const char* end, const char* word, size_t length) {
        if (static_cast<size_t>(end - p) != length) {
            return false;
        }
        for (size_t i = 0; i < length; ++i) {
            if ((p[i] | 0x20) != word[i]) {
                return false;
            }
        }
        return true;
    }

    // lazer reads each preset with .NET double.TryParse (NumberStyles.Float): ASCII spaces around
    // an optional sign and a decimal, or NaN or Infinity in any case. There is no range limit;
    // out-of-range values become infinities or zeros.
    inline nonstd::optional<double> parse_velocity_preset(nonstd::string_view text) {
        const char* p = text.data();
        const char* end = p + text.size();
        while (p < end && is_numeric_space(*p)) {
            ++p;
        }
        while (end > p && is_numeric_space(end[-1])) {
            --end;
        }
        const bool negative = p < end && *p == '-';
        if (p < end && (*p == '-' || *p == '+')) {
            ++p;
        }
        if (preset_word_equals(p, end, "nan", 3)) {
            return nonstd::optional<double>(std::numeric_limits<double>::quiet_NaN());
        }
        if (preset_word_equals(p, end, "infinity", 8)) {
            return nonstd::optional<double>(negative ? -std::numeric_limits<double>::infinity()
                                                     : std::numeric_limits<double>::infinity());
        }
        if (p == end || !(is_digit(*p) || *p == '.')) {
            return nonstd::optional<double>();
        }
        double value;
        const char* q = bounded_double(p, end, value);
        if (q != end) {
            return nonstd::optional<double>();
        }
        return nonstd::optional<double>(negative ? -value : value);
    }

    inline bool parse_velocity_presets(Beatmap& beatmap, size_t& count, nonstd::string_view input,
                                       bool lazer_format) {
        size_t parsed = 0;
        const char* p = input.data();
        const char* end = p + input.size();
        while (p < end) {
            const char* comma = find_byte(',', p, end);
            const nonstd::optional<double> value =
                parse_velocity_preset(nonstd::string_view(p, static_cast<size_t>(comma - p)));
            // Stable stores exactly three presets; lazer accepts any number.
            const size_t limit = lazer_format ? beatmap.velocity_presets.size() : 3;
            if (value.has_value() && parsed == limit) {
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
        const ParseContext* context;
        void operator()(nonstd::string_view line) {
            KeyValue field;
            if (!split_key_value(line, field)) {
                return;
            }
            if (field.key == nonstd::string_view("VelocityPresets")) {
                *velocity_presets_seen = true;
                if (!parse_velocity_presets(*beatmap, *velocity_preset_count, field.value,
                                            context->format.lazer_format)) {
                    ++beatmap->stats.malformed_lines;
                }
                return;
            }
            parse_key_value(*beatmap, kEditorFields, field, *context);
        }
    };

    inline const char* parse_editor_section(Beatmap& beatmap, size_t& velocity_preset_count,
                                            bool& velocity_presets_seen, const char* p, const char* end,
                                            const ParseContext& context) {
        EditorSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.velocity_preset_count = &velocity_preset_count;
        parse_line.velocity_presets_seen = &velocity_presets_seen;
        parse_line.context = &context;
        return for_each_section_line(p, end, parse_line, context.client);
    }
}} // namespace fosu::internal

#endif
