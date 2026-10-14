#ifndef PPPP_UTILS_MATH_DECIMAL_H
#define PPPP_UTILS_MATH_DECIMAL_H

#include "pppp/utils/math/uint128.h"

namespace pppp { namespace utils { namespace math {
    const int DEC_SCALE_MAX = 28;

    const unsigned TEN_TO_POWER_NINE = 1000000000u;

    struct Decimal {
        Uint128 mantissa;
        int scale;
        bool negative;
    };

    /// Convert float to Decimal
    bool var_dec_from_r4(float input, Decimal* result);

    /// Convert double to Decimal
    bool var_dec_from_r8(double input, Decimal* result);

    /// Convert Decimal to float
    float var_r4_from_dec(const Decimal& value);

    /// Convert Decimal to double
    double var_r8_from_dec(const Decimal& value);
}}} // namespace pppp::utils::math

#endif
