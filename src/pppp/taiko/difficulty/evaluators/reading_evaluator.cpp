// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/evaluators/reading_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    double ReadingEvaluator::evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& object) {
        double mid_center = (MID_VELOCITY_MAX + MID_VELOCITY_MIN) / 2.0;
        double mid_range = MID_VELOCITY_MAX - MID_VELOCITY_MIN;
        double high_center = (HIGH_VELOCITY_MAX + HIGH_VELOCITY_MIN) / 2.0;
        double high_range = HIGH_VELOCITY_MAX - HIGH_VELOCITY_MIN;

        // Apply a cap to prevent outlier values on maps that exceed the editor's parameters.
        double effective_bpm = std::max(1.0, object.effective_bpm);

        double mid_velocity_difficulty =
            0.5 * pppp::utils::logistic(effective_bpm, mid_center, 1.0 / (mid_range / 10.0));

        // Expected DeltaTime is the DeltaTime this note would need to be spaced equally to a base slider
        // velocity 1/4 note.
        double expected_delta_time = 21000.0 / effective_bpm;
        double object_density = expected_delta_time / std::max(1.0, object.delta_time);

        // High density is penalised at high velocity as it is generally considered easier to read. See
        // https://www.desmos.com/calculator/u63f3ntdsi
        double density_penalty = pppp::utils::logistic(object_density, 0.925, 15.0);

        double high_velocity_difficulty =
            (1.0 - 0.33 * density_penalty) *
            pppp::utils::logistic(effective_bpm, high_center + 8.0 * density_penalty,
                                  (1.0 + 0.5 * density_penalty) / (high_range / 10.0));

        return mid_velocity_difficulty + high_velocity_difficulty;
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
