// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_BASE_PERFORMANCE_H
#define PPPP_OSU_DIFFICULTY_BASE_PERFORMANCE_H

#include "pppp/utils/difficulty_calculation_utils.h"

namespace pppp { namespace osu { namespace difficulty {
    const double PERFORMANCE_BASE_MULTIPLIER = 1.12; // This is being adjusted to keep the final pp value
                                                     // scaled around what it used to be when changing things.
    const double PERFORMANCE_NORM_EXPONENT = 1.1;

    inline double difficulty_to_performance(double difficulty) {
        return 4.0 * pppp::utils::pow(difficulty, 3);
    }
}}} // namespace pppp::osu::difficulty

#endif
