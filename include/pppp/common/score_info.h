// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_SCORE_INFO_H
#define PPPP_COMMON_SCORE_INFO_H

#include "pppp/common/hit_result.h"
#include "pppp/config.h"
#include "pppp/mods/mods.h"

namespace pppp { namespace common {
    struct ScoreInfo {
        int statistics[pppp::common::HIT_RESULT_COUNT];
        int maximum_statistics[pppp::common::HIT_RESULT_COUNT];
        int max_combo;
        double accuracy;

        /// Used to preserve the total score for legacy scores.
        /// @remarks Not populated when the score is not a legacy score.
        nonstd::optional<pppp_int64> legacy_total_score;

        pppp::mods::Mods mods;

        ScoreInfo();
    };
}} // namespace pppp::common

#endif
