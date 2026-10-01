// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/osu_hit_windows.h"
#include "pppp/utils/difficulty_range.h"
#include <cmath>

namespace pppp { namespace osu {
    void OsuHitWindows::set_difficulty(double od) {
        great = std::floor(utils::difficulty_range(od, GREAT_WINDOW_RANGE)) - 0.5;
        ok = std::floor(utils::difficulty_range(od, OK_WINDOW_RANGE)) - 0.5;
        meh = std::floor(utils::difficulty_range(od, MEH_WINDOW_RANGE)) - 0.5;
    }

    double OsuHitWindows::window_for(pppp::common::HitResult result) const {
        switch (result) {
        case pppp::common::HIT_RESULT_GREAT: return great;
        case pppp::common::HIT_RESULT_OK: return ok;
        case pppp::common::HIT_RESULT_MEH: return meh;
        case pppp::common::HIT_RESULT_MISS: return MISS_WINDOW;
        default: return 0.0;
        }
    }
}} // namespace pppp::osu
