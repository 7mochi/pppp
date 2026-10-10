#ifndef FOSU_ENGINE_SECTIONS_DIFFICULTY_H
#define FOSU_ENGINE_SECTIONS_DIFFICULTY_H

#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/key_value.h>

namespace fosu { namespace internal {
    // HP, CS, OD or AR, clamped to [0, 10] except as noted.
    inline nonstd::optional<double> parse_difficulty_rating(const BeatmapHeader& header,
                                                            nonstd::string_view input,
                                                            Client::Value client, bool circle_size) {
        if (client == Client::Lazer) {
            const nonstd::optional<double> value = parse_field_float(input);
            // Lazer clamps CS after every section, with the final mode; see parse_document.
            if (!value.has_value() || circle_size) {
                return value;
            }
            return nonstd::optional<double>(clamp_value(*value, 0.0, 10.0));
        }
        // Before v13, stable reads ratings with byte.Parse: an integer in [0, 255].
        if (header.format_version < 13) {
            const nonstd::optional<fosu::i32> byte = parse_field_integer(input);
            if (!byte.has_value() || *byte < 0 || *byte > 255) {
                return nonstd::optional<double>();
            }
            return nonstd::optional<double>(clamp_value(static_cast<double>(*byte), 0.0, 10.0));
        }
        const nonstd::optional<double> value = parse_field_float(input);
        if (!value.has_value()) {
            return nonstd::optional<double>();
        }
        // stable clamps as it reads, so CS gets the mania key range only when [General] already
        // set mania.
        if (circle_size && header.mode == 3) {
            return nonstd::optional<double>(clamp_value(*value, 1.0, 18.0));
        }
        return nonstd::optional<double>(clamp_value(*value, 0.0, 10.0));
    }

    inline bool assign_hp_rating(BeatmapHeader& header, nonstd::string_view input,
                                 const ParseContext& context) {
        const nonstd::optional<double> value = parse_difficulty_rating(header, input, context.client, false);
        if (!value.has_value()) {
            return false;
        }
        header.hp = *value;
        return true;
    }

    inline bool assign_cs_rating(BeatmapHeader& header, nonstd::string_view input,
                                 const ParseContext& context) {
        const nonstd::optional<double> value = parse_difficulty_rating(header, input, context.client, true);
        if (!value.has_value()) {
            return false;
        }
        header.cs = *value;
        return true;
    }

    inline bool assign_od_rating(BeatmapHeader& header, nonstd::string_view input,
                                 const ParseContext& context) {
        const nonstd::optional<double> value = parse_difficulty_rating(header, input, context.client, false);
        if (!value.has_value()) {
            return false;
        }
        header.od = *value;
        return true;
    }

    inline nonstd::optional<double> parse_slider_multiplier(nonstd::string_view input) {
        const nonstd::optional<double> value = parse_field_double(input);
        if (value.has_value()) {
            return nonstd::optional<double>(clamp_value(*value, 0.4, 3.6));
        }
        return nonstd::optional<double>();
    }

    inline nonstd::optional<double> parse_slider_tick_rate(nonstd::string_view input) {
        const nonstd::optional<double> value = parse_field_double(input);
        if (value.has_value()) {
            return nonstd::optional<double>(clamp_value(*value, 0.5, 8.0));
        }
        return nonstd::optional<double>();
    }

    extern const StringLookup<FieldParser> kDifficultyFields;

    struct DifficultySectionLine {
        Beatmap* beatmap;
        nonstd::optional<double>* approach_rate;
        const ParseContext* context;
        void operator()(nonstd::string_view line) {
            KeyValue field;
            if (!split_key_value(line, field)) {
                return;
            }
            if (field.key == nonstd::string_view("ApproachRate")) {
                // parse_document supplies the final OD if no valid AR was specified.
                const nonstd::optional<double> value =
                    parse_difficulty_rating(*beatmap, field.value, context->client, false);
                if (value.has_value()) {
                    *approach_rate = value;
                } else {
                    ++beatmap->stats.malformed_lines;
                }
            } else {
                parse_key_value(*beatmap, kDifficultyFields, field, *context);
            }
        }
    };

    inline const char* parse_difficulty_section(Beatmap& beatmap, nonstd::optional<double>& approach_rate,
                                                const char* p, const char* end,
                                                const ParseContext& context) {
        DifficultySectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.approach_rate = &approach_rate;
        parse_line.context = &context;
        return for_each_section_line(p, end, parse_line, context.client);
    }
}} // namespace fosu::internal

#endif
