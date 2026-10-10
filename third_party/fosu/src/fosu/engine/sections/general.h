#ifndef FOSU_ENGINE_SECTIONS_GENERAL_H
#define FOSU_ENGINE_SECTIONS_GENERAL_H

#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/key_value.h>
#include <fosu/engine/parsing/sample_sets.h>
#include <fosu/format.h>

namespace fosu { namespace internal {
    inline nonstd::optional<fosu::i32> parse_mode(nonstd::string_view input) {
        const nonstd::optional<fosu::i32> value = parse_field_integer(input);
        if (!value.has_value() || *value < 0 || *value > 3) {
            return nonstd::optional<fosu::i32>();
        }
        return value;
    }

    // Legacy Enum.Parse accepts names, comma-separated combinations, and the full
    // int32 range. Individual domains can impose stricter rules on that spelling.
    inline nonstd::optional<fosu::i32> parse_legacy_enum(nonstd::string_view input, const StringLookup<fosu::i32>& names) {
        const char* p = skip_numeric_space(input.data(), input.data() + input.size());
        const char* end = input.data() + input.size();
        while (end > p && skip_numeric_space(end - 1, end) == end) {
            --end;
        }
        fosu::i64 number;
        const char* q = parse_i64(p, end, number);
        if (q != p && q == end && number >= kInt32Min && number <= kInt32Max) {
            return nonstd::optional<fosu::i32>(static_cast<fosu::i32>(number));
        }
        fosu::i32 value = 0;
        do {
            const char* comma = find_byte(',', p, end);
            const char* part_end = comma;
            while (part_end > p && skip_numeric_space(part_end - 1, part_end) == part_end) {
                --part_end;
            }
            const fosu::i32* named_value = names.find(nonstd::string_view(p, static_cast<size_t>(part_end - p)));
            if (!named_value) {
                return nonstd::optional<fosu::i32>();
            }
            value |= *named_value;
            if (comma == end) {
                return nonstd::optional<fosu::i32>(value);
            }
            p = skip_numeric_space(comma + 1, end);
        } while (p < end);
        return nonstd::optional<fosu::i32>();
    }

    extern const StringLookup<fosu::i32> kCountdownNames;
    extern const StringLookup<fosu::i32> kSampleSetNames;

    inline nonstd::optional<fosu::i32> parse_countdown(nonstd::string_view input) {
        return parse_legacy_enum(input, kCountdownNames);
    }

    inline nonstd::optional<SampleSet::Value> parse_field_sample_set(nonstd::string_view input) {
        // Sample sets are choices, not flags.
        if (input.find(',') != nonstd::string_view::npos) {
            return nonstd::optional<SampleSet::Value>();
        }
        const nonstd::optional<fosu::i32> value = parse_legacy_enum(input, kSampleSetNames);
        return value.has_value() ? parse_sample_set(*value) : nonstd::optional<SampleSet::Value>();
    }

    inline bool parse_skin_sprites(BeatmapHeader& header, nonstd::string_view input, const ParseContext&) {
        header.use_skin_sprites = !input.empty() && input.front() == '1';
        return true;
    }

    inline bool parse_preview_time(BeatmapHeader& header, nonstd::string_view input,
                                   const ParseContext& context) {
        const nonstd::optional<fosu::i32> time = parse_field_integer(input);
        if (!time.has_value()) {
            return false;
        }
        const int offset = *time != -1 ? context.format.time_offset : 0;
        header.preview_time = static_cast<fosu::i32>(static_cast<fosu::u32>(*time) + offset);
        return true;
    }

    // stable clamps stack leniency to [0, 1]; lazer keeps any value.
    inline bool parse_stack_leniency(BeatmapHeader& header, nonstd::string_view input,
                                     const ParseContext& context) {
        const nonstd::optional<double> value = parse_field_float(input);
        if (!value.has_value()) {
            return false;
        }
        double leniency = *value;
        if (context.client == Client::Stable) {
            if (leniency < 0.0) {
                leniency = 0.0;
            }
            if (leniency > 1.0) {
                leniency = 1.0;
            }
        }
        header.stack_leniency = leniency;
        return true;
    }

    extern const StringLookup<FieldParser> kGeneralFields;

    inline const char* parse_general_section(Beatmap& beatmap, const char* p, const char* end,
                                             const ParseContext& context) {
        return parse_key_value_section(beatmap, kGeneralFields, p, end, context);
    }
}} // namespace fosu::internal

#endif
