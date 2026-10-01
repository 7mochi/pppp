// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/evaluators/stamina_evaluator.h"
#include <algorithm>

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    namespace {
        /// Applies a speed bonus dependent on the time since the last hit performed using this finger.
        /// @param interval The interval between the current and previous note hit using the same finger.
        double speed_bonus(double interval) {
            // Interval is capped at a very small value to prevent infinite values.
            return 20.0 / std::max(interval, 1.0);
        }

        int available_fingers_for(const preprocessing::TaikoDifficultyHitObject& object) {
            const preprocessing::TaikoDifficultyHitObject* previous_change =
                object.colour.previous_colour_change();
            if (previous_change && object.start_time - previous_change->start_time < 300.0) {
                return 2;
            }

            const preprocessing::TaikoDifficultyHitObject* next_change = object.colour.next_colour_change();
            if (next_change && next_change->start_time - object.start_time < 300.0) {
                return 2;
            }

            return 8;
        }
    } // namespace

    double StaminaEvaluator::evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& current) {
        if (!current.is_hit()) {
            return 0.0;
        }

        // Find the previous hit object hit by the current finger, which is n notes prior, n being the number
        // of available fingers.
        const preprocessing::TaikoDifficultyHitObject* previous =
            static_cast<const preprocessing::TaikoDifficultyHitObject*>(current.previous(1));
        const preprocessing::TaikoDifficultyHitObject* previous_same_finger =
            current.previous_mono(available_fingers_for(current) - 1);

        double object_strain = 0.5; // Add a base strain to all objects
        if (!previous) {
            return object_strain;
        }

        if (previous_same_finger) {
            object_strain += speed_bonus(current.start_time - previous_same_finger->start_time) +
                             0.5 * speed_bonus(current.start_time - previous->start_time);
        }

        return object_strain;
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
