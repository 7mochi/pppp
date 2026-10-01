// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/evaluators/individual_strain_evaluator.h"
#include "pppp/utils/precision.h"

namespace pppp { namespace mania { namespace difficulty { namespace evaluators {
    double IndividualStrainEvaluator::evaluate_difficulty_of(
        const preprocessing::ManiaDifficultyHitObject& current) {
        const double start_time = current.start_time;
        const double end_time = current.end_time;

        double hold_factor = 1.0; // Factor to all additional strains in case something else is held

        // We award a bonus if this note starts and ends before the end of another hold note.
        for (size_t i = 0; i < current.previous_in_columns.size(); i++) {
            const preprocessing::ManiaDifficultyHitObject* previous = current.previous_in_columns[i];
            if (!previous) {
                continue;
            }

            if (utils::definitely_bigger(previous->end_time, end_time, 1) &&
                utils::definitely_bigger(start_time, previous->start_time, 1)) {
                hold_factor = 1.25;
                break;
            }
        }

        return 2.0 * hold_factor;
    }
}}}} // namespace pppp::mania::difficulty::evaluators
