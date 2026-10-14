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
            interval = std::max(interval, 1.0);

            return 20 / interval;
        }

        /// Determines the number of fingers available to hit the current TaikoDifficultyHitObject.
        /// Any mono notes that is more than 300ms apart from a colour change will be considered to have more
        /// than 2 fingers available, since players can hit the same key with multiple fingers.
        int available_fingers_for(const preprocessing::TaikoDifficultyHitObject& hit_object) {
            const preprocessing::TaikoDifficultyHitObject* previous_colour_change =
                hit_object.colour.previous_colour_change();
            const preprocessing::TaikoDifficultyHitObject* next_colour_change =
                hit_object.colour.next_colour_change();

            if (previous_colour_change != 0 &&
                hit_object.start_time - previous_colour_change->start_time < 300) {
                return 2;
            }

            if (next_colour_change != 0 && next_colour_change->start_time - hit_object.start_time < 300) {
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
        const preprocessing::TaikoDifficultyHitObject& taiko_current = current;
        const preprocessing::TaikoDifficultyHitObject* taiko_previous =
            static_cast<const preprocessing::TaikoDifficultyHitObject*>(current.previous(1));
        const preprocessing::TaikoDifficultyHitObject* previous_mono =
            taiko_current.previous_mono(available_fingers_for(taiko_current) - 1);

        double object_strain = 0.5; // Add a base strain to all objects
        if (taiko_previous == 0) {
            return object_strain;
        }

        if (previous_mono != 0) {
            object_strain += speed_bonus(taiko_current.start_time - previous_mono->start_time) +
                             0.5 * speed_bonus(taiko_current.start_time - taiko_previous->start_time);
        }

        return object_strain;
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
