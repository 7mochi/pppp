// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/utils/angle_utils.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"

namespace pppp { namespace osu { namespace difficulty { namespace utils {
    double AngleUtils::calculate_wideness(double angle) {
        return pppp::utils::smoothstep(angle, pppp::utils::math::degrees_to_radians(40),
                                       pppp::utils::math::degrees_to_radians(140));
    }

    double AngleUtils::calculate_acuteness(double angle) {
        return pppp::utils::smoothstep(angle, pppp::utils::math::degrees_to_radians(140),
                                       pppp::utils::math::degrees_to_radians(40));
    }
}}}} // namespace pppp::osu::difficulty::utils
