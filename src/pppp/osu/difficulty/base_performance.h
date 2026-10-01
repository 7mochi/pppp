#ifndef PPPP_OSU_DIFFICULTY_BASE_PERFORMANCE_H
#define PPPP_OSU_DIFFICULTY_BASE_PERFORMANCE_H

#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"

namespace pppp { namespace osu { namespace difficulty {
    // This is being adjusted to keep the final pp value scaled around what it used to be when
    // changing things.
    const double PERFORMANCE_BASE_MULTIPLIER = 1.12;
    const double PERFORMANCE_NORM_EXPONENT = 1.1;

    inline double difficulty_to_performance(double difficulty) { return 4.0 * utils::pow(difficulty, 3); }

    inline double flashlight_difficulty_to_performance(double difficulty) {
        return 25.0 * utils::pow(difficulty, 2);
    }

    inline double sum_cognition_difficulty(double reading, double flashlight) {
        if (reading <= 0) {
            return flashlight;
        }
        if (flashlight <= 0) {
            return reading;
        }

        double ratio = flashlight / reading;
        double nerfed = flashlight * utils::math::clamp(ratio, 0.25, 1.0);

        // Nerf flashlight value in cognition sum when reading is greater than flashlight
        return utils::norm(PERFORMANCE_NORM_EXPONENT, reading, nerfed);
    }
}}} // namespace pppp::osu::difficulty

#endif
