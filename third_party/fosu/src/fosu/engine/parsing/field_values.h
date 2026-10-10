#ifndef FOSU_ENGINE_PARSING_FIELD_VALUES_H
#define FOSU_ENGINE_PARSING_FIELD_VALUES_H

#include <cstddef>
#include <cstring>

#include <fosu/compiler.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/nonstd_config.h>

namespace fosu { namespace internal {
    inline bool consumed_field_value(nonstd::string_view input, const char* next) {
        if (next == input.data()) {
            return false;
        }
        const char* end = input.data() + input.size();
        while (next < end && (*next == ' ' || *next == '\t')) {
            ++next;
        }
        return next == end;
    }

    inline nonstd::optional<fosu::i32> parse_field_integer(nonstd::string_view input) {
        if (input.empty()) {
            return nonstd::optional<fosu::i32>();
        }
        fosu::i64 value;
        if (!consumed_field_value(input, parse_osu_int(input.data(), input.data() + input.size(), value))) {
            return nonstd::optional<fosu::i32>();
        }
        return nonstd::optional<fosu::i32>(static_cast<fosu::i32>(value));
    }

    const size_t kMaxGroupedLength = 128;
    struct UngroupBuffer {
        char data[2 * kMaxGroupedLength];
    };

    // `input` without the ',' group separators .NET's AllowThousands accepts: any number of them
    // anywhere in the integer part after its first digit. The result is followed by readable zero
    // bytes, as the numeric parsers require. Values over kMaxGroupedLength bytes keep their commas,
    // and so fail.
    inline nonstd::string_view without_group_separators(nonstd::string_view input, UngroupBuffer& buffer) {
        if (input.find(',') == nonstd::string_view::npos || input.size() > kMaxGroupedLength) {
            return input;
        }
        size_t length = 0;
        bool digits = false, integer_part = true;
        for (size_t i = 0; i < input.size(); ++i) {
            const char c = input[i];
            if (is_digit(c)) {
                digits = true;
            } else if (c == '.' || c == 'e' || c == 'E') {
                integer_part = false;
            } else if (c == ',' && digits && integer_part) {
                continue;
            }
            buffer.data[length++] = c;
        }
        std::memset(buffer.data + length, 0, sizeof(buffer.data) - length);
        return nonstd::string_view(buffer.data, length);
    }

    // Decode directly to float32, as osu! does, then widen for the public storage.
    inline nonstd::optional<double> parse_float_value(nonstd::string_view input) {
        if (input.empty()) {
            return nonstd::optional<double>();
        }
        float value;
        if (!consumed_field_value(input, parse_osu_float(input.data(), input.data() + input.size(), value))) {
            return nonstd::optional<double>();
        }
        return nonstd::optional<double>(value);
    }

    inline nonstd::optional<double> parse_double_value(nonstd::string_view input) {
        if (input.empty()) {
            return nonstd::optional<double>();
        }
        double value;
        if (!consumed_field_value(input, parse_double(input.data(), input.data() + input.size(), value)) ||
            value < -kInt32Max || value > kInt32Max) {
            return nonstd::optional<double>();
        }
        return nonstd::optional<double>(value);
    }

    // Both clients' .NET parsers accept group separators in float and double fields. Maps almost
    // never use them, so only a value that fails to parse without them pays to remove them.
    template <nonstd::optional<double> (*ParseValue)(nonstd::string_view)>
    inline nonstd::optional<double> parse_grouped_value(nonstd::string_view input) {
        const nonstd::optional<double> value = ParseValue(input);
        if (value.has_value()) {
            return value;
        }
        UngroupBuffer buffer;
        const nonstd::string_view ungrouped = without_group_separators(input, buffer);
        return ungrouped.data() == input.data() ? nonstd::optional<double>() : ParseValue(ungrouped);
    }

    inline nonstd::optional<double> parse_field_float(nonstd::string_view input) {
        return parse_grouped_value<parse_float_value>(input);
    }

    inline nonstd::optional<double> parse_field_double(nonstd::string_view input) {
        return parse_grouped_value<parse_double_value>(input);
    }

    inline nonstd::optional<bool> parse_field_boolean(nonstd::string_view input) {
        const nonstd::optional<fosu::i32> value = parse_field_integer(input);
        if (value.has_value()) {
            return nonstd::optional<bool>(*value == 1);
        }
        return nonstd::optional<bool>();
    }
}} // namespace fosu::internal

#endif
