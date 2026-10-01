// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_CATCH_PERFORMANCE_CALCULATOR_H
#define PPPP_FRUITS_DIFFICULTY_CATCH_PERFORMANCE_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/common/score_info.h"
#include "pppp/config.h"
#include "pppp/fruits/difficulty/catch_difficulty_attributes.h"
#include "pppp/fruits/difficulty/catch_performance_attributes.h"

namespace pppp { namespace fruits { namespace difficulty {
    Result::Value calculate_performance(CatchPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                        const CatchDifficultyAttributes& attributes,
                                        const pppp::beatmaps::Beatmap& beatmap);
}}} // namespace pppp::fruits::difficulty

#endif
