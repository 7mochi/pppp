#ifndef PPPP_UTILS_PRECISION_H
#define PPPP_UTILS_PRECISION_H

#include <cmath>

/// The floating-point precision every ruleset shares: the named constants the calculations measure
/// in, the single-precision cast that pins a computed `double` back to the `float` the game stores,
/// and the three tolerance tests of osu!framework's `Precision` (`AlmostEquals`, `AlmostBigger`,
/// `DefinitelyBigger`).
namespace pppp { namespace utils {
    const double PI_D = 3.14159265358979323846;
    const double E_D = 2.7182818284590452354;
    const float PI_F = 3.14159274f;

    const double SQRT2 = 1.4142135623730950;

    /// Rounds `x` to the single-precision value the game would hold, so a setting that was compared
    /// as a `float` keeps its exact binary representation.
    /// @param x The value to narrow.
    /// @returns `x` rounded to `float` and widened back to `double`.
    inline double f32(double x) {
        float f = static_cast<float>(x);
        return static_cast<double>(f);
    }

    /// The default epsilon for all `float` values.
    const float FLOAT_EPSILON = 1e-3f;

    /// The default epsilon for all `double` values.
    const double DOUBLE_EPSILON = 1e-7;

    /// Computes whether a value is definitely greater than another given an acceptable difference.
    /// @param value1 The value to compare.
    /// @param value2 The value to compare against.
    /// @param acceptable_difference The acceptable difference. Defaults to `DOUBLE_EPSILON`.
    /// @returns Whether `value1` is definitely greater than `value2`.
    inline bool definitely_bigger(double value1, double value2,
                                  double acceptable_difference = DOUBLE_EPSILON) {
        return value1 - acceptable_difference > value2;
    }

    /// Computes whether a value is almost greater than another given an acceptable difference.
    /// @param value1 The value to compare.
    /// @param value2 The value to compare against.
    /// @param acceptable_difference The acceptable difference. Defaults to `DOUBLE_EPSILON`.
    /// @returns Whether `value1` is almost greater than `value2`.
    inline bool almost_bigger(double value1, double value2, double acceptable_difference = DOUBLE_EPSILON) {
        return value1 > value2 - acceptable_difference;
    }

    /// Computes whether two values are equal within an acceptable difference.
    /// @param value1 The first value.
    /// @param value2 The second value.
    /// @param acceptable_difference The acceptable difference. Defaults to `FLOAT_EPSILON`.
    /// @returns Whether `value1` and `value2` are almost equal.
    inline bool almost_equals(float value1, float value2, float acceptable_difference = FLOAT_EPSILON) {
        return std::fabs(value1 - value2) <= acceptable_difference;
    }

    /// Computes whether two values are equal within an acceptable difference.
    /// @param value1 The first value.
    /// @param value2 The second value.
    /// @param acceptable_difference The acceptable difference. Defaults to `DOUBLE_EPSILON`.
    /// @returns Whether `value1` and `value2` are almost equal.
    inline bool almost_equals(double value1, double value2, double acceptable_difference = DOUBLE_EPSILON) {
        return std::fabs(value1 - value2) <= acceptable_difference;
    }
}} // namespace pppp::utils

#endif
