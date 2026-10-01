// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/taiko_hit_windows.h"
#include "pppp/utils/difficulty_range.h"
#include <cmath>

namespace pppp { namespace taiko {
    TaikoHitWindows::TaikoHitWindows()
        : great(0.0),
          ok(0.0),
          miss(0.0) {}

    void TaikoHitWindows::set_difficulty(double difficulty) {
        great = std::floor(utils::difficulty_range(difficulty, GREAT_WINDOW_RANGE)) - 0.5;
        ok = std::floor(utils::difficulty_range(difficulty, OK_WINDOW_RANGE)) - 0.5;
        miss = std::floor(utils::difficulty_range(difficulty, MISS_WINDOW_RANGE)) - 0.5;
    }
}} // namespace pppp::taiko
