#ifndef FOSU_ENGINE_PARSING_DECIMAL_H
#define FOSU_ENGINE_PARSING_DECIMAL_H

// Exact (correctly rounded) decimal -> binary32/binary64 conversion, the C++98 replacement of
// fast_float: the same accepted syntax and the same values, including underflow to ±0 and
// overflow rejection.

#include <cstddef>

#include <fosu/compiler.h>

namespace fosu { namespace internal {

    // A scanned decimal number: [-]integer_digits[.fraction_digits][(e|E)[+|-]exponent_digits].
    struct DecimalNumber {
        const char* integer_begin;
        size_t integer_length;
        const char* fraction_begin;
        size_t fraction_length;
        fosu_int64 exponent; // explicit exponent (0 when absent), magnitude capped like fast_float
        bool negative;
    };

    // Scans a number with fast_float's general syntax (an optional leading '-' only; a trailing 'e'
    // without digits is not consumed). Returns the end of the number, or `p` when there is no digit
    // at all. "inf"/"nan" spellings are not accepted (every caller rejects non-finite values).
    const char* scan_decimal(const char* p, const char* end, DecimalNumber& out);

    // Correctly rounded conversions. Return false on overflow (|value| beyond the type); a value
    // that rounds to zero is accepted as ±0 (true).
    bool decimal_to_double(const DecimalNumber& number, double& out);
    bool decimal_to_float(const DecimalNumber& number, float& out);

}} // namespace fosu::internal

#endif
