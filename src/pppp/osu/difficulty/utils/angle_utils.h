// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_UTILS_ANGLE_UTILS_H
#define PPPP_OSU_DIFFICULTY_UTILS_ANGLE_UTILS_H

namespace pppp { namespace osu { namespace difficulty { namespace utils {
    struct AngleUtils {
        static double calculate_wideness(double angle);
        static double calculate_acuteness(double angle);
    };
}}}} // namespace pppp::osu::difficulty::utils

#endif
