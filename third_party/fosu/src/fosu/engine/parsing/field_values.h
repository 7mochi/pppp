#ifndef FOSU_ENGINE_PARSING_FIELD_VALUES_H
#define FOSU_ENGINE_PARSING_FIELD_VALUES_H

#include <fosu/compiler.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/span.h>

namespace fosu { namespace internal {

    inline bool consumed_field_value(StringView input, const char* next) {
        if (next == input.data()) {
            return false;
        }
        const char* end = input.data() + input.size();
        while (next < end && (*next == ' ' || *next == '\t')) {
            ++next;
        }
        return next == end;
    }

    inline Optional<fosu_int32> parse_field_integer(StringView input) {
        if (input.empty()) {
            return Optional<fosu_int32>();
        }
        fosu_int64 value;
        if (!consumed_field_value(input, parse_osu_int(input.data(), input.data() + input.size(), value))) {
            return Optional<fosu_int32>();
        }
        return Optional<fosu_int32>(static_cast<fosu_int32>(value));
    }

    // Decode directly to float32, as osu! does, then widen for the public storage.
    inline Optional<double> parse_field_float(StringView input) {
        if (input.empty()) {
            return Optional<double>();
        }
        float value;
        if (!consumed_field_value(input, parse_osu_float(input.data(), input.data() + input.size(), value))) {
            return Optional<double>();
        }
        return Optional<double>(value);
    }

    inline Optional<double> parse_field_double(StringView input) {
        if (input.empty()) {
            return Optional<double>();
        }
        double value;
        if (!consumed_field_value(input, parse_double(input.data(), input.data() + input.size(), value)) ||
            value < -kInt32Max || value > kInt32Max) {
            return Optional<double>();
        }
        return Optional<double>(value);
    }

    inline Optional<bool> parse_field_boolean(StringView input) {
        const Optional<fosu_int32> value = parse_field_integer(input);
        if (value.has_value()) {
            return Optional<bool>(*value == 1);
        }
        return Optional<bool>();
    }

}} // namespace fosu::internal

#endif
