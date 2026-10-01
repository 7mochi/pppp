// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_EVALUATORS_SPEED_RHYTHM_EVALUATOR_H
#define PPPP_OSU_DIFFICULTY_EVALUATORS_SPEED_RHYTHM_EVALUATOR_H

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace speed {
    struct RhythmEvaluator {
        /// Calculates a rhythm multiplier for the difficulty of the tap, using the historic data of the
        /// current difficulty hit object.
        static double evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current);
    };
}}}}} // namespace pppp::osu::difficulty::evaluators::speed

#endif
