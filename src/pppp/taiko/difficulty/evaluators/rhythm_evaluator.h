// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_EVALUATORS_RHYTHM_EVALUATOR_H
#define PPPP_TAIKO_DIFFICULTY_EVALUATORS_RHYTHM_EVALUATOR_H

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    const double PI = 3.14159265358979323846;

    struct RhythmEvaluator {
        /// Evaluate the difficulty of a hitobject considering its interval change.
        static double evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& object);
    };
}}}} // namespace pppp::taiko::difficulty::evaluators

#endif
