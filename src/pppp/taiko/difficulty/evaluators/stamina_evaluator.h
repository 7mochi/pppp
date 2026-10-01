// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_EVALUATORS_STAMINA_EVALUATOR_H
#define PPPP_TAIKO_DIFFICULTY_EVALUATORS_STAMINA_EVALUATOR_H

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    struct StaminaEvaluator {
        /// Evaluates the minimum mechanical stamina required to play the current object. This is
        /// calculated using the maximum possible interval between two hits using the same key, by
        /// alternating available fingers for each colour.
        static double evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& current);
    };
}}}} // namespace pppp::taiko::difficulty::evaluators

#endif
