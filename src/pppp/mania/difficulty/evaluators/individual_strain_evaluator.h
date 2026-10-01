// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_DIFFICULTY_EVALUATORS_INDIVIDUAL_STRAIN_EVALUATOR_H
#define PPPP_MANIA_DIFFICULTY_EVALUATORS_INDIVIDUAL_STRAIN_EVALUATOR_H

#include "pppp/mania/difficulty/preprocessing/mania_difficulty_hit_object.h"

namespace pppp { namespace mania { namespace difficulty { namespace evaluators {
    struct IndividualStrainEvaluator {
        static double evaluate_difficulty_of(const preprocessing::ManiaDifficultyHitObject& current);
    };
}}}} // namespace pppp::mania::difficulty::evaluators

#endif
