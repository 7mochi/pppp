#ifndef FOSU_ENGINE_PARSING_KEY_VALUE_H
#define FOSU_ENGINE_PARSING_KEY_VALUE_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/string_lookup.h>
#include <fosu/format.h>

namespace fosu { namespace internal {
    struct KeyValue {
        nonstd::string_view key;
        nonstd::string_view value;
    };

    // UTF-8 spellings of the whitespace trimmed by .NET String.Trim. The common
    // ASCII case does not decode Unicode or allocate a replacement string.
    inline size_t field_space_width(nonstd::string_view text) {
        if (text.empty()) {
            return 0;
        }
        const unsigned char* p = reinterpret_cast<const unsigned char*>(text.data());
        if (p[0] == ' ' || (p[0] >= 9 && p[0] <= 13)) {
            return 1;
        }
        if (text.size() >= 2 && p[0] == 0xC2 && (p[1] == 0x85 || p[1] == 0xA0)) {
            return 2;
        }
        if (text.size() >= 3 &&
            ((p[0] == 0xE1 && p[1] == 0x9A && p[2] == 0x80) ||
             (p[0] == 0xE2 && p[1] == 0x80 &&
              ((p[2] <= 0x8A && p[2] >= 0x80) || p[2] == 0xA8 || p[2] == 0xA9 || p[2] == 0xAF)) ||
             (p[0] == 0xE2 && p[1] == 0x81 && p[2] == 0x9F) ||
             (p[0] == 0xE3 && p[1] == 0x80 && p[2] == 0x80))) {
            return 3;
        }
        return 0;
    }

    inline nonstd::string_view trim_field(nonstd::string_view text) {
        for (;;) {
            const size_t width = field_space_width(text);
            if (!width) {
                break;
            }
            text.remove_prefix(width);
        }
        while (!text.empty()) {
            size_t last = text.size() - 1;
            while (last && (static_cast<unsigned char>(text[last]) & 0xC0) == 0x80) {
                --last;
            }
            if (field_space_width(nonstd::string_view(text.data() + last, text.size() - last)) != text.size() - last) {
                break;
            }
            text = nonstd::string_view(text.data(), last);
        }
        return text;
    }

    inline bool split_key_value(nonstd::string_view line, KeyValue& out) {
        const char* p = line.data();
        const char* end = p + line.size();
        const char* colon = find_byte(':', p, end);
        if (colon == end) {
            return false;
        }
        out.key = trim_field(nonstd::string_view(p, static_cast<size_t>(colon - p)));
        out.value = trim_field(nonstd::string_view(colon + 1, static_cast<size_t>(end - colon - 1)));
        return true;
    }

    typedef bool (*FieldParser)(BeatmapHeader&, nonstd::string_view, const ParseContext&);

    // Parses `input` with `ParseValue` and stores the result in `Member` (upstream: a template on
    // `auto` non-type parameters; C++98 needs the member type spelled out).
    template <typename T, T BeatmapHeader::* Member, nonstd::optional<T> (*ParseValue)(nonstd::string_view)>
    inline bool assign_field_value(BeatmapHeader& header, nonstd::string_view input, const ParseContext&) {
        const nonstd::optional<T> value = ParseValue(input);
        if (!value.has_value()) {
            return false;
        }
        header.*Member = *value;
        return true;
    }

    // The same with a widening store (an int32 field value into an int64 member).
    template <typename T, T BeatmapHeader::* Member, typename V, nonstd::optional<V> (*ParseValue)(nonstd::string_view)>
    inline bool assign_field_widened(BeatmapHeader& header, nonstd::string_view input, const ParseContext&) {
        const nonstd::optional<V> value = ParseValue(input);
        if (!value.has_value()) {
            return false;
        }
        header.*Member = *value;
        return true;
    }

    template <nonstd::string_view BeatmapHeader::* Member>
    inline bool assign_field_text(BeatmapHeader& header, nonstd::string_view input, const ParseContext&) {
        header.*Member = input;
        return true;
    }

    inline void parse_key_value(Beatmap& beatmap, const StringLookup<FieldParser>& fields,
                                const KeyValue& field, const ParseContext& context) {
        const FieldParser* parse = fields.find(field.key);
        if (parse) {
            if (!(*parse)(beatmap, field.value, context)) {
                ++beatmap.stats.malformed_lines;
            }
        }
    }

    struct KeyValueSectionLine {
        Beatmap* beatmap;
        const StringLookup<FieldParser>* fields;
        const ParseContext* context;
        void operator()(nonstd::string_view line) {
            KeyValue field;
            if (split_key_value(line, field)) {
                parse_key_value(*beatmap, *fields, field, *context);
            }
        }
    };

    inline const char* parse_key_value_section(Beatmap& beatmap, const StringLookup<FieldParser>& fields,
                                               const char* p, const char* end,
                                               const ParseContext& context) {
        KeyValueSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.fields = &fields;
        parse_line.context = &context;
        return for_each_section_line(p, end, parse_line, context.client);
    }
}} // namespace fosu::internal

#endif
