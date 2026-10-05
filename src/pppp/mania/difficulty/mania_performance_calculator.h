// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_DIFFICULTY_MANIA_PERFORMANCE_CALCULATOR_H
#define PPPP_MANIA_DIFFICULTY_MANIA_PERFORMANCE_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/common/score_info.h"
#include "pppp/mania/difficulty/mania_difficulty_attributes.h"
#include "pppp/mania/difficulty/mania_performance_attributes.h"
#include "pppp/status.h"

namespace pppp { namespace mania { namespace difficulty {
    Status calculate_performance(ManiaPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                 const ManiaDifficultyAttributes& attributes,
                                 const pppp::beatmaps::Beatmap& beatmap);
}}} // namespace pppp::mania::difficulty

#endif
