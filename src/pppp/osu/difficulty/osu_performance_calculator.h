// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_OSU_PERFORMANCE_CALCULATOR_H
#define PPPP_OSU_DIFFICULTY_OSU_PERFORMANCE_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/common/score_info.h"
#include "pppp/config.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/osu_performance_attributes.h"

namespace pppp { namespace osu { namespace difficulty {
    Result::Value calculate_performance(OsuPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                        const OsuDifficultyAttributes& attributes,
                                        const pppp::beatmaps::Beatmap& beatmap);
}}} // namespace pppp::osu::difficulty

#endif
