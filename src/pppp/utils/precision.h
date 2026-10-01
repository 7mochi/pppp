#ifndef PPPP_UTILS_PRECISION_H
#define PPPP_UTILS_PRECISION_H

/// The floating-point precision every ruleset shares: the named constants the calculations measure
/// in, the single-precision cast that pins a computed `double` back to the `float` the game stores,
/// and the three tolerance tests of osu!framework's `Precision` (`AlmostEquals`, `AlmostBigger`,
/// `DefinitelyBigger`). The tests live together because each caller supplies its own acceptable
/// difference — the rulesets use very different ones, from `1e-7` up to a whole millisecond — so
/// the comparison cannot be a single equality and the tolerance must stay the caller's choice.
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

    /// Equal within an acceptable difference each caller supplies.
    /// @param a The left value.
    /// @param b The right value.
    /// @param acceptable_difference How far apart the two may be and still be equal.
    /// @returns Whether the two are within the acceptable difference.
    inline bool almost_equals(double a, double b, double acceptable_difference) {
        return (a > b ? a - b : b - a) <= acceptable_difference;
    }

    /// Greater than, allowing for the caller's acceptable difference.
    /// @param a The left value.
    /// @param b The right value.
    /// @param acceptable_difference How far apart the two may be and still be treated as equal.
    /// @returns Whether `a` is greater than `b` by more than the acceptable difference allows.
    inline bool almost_bigger(double a, double b, double acceptable_difference) {
        return a - b > -acceptable_difference;
    }

    /// Greater than by at least the caller's acceptable difference.
    /// @param a The left value.
    /// @param b The right value.
    /// @param acceptable_difference How far apart the two must be to count as strictly greater.
    /// @returns Whether `a` is greater than `b` by more than the acceptable difference.
    inline bool definitely_bigger(double a, double b, double acceptable_difference) {
        return a - b > acceptable_difference;
    }
}} // namespace pppp::utils

#endif
