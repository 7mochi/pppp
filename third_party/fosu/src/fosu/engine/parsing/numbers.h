#ifndef FOSU_ENGINE_PARSING_NUMBERS_H
#define FOSU_ENGINE_PARSING_NUMBERS_H

// Scalar numeric parsing. Like the rest of the parser, these helpers assume the buffer is
// followed by kBufferPadding readable zero bytes.

#include <cmath>
#include <limits>

#include <fosu/compiler.h>
#include <fosu/engine/parsing/decimal.h>

namespace fosu { namespace internal {

    inline bool is_digit(char c) { return static_cast<unsigned char>(c - '0') <= 9; }

    // All parse_* helpers return the advanced pointer, or `p` unchanged on failure.

    inline const char* parse_u64(const char* p, const char* end, fosu_uint64& out) {
        const char* start = p;
        fosu_uint64 v = 0;
        const fosu_uint64 max = uint64_max();
        while (p < end && is_digit(*p)) {
            const fosu_uint64 digit = static_cast<unsigned>(*p - '0');
            if (v > max / 10 || (v == max / 10 && digit > max % 10)) {
                do {
                    ++p;
                } while (p < end && is_digit(*p));
                out = max;
                return p;
            }
            v = v * 10 + digit;
            ++p;
        }
        if (p == start) {
            return start;
        }
        out = v;
        return p;
    }

    inline const char* parse_i64(const char* p, const char* end, fosu_int64& out) {
        const char* start = p;
        bool neg = false;
        if (p < end && (*p == '-' || *p == '+')) {
            neg = *p == '-';
            ++p;
        }
        fosu_uint64 mag;
        const char* q = parse_u64(p, end, mag);
        if (q == p) {
            return start;
        }
        // Convert only representable magnitudes; negating INT64_MIN is undefined.
        const fosu_uint64 max_positive = static_cast<fosu_uint64>(int64_max());
        if (neg) {
            out = mag >= max_positive + 1 ? int64_min() : -static_cast<fosu_int64>(mag);
        } else {
            out = mag > max_positive ? int64_max() : static_cast<fosu_int64>(mag);
        }
        return q;
    }

    inline fosu_int32 clamp_i32(fosu_int64 v) {
        if (v > kInt32Max) {
            return kInt32Max;
        }
        if (v < kInt32Min) {
            return kInt32Min;
        }
        return static_cast<fosu_int32>(v);
    }

    // std::clamp (C++17): `low` when v < low, `high` when high < v, otherwise v (so NaN stays NaN).
    template <typename T>
    inline T clamp_value(T v, T low, T high) {
        return v < low ? low : (high < v ? high : v);
    }

    // fast_float-compatible conversion of a decimal at `p` (after an optional '+' handled by the
    // callers). Accepts underflow rounded to signed zero like .NET; rejects overflow.
    inline const char* bounded_double(const char* start, const char* end, double& value) {
        const char* p = start;
        while (p < end && (*p == ' ' || *p == '\t')) {
            ++p;
        }
        if (p < end && *p == '+') {
            ++p;
            if (p < end && (*p == '+' || *p == '-')) {
                return start;
            }
        }
        DecimalNumber number;
        const char* q = scan_decimal(p, end, number);
        if (q == p) {
            return start;
        }
        return decimal_to_double(number, value) ? q : start;
    }

    inline const char* parse_double(const char* p, const char* end, double& out) {
        const char* q = bounded_double(p, end, out);
        return q != p && out == out && out <= std::numeric_limits<double>::max() &&
                       out >= -std::numeric_limits<double>::max()
                   ? q
                   : p;
    }

    inline bool is_numeric_space(char c) { return c == ' ' || (static_cast<unsigned char>(c) - 9u <= 4u); }

    inline const char* skip_numeric_space(const char* p, const char* end) {
        // The input is padded even at end. Digits and field separators take one byte comparison;
        // uncommon control/space bytes use the bounded loop.
        if (static_cast<unsigned char>(*p) > 32) {
            return p;
        }
        while (p < end && is_numeric_space(*p)) {
            ++p;
        }
        return p;
    }

    // Numeric acceptance follows osu.Game Parsing, independently of gameplay clamping or our
    // raw-field storage types.
    inline const char* parse_osu_int(const char* p, const char* end, fosu_int64& out) {
        const char* first = skip_numeric_space(p, end);
        const char* q = parse_i64(first, end, out);
        if (q == first || out < -static_cast<fosu_int64>(kInt32Max) || out > kInt32Max) {
            return p;
        }
        return skip_numeric_space(q, end);
    }

    inline const char* parse_osu_double(const char* p, const char* end, double& out,
                                        double limit = kInt32Max) {
        const char* first = skip_numeric_space(p, end);
        const char* q = bounded_double(first, end, out);
        // One absolute-value bound also rejects infinities and NaN.
        if (q == first || !(std::fabs(out) <= limit)) {
            return p;
        }
        return skip_numeric_space(q, end);
    }

    inline const char* parse_osu_float(const char* p, const char* end, float& out,
                                       float limit = static_cast<float>(kInt32Max)) {
        const char* first = skip_numeric_space(p, end);
        if (first < end && *first == '+') {
            ++first;
            if (first < end && (*first == '+' || *first == '-')) {
                return p;
            }
        }
        DecimalNumber number;
        const char* q = scan_decimal(first, end, number);
        if (q == first || !decimal_to_float(number, out)) {
            return p;
        }
        if (!(std::fabs(out) <= limit)) {
            return p;
        }
        return skip_numeric_space(q, end);
    }

}} // namespace fosu::internal

#endif
