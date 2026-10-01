// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/evaluators/overall_strain_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace mania { namespace difficulty { namespace evaluators {
    double
    OverallStrainEvaluator::evaluate_difficulty_of(const preprocessing::ManiaDifficultyHitObject& current) {
        const double start_time = current.start_time;
        const double end_time = current.end_time;
        bool is_overlapping = false;

        double closest_end_time =
            std::fabs(end_time - start_time); // Lowest value we can assume with the current information
        double hold_factor = 1.0; // Factor to all additional strains in case something else is held
        double hold_addition =
            0; // Addition to the current note in case it's a hold and has to be released awkwardly

        for (size_t i = 0; i < current.previous_in_columns.size(); i++) {
            const preprocessing::ManiaDifficultyHitObject* previous = current.previous_in_columns[i];
            if (!previous) {
                continue;
            }

            // The current note is overlapped if a previous note or end is overlapping the current note body
            if (utils::definitely_bigger(previous->end_time, start_time, 1) &&
                utils::definitely_bigger(end_time, previous->end_time, 1) &&
                utils::definitely_bigger(start_time, previous->start_time, 1)) {
                is_overlapping = true;
            }

            // We give a slight bonus to everything if something is held meanwhile
            if (utils::definitely_bigger(previous->end_time, end_time, 1) &&
                utils::definitely_bigger(start_time, previous->start_time, 1)) {
                hold_factor = 1.25;
            }

            closest_end_time = std::min(closest_end_time, std::fabs(end_time - previous->end_time));
        }

        // The hold addition is given if there was an overlap, however it is only valid if there are no other
        // note with a similar ending. Releasing multiple notes is just as easy as releasing 1. Nerfs the hold
        // addition by half if the closest release is release_threshold away. holdAddition
        //     ^
        // 1.0 + - - - - - -+-----------
        //     |           /
        // 0.5 + - - - - -/   Sigmoid Curve
        //     |         /|
        // 0.0 +--------+-+---------------> Release Difference / ms
        //         release_threshold
        if (is_overlapping) {
            hold_addition = utils::logistic(closest_end_time, RELEASE_THRESHOLD, 0.27);
        }

        return (1 + hold_addition) * hold_factor;
    }
}}}} // namespace pppp::mania::difficulty::evaluators
