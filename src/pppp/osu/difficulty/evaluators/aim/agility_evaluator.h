// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_EVALUATORS_AIM_AGILITY_EVALUATOR_H
#define PPPP_OSU_DIFFICULTY_EVALUATORS_AIM_AGILITY_EVALUATOR_H

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    struct AgilityEvaluator {
        /// Evaluates the difficulty of fast aiming
        static double evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current);
    };
}}}}} // namespace pppp::osu::difficulty::evaluators::aim

#endif
