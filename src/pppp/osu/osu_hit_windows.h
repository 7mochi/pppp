// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OSU_HIT_WINDOWS_H
#define PPPP_OSU_OSU_HIT_WINDOWS_H

#include "pppp/common/hit_result.h"
#include "pppp/utils/difficulty_range.h"

namespace pppp { namespace osu {
    /// osu! ruleset has a fixed miss window regardless of difficulty settings.
    const double MISS_WINDOW = 400.0;

    const utils::DifficultyRange GREAT_WINDOW_RANGE(80.0, 50.0, 20.0);
    const utils::DifficultyRange OK_WINDOW_RANGE(140.0, 100.0, 60.0);
    const utils::DifficultyRange MEH_WINDOW_RANGE(200.0, 150.0, 100.0);

    struct OsuHitWindows {
        double great;
        double ok;
        double meh;

        void set_difficulty(double od);
        double window_for(pppp::common::HitResult result) const;
    };
}} // namespace pppp::osu

#endif
