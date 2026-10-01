#ifndef PPPP_UTILS_MATH_LEGACY_SCORE_H
#define PPPP_UTILS_MATH_LEGACY_SCORE_H

#include "pppp/config.h"

namespace pppp { namespace utils { namespace math {
    /// Scales a `double` to a fixed-point `pppp_uint64` with fifteen significant digits, reporting
    /// the sign through `negative`. The legacy score simulation converts each of a beatmap's
    /// difficulty settings this way before going on in integers, because the peppy-star routine it
    /// reproduces used a wider decimal type than `double` and the result must land on the same
    /// rounded value. It is this unit's own decimal arithmetic, not the framework's.
    /// @param x The value to scale.
    /// @param negative Set to whether `x` was negative; the returned magnitude has no sign.
    /// @returns The magnitude of `x`, scaled to fifteen significant digits.
    pppp_uint64 decimal15_scaled(double x, bool* negative);
}}} // namespace pppp::utils::math

#endif
