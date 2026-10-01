// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_TAIKO_PERFORMANCE_CALCULATOR_H
#define PPPP_TAIKO_DIFFICULTY_TAIKO_PERFORMANCE_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/common/score_info.h"
#include "pppp/config.h"
#include "pppp/taiko/difficulty/taiko_difficulty_attributes.h"
#include "pppp/taiko/difficulty/taiko_performance_attributes.h"

namespace pppp { namespace taiko { namespace difficulty {
    Result::Value calculate_performance(TaikoPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                        const TaikoDifficultyAttributes& attributes,
                                        const pppp::beatmaps::Beatmap& beatmap);
}}} // namespace pppp::taiko::difficulty

#endif
