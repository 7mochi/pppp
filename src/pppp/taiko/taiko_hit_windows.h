// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_TAIKO_HIT_WINDOWS_H
#define PPPP_TAIKO_TAIKO_HIT_WINDOWS_H

#include "pppp/utils/difficulty_range.h"

namespace pppp { namespace taiko {
    const utils::DifficultyRange GREAT_WINDOW_RANGE(50.0, 35.0, 20.0);
    const utils::DifficultyRange OK_WINDOW_RANGE(120.0, 80.0, 50.0);
    const utils::DifficultyRange MISS_WINDOW_RANGE(135.0, 95.0, 70.0);

    struct TaikoHitWindows {
        double great;
        double ok;
        double miss;

        TaikoHitWindows();

        void set_difficulty(double difficulty);
    };
}} // namespace pppp::taiko

#endif
