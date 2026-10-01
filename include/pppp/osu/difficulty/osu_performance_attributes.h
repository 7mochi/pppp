// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_OSU_PERFORMANCE_ATTRIBUTES_H
#define PPPP_OSU_DIFFICULTY_OSU_PERFORMANCE_ATTRIBUTES_H

#include "pppp/common/performance_attributes.h"
#include "pppp/config.h" // IWYU pragma: export

namespace pppp { namespace osu { namespace difficulty {
    struct OsuPerformanceAttributes : pppp::common::PerformanceAttributes {
        double aim;
        double speed;
        double accuracy;
        double flashlight;
        double reading;
        double effective_miss_count;
        double combo_based_estimated_miss_count;
        nonstd::optional<double> score_based_estimated_miss_count;
        double aim_estimated_slider_breaks;
        double speed_estimated_slider_breaks;
        nonstd::optional<double> speed_deviation;

        OsuPerformanceAttributes();
    };
}}} // namespace pppp::osu::difficulty

#endif
