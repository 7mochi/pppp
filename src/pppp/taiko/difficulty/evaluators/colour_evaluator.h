// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_EVALUATORS_COLOUR_EVALUATOR_H
#define PPPP_TAIKO_DIFFICULTY_EVALUATORS_COLOUR_EVALUATOR_H

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    const double E = 2.7182818284590451;

    struct ColourEvaluator {
        /// Evaluate the difficulty of the first hitobject within a colour streak.
        static double evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& object);
    };
}}}} // namespace pppp::taiko::difficulty::evaluators

#endif
