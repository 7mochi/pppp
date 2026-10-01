#ifndef FOSU_ENGINE_SECTIONS_DIFFICULTY_H
#define FOSU_ENGINE_SECTIONS_DIFFICULTY_H

#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/key_value.h>

namespace fosu { namespace internal {

    inline Optional<double> parse_difficulty_rating(StringView input) {
        const Optional<double> value = parse_field_float(input);
        if (value.has_value()) {
            return Optional<double>(clamp_value(*value, 0.0, 10.0));
        }
        return Optional<double>();
    }

    inline Optional<double> parse_slider_multiplier(StringView input) {
        const Optional<double> value = parse_field_double(input);
        if (value.has_value()) {
            return Optional<double>(clamp_value(*value, 0.4, 3.6));
        }
        return Optional<double>();
    }

    inline Optional<double> parse_slider_tick_rate(StringView input) {
        const Optional<double> value = parse_field_double(input);
        if (value.has_value()) {
            return Optional<double>(clamp_value(*value, 0.5, 8.0));
        }
        return Optional<double>();
    }

    extern const StringLookup<FieldParser> kDifficultyFields;

    struct DifficultySectionLine {
        Beatmap* beatmap;
        Optional<double>* approach_rate;
        void operator()(StringView line) {
            KeyValue field;
            if (!split_key_value(line, field)) {
                return;
            }
            if (field.key == StringView("ApproachRate")) {
                // parse_document supplies the final OD if no valid AR was specified.
                const Optional<double> value = parse_difficulty_rating(field.value);
                if (value.has_value()) {
                    *approach_rate = value;
                } else {
                    ++beatmap->stats.malformed_lines;
                }
            } else {
                parse_key_value(*beatmap, kDifficultyFields, field);
            }
        }
    };

    inline const char* parse_difficulty_section(Beatmap& beatmap, Optional<double>& approach_rate,
                                                const char* p, const char* end) {
        DifficultySectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.approach_rate = &approach_rate;
        return for_each_section_line(p, end, parse_line);
    }

}} // namespace fosu::internal

#endif
