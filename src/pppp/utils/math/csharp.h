#ifndef PPPP_UTILS_MATH_CSHARP_H
#define PPPP_UTILS_MATH_CSHARP_H

#include <cmath>

namespace pppp { namespace utils { namespace math {
    inline double round_half_even(double x) {
        double r = std::floor(x);
        double diff = x - r;
        double rounded;
        if (diff > 0.5) {
            rounded = r + 1.0;
        } else if (diff < 0.5) {
            rounded = r;
        } else {
            rounded = (std::fmod(r, 2.0) == 0.0) ? r : r + 1.0;
        }
        return rounded == 0.0 ? x * 0.0 : rounded;
    }

    inline double truncate(double x) { return x < 0.0 ? std::ceil(x) : std::floor(x); }

    inline int round_half_even_int(double x) { return static_cast<int>(round_half_even(x)); }

    inline double round2_to_even(double x) {
        if (std::fabs(x) < 1e16) {
            return round_half_even(x * 100.0) / 100.0;
        }
        return x;
    }

    inline int compare_to(double value, double other) {
        if (value < other) {
            return -1;
        }
        if (value > other) {
            return 1;
        }
        if (value == other) {
            return 0;
        }

        // At least one of the values is NaN.
        if (value != value) {
            return other != other ? 0 : -1;
        }
        return 1;
    }

    inline int sign(float value) { return value > 0 ? 1 : (value < 0 ? -1 : 0); }

    inline int sign(double value) { return value > 0 ? 1 : (value < 0 ? -1 : 0); }

    double clamp(double x, double lo, double hi);

    inline int clamp_int(int value, int low, int high) {
        if (high < low || value < low) {
            return low;
        }
        return value > high ? high : value;
    }

    inline int imin(int a, int b) { return a < b ? a : b; }

    double lerp(double a, double b, double t);

    /// Performs a linear interpolation between two values based on the given weight.
    /// @param value1 The first value, which is intended to be the lower bound.
    /// @param value2 The second value, which is intended to be the upper bound.
    /// @param amount A value, intended to be between 0 and 1, that indicates the weight of the
    /// interpolation.
    /// @returns The interpolated value.
    /// @remarks This method presumes inputs are well formed and does not validate that `value1 < value2`
    /// nor that `0 <= amount <= 1`.
    double lerp_dotnet(double value1, double value2, double amount);

    /// Converts a given value from degrees to radians.
    /// @param degrees The value to convert to radians.
    /// @returns The value of `degrees` converted to radians.
    double degrees_to_radians(double degrees);

    /// 2^(1/3)
    const double CBRT2 = 1.2599210498948731648;

    /// 2^(2/3)
    const double SQR_CBRT2 = 1.5874010519681994748;

    const double CBRT_FACTOR[5] = {1.0 / SQR_CBRT2, 1.0 / CBRT2, 1.0, CBRT2, SQR_CBRT2};

    /// Compute cubic root of double value.
    double cbrt(double x);

    /// 2PI * 2^-64.
    const double SINCOSF_PI63 = 3.4061215800865545e-19;

    /// PI / 4.
    const float SINCOSF_PIO4 = 0.78539818525314331f;

    const double SINCOSF_HPI_HI = 1.5707963267948912;

    const double SINCOSF_HPI_LO = 5.3290705182007514e-15;

    /// Fast sinf implementation. Worst-case ULP is 0.5607, maximum relative error is 0.5303 * 2^-23.
    /// A single-step range reduction is used for small values. Large inputs have their range reduced
    /// using fast integer arithmetic.
    float sinf(float y);

    /// Fast cosf implementation. Worst-case ULP is 0.5607, maximum relative error is 0.5303 * 2^-23.
    /// A single-step range reduction is used for small values. Large inputs have their range reduced
    /// using fast integer arithmetic.
    float cosf(float y);

    /// Correctly-rounded arctangent function of two binary32 values.
    float atan2f(float y, float x);
}}} // namespace pppp::utils::math

#endif
