#ifndef PPPP_UTILS_MATH_CSHARP_H
#define PPPP_UTILS_MATH_CSHARP_H

#include <cmath>

namespace pppp { namespace utils { namespace math {
    /// Rounds `x` to the nearest integer, breaking a tie towards the even neighbour.
    /// @param x The value to round.
    /// @returns The rounded value.
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
        // `Math.Round` keeps the sign of its argument when it rounds to zero: -0.4 gives -0.0.
        return rounded == 0.0 ? x * 0.0 : rounded;
    }

    /// Rounds `x` half-even and truncates the result to `int`.
    /// @param x The value to round.
    /// @returns The rounded value.
    inline int round_half_even_int(double x) { return static_cast<int>(round_half_even(x)); }

    /// Rounds `x` to two decimals, breaking a tie towards the even neighbour. From 1e16 up a double
    /// has no decimals left, and `Math.Round(x, 2)` returns it untouched instead of scaling it, which
    /// near the top of the range would overflow to infinity.
    /// @param x The value to round.
    /// @returns The rounded value.
    inline double round2_to_even(double x) {
        if (std::fabs(x) < 1e16) {
            return round_half_even(x * 100.0) / 100.0;
        }
        return x;
    }

    /// https://github.com/dotnet/runtime/blob/26d2fafe1c84107e03ccdd8d6d8974ee7b49d9a5/src/libraries/System.Private.CoreLib/src/System/Double.cs#L258
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

    /// Takes a float and answers -1, 0 or 1.
    inline int sign(float value) { return value > 0 ? 1 : (value < 0 ? -1 : 0); }

    /// Takes a double and answers -1, 0 or 1.
    inline int sign(double value) { return value > 0 ? 1 : (value < 0 ? -1 : 0); }

    /// Clamps `x` into `[lo, hi]`.
    /// @param x The value to clamp.
    /// @param lo The inclusive lower bound.
    /// @param hi The inclusive upper bound.
    /// @returns The clamped value.
    double clamp(double x, double lo, double hi);

    /// Clamps `value` into `[low, high]`, preferring the lower bound when the bounds are inverted.
    /// @param value The value to clamp.
    /// @param low The lower bound.
    /// @param high The upper bound.
    /// @returns The clamped value.
    inline int clamp_int(int value, int low, int high) {
        if (high < low || value < low) {
            return low;
        }
        return value > high ? high : value;
    }

    /// The lesser of two integers.
    inline int imin(int a, int b) { return a < b ? a : b; }

    /// The greater of two integers.
    inline int imax(int a, int b) { return a > b ? a : b; }

    /// `a - b`, floored at zero.
    inline int sub_sat(int a, int b) { return a > b ? a - b : 0; }

    /// Linear interpolation, `a + (b - a) * t`.
    /// @param a The value at `t = 0`.
    /// @param b The value at `t = 1`.
    /// @param t The weight.
    /// @returns The interpolated value.
    double lerp(double a, double b, double t);

    /// Linear interpolation in the framework's own form,
    /// `value1 * (1 - amount) + value2 * amount`, which rounds differently from the `a + (b - a) * t`
    /// form next to it. The settings that go through this form keep their last bits only if it is the
    /// one that runs:
    /// https://github.com/dotnet/runtime/blob/1d1bf92fcf43aa6981804dc53c5174445069c9e4/src/libraries/System.Private.CoreLib/src/System/Double.cs#L841
    /// @param a The value at `t = 0`.
    /// @param b The value at `t = 1`.
    /// @param t The weight.
    /// @returns The interpolated value.
    double lerp_dotnet(double a, double b, double t);

    /// Converts degrees to radians.
    /// @param deg The angle in degrees.
    /// @returns The angle in radians.
    double degrees_to_radians(double deg);

    /// 2^(1/3)
    const double CBRT2 = 1.2599210498948731648;

    /// 2^(2/3)
    const double SQR_CBRT2 = 1.5874010519681994748;

    /// glibc's `factor`: 2^(e/3) for the remainders -2 to 2 of the exponent divided by three.
    const double CBRT_FACTOR[5] = {1.0 / SQR_CBRT2, 1.0 / CBRT2, 1.0, CBRT2, SQR_CBRT2};

    /// The cube root. `Math.Cbrt` is the platform libm's, and parity is proven against .NET 8 on
    /// glibc, so this is glibc's own algorithm, reproduced bit for bit rather than taken from the
    /// target's libm, where `cbrt` is C99 and differs between libms:
    /// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/dbl-64/s_cbrt.c;hb=refs/tags/glibc-2.42
    /// `pow(x, 1.0 / 3.0)` is not the same function: it is off by an ulp on four inputs in five.
    /// @param x The value to take the root of.
    /// @returns The cube root of `x`.
    double cbrt(double x);
}}} // namespace pppp::utils::math

#endif
