// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/osu_performance_attributes.h"

namespace pppp { namespace osu { namespace difficulty {
    OsuPerformanceAttributes::OsuPerformanceAttributes()
        : aim(0.0),
          speed(0.0),
          accuracy(0.0),
          flashlight(0.0),
          reading(0.0),
          effective_miss_count(0.0),
          combo_based_estimated_miss_count(0.0),
          score_based_estimated_miss_count(),
          aim_estimated_slider_breaks(0.0),
          speed_estimated_slider_breaks(0.0),
          speed_deviation() {}
}}} // namespace pppp::osu::difficulty
