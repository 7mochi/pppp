#include "pppp/mods/mod_setting_value.h"
#include "pppp/utils/math/decimal.h"
#include <algorithm>

namespace pppp { namespace mods {
    namespace math = ::pppp::utils::math;

    namespace {
        template <typename T>
        T clamp(T value, T min, T max) {
            if (value < min) {
                return min;
            }
            if (value > max) {
                return max;
            }
            return value;
        }

        math::Uint128 power10(int power) {
            math::Uint128 result = math::u128_from(1);
            for (int i = 0; i < power; i++) {
                result = math::u128_mul(result, 10);
            }
            return result;
        }

        math::Decimal round_to(const math::Decimal& value, const math::Decimal& precision) {
            const pppp_uint64 step = precision.mantissa.lo;
            const math::Uint128 numerator = math::u128_mul(value.mantissa, power10(precision.scale).lo);
            const math::Uint128 denominator = math::u128_mul(power10(value.scale), step);

            math::Uint128 quotient;
            math::Uint128 remainder;
            math::u128_divmod(numerator, denominator, &quotient, &remainder);

            const int half = math::u128_cmp(math::u128_shl1(remainder), denominator);
            if (half > 0 || (half == 0 && math::u128_is_odd(quotient))) {
                quotient = math::u128_add(quotient, math::u128_from(1));
            }

            math::Decimal result;
            result.mantissa = math::u128_mul(quotient, step);
            result.scale = precision.scale;
            result.negative = value.negative;
            return result;
        }

        bool round_to_precision(double value, double precision, double* out) {
            math::Decimal num;
            math::Decimal num2;
            if (!math::var_dec_from_r8(precision, &num) || !math::var_dec_from_r8(value, &num2)) {
                return false;
            }
            *out = math::var_r8_from_dec(round_to(num2, num));
            return true;
        }

        bool round_to_precision(float value, float precision, float* out) {
            math::Decimal num;
            math::Decimal num2;
            if (!math::var_dec_from_r4(precision, &num) || !math::var_dec_from_r4(value, &num2)) {
                return false;
            }
            *out = math::var_r4_from_dec(round_to(num2, num));
            return true;
        }

        template <typename T>
        bool set_value_of(T* target, T value, T min_value, T max_value, T precision) {
            T rounded;
            if (!round_to_precision(clamp(value, min_value, max_value), precision, &rounded)) {
                return false;
            }
            *target = clamp(rounded, min_value, max_value);
            return true;
        }
    } // namespace

    bool clamp_and_round(double* target, double value, double min_value, double max_value, double precision) {
        return set_value_of(target, value, min_value, max_value, precision);
    }

    bool clamp_and_round(float* target, float value, float min_value, float max_value, float precision) {
        return set_value_of(target, value, min_value, max_value, precision);
    }

    float clamp_to_extended_limits(float value, float min_value, float max_value,
                                   nonstd::optional<float> extended_min_value,
                                   nonstd::optional<float> extended_max_value) {
        // Ensure that in the case serialisation runs in the wrong order (and limit extensions aren't applied
        // yet) the deserialised value is still propagated.
        float current_min_value =
            clamp(std::min(min_value, value), extended_min_value.value_or(min_value), min_value);
        float current_max_value =
            clamp(std::max(max_value, value), max_value, extended_max_value.value_or(max_value));
        return clamp(value, current_min_value, current_max_value);
    }
}} // namespace pppp::mods
