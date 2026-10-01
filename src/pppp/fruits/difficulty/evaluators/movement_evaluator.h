// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_EVALUATORS_MOVEMENT_EVALUATOR_H
#define PPPP_FRUITS_DIFFICULTY_EVALUATORS_MOVEMENT_EVALUATOR_H

#include "pppp/fruits/difficulty/preprocessing/catch_difficulty_hit_object.h"

namespace pppp { namespace fruits { namespace difficulty { namespace evaluators {
    const double DIRECTION_CHANGE_BONUS = 21.0;

    struct MovementEvaluator {
        static double evaluate_difficulty_of(const preprocessing::CatchDifficultyHitObject& current);
    };
}}}} // namespace pppp::fruits::difficulty::evaluators

#endif
