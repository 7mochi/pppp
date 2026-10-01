#ifndef FOSU_ENGINE_SECTIONS_GENERAL_H
#define FOSU_ENGINE_SECTIONS_GENERAL_H

#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/key_value.h>
#include <fosu/engine/parsing/sample_sets.h>

namespace fosu { namespace internal {

    inline Optional<fosu_int32> parse_mode(StringView input) {
        const Optional<fosu_int32> value = parse_field_integer(input);
        if (!value.has_value() || *value < 0 || *value > 3) {
            return Optional<fosu_int32>();
        }
        return value;
    }

    // Legacy Enum.Parse accepts names, comma-separated combinations, and the full
    // int32 range. Individual domains can impose stricter rules on that spelling.
    inline Optional<fosu_int32> parse_legacy_enum(StringView input, const StringLookup<fosu_int32>& names) {
        const char* p = skip_numeric_space(input.data(), input.data() + input.size());
        const char* end = input.data() + input.size();
        while (end > p && skip_numeric_space(end - 1, end) == end) {
            --end;
        }
        fosu_int64 number;
        const char* q = parse_i64(p, end, number);
        if (q != p && q == end && number >= kInt32Min && number <= kInt32Max) {
            return Optional<fosu_int32>(static_cast<fosu_int32>(number));
        }
        fosu_int32 value = 0;
        do {
            const char* comma = find_byte(',', p, end);
            const char* part_end = comma;
            while (part_end > p && skip_numeric_space(part_end - 1, part_end) == part_end) {
                --part_end;
            }
            const fosu_int32* named_value = names.find(StringView(p, static_cast<size_t>(part_end - p)));
            if (!named_value) {
                return Optional<fosu_int32>();
            }
            value |= *named_value;
            if (comma == end) {
                return Optional<fosu_int32>(value);
            }
            p = skip_numeric_space(comma + 1, end);
        } while (p < end);
        return Optional<fosu_int32>();
    }

    extern const StringLookup<fosu_int32> kCountdownNames;
    extern const StringLookup<fosu_int32> kSampleSetNames;

    inline Optional<fosu_int32> parse_countdown(StringView input) {
        return parse_legacy_enum(input, kCountdownNames);
    }

    inline Optional<SampleSet::Value> parse_field_sample_set(StringView input) {
        // Sample sets are choices, not flags.
        if (input.find(',') != StringView::npos) {
            return Optional<SampleSet::Value>();
        }
        const Optional<fosu_int32> value = parse_legacy_enum(input, kSampleSetNames);
        return value.has_value() ? parse_sample_set(*value) : Optional<SampleSet::Value>();
    }

    inline bool parse_skin_sprites(BeatmapHeader& header, StringView input) {
        header.use_skin_sprites = !input.empty() && input.front() == '1';
        return true;
    }

    inline bool parse_preview_time(BeatmapHeader& header, StringView input) {
        const Optional<fosu_int32> time = parse_field_integer(input);
        if (!time.has_value()) {
            return false;
        }
        const int offset = header.format_version < 5 && *time != -1 ? 24 : 0;
        header.preview_time = static_cast<fosu_int32>(static_cast<fosu_uint32>(*time) + offset);
        return true;
    }

    extern const StringLookup<FieldParser> kGeneralFields;

    inline const char* parse_general_section(Beatmap& beatmap, const char* p, const char* end) {
        return parse_key_value_section(beatmap, kGeneralFields, p, end);
    }

}} // namespace fosu::internal

#endif
