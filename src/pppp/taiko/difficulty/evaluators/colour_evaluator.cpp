// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/evaluators/colour_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/linq/csharp.h"
#include <cmath>
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    namespace linq = ::pppp::utils::linq;

    namespace {
        /// Calculates a consistency penalty based on the number of consecutive consistent intervals,
        /// considering the delta time between each colour sequence.
        /// @param hit_object The current hitObject to consider.
        /// @param threshold  The allowable margin of error for determining whether ratios are consistent.
        /// @param max_objects_to_check The maximum objects to check per count of consistent ratio.
        double consistent_ratio_penalty(const preprocessing::TaikoDifficultyHitObject& hit_object,
                                        double threshold = 0.01, int max_objects_to_check = 64) {
            int consistent_ratio_count = 0;
            double total_ratio_count = 0.0;

            std::vector<double> recent_ratios;
            const preprocessing::TaikoDifficultyHitObject* current = &hit_object;
            const preprocessing::TaikoDifficultyHitObject* previous_hit_object =
                static_cast<const preprocessing::TaikoDifficultyHitObject*>(current->previous(1));

            for (int i = 0; i < max_objects_to_check; i++) {
                // Break if there is no valid previous object
                if (current->index <= 1) {
                    break;
                }

                double current_ratio = current->rhythm.ratio;
                double previous_ratio = previous_hit_object->rhythm.ratio;

                recent_ratios.push_back(current_ratio);

                // A consistent interval is defined as the percentage difference between the two rhythmic
                // ratios with the margin of error.
                if (std::fabs(1 - current_ratio / previous_ratio) <= threshold) {
                    consistent_ratio_count++;
                    total_ratio_count += current_ratio;
                    break;
                }

                current = previous_hit_object;
            }

            // Ensure no division by zero
            if (consistent_ratio_count > 0) {
                return 1 - total_ratio_count / (consistent_ratio_count + 1) * 0.80;
            }

            if (recent_ratios.size() <= 1) {
                return 1.0;
            }

            // As a fallback, calculate the maximum deviation from the average of the recent ratios to ensure
            // slightly off-snapped objects don't bypass the penalty.
            double max_ratio_deviation = linq::from(recent_ratios)
                                             .max(linq::bind(static_cast<double (*)(double)>(&std::fabs),
                                                             linq::_1 - linq::from(recent_ratios).average()));

            double consistent_ratio_penalty =
                0.7 + 0.3 * pppp::utils::smootherstep(max_ratio_deviation, 0.0, 1.0);

            return consistent_ratio_penalty;
        }

        double evaluate_repeating_hit_patterns_difficulty(
            const preprocessing::RepeatingHitPatterns& repeating_hit_pattern) {
            return 2 * (1 - pppp::utils::logistic(E * repeating_hit_pattern.repetition_interval - 2 * E));
        }

        double evaluate_alternating_mono_pattern_difficulty(
            const preprocessing::AlternatingMonoPattern& alternating_mono_pattern) {
            return pppp::utils::logistic(E * alternating_mono_pattern.index - 2 * E) *
                   evaluate_repeating_hit_patterns_difficulty(*alternating_mono_pattern.parent);
        }

        double evaluate_mono_streak_difficulty(const preprocessing::MonoStreak& mono_streak) {
            return pppp::utils::logistic(E * mono_streak.index - 2 * E) *
                   evaluate_alternating_mono_pattern_difficulty(*mono_streak.parent) * 0.5;
        }
    } // namespace

    double
    ColourEvaluator::evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& hit_object) {
        const preprocessing::TaikoDifficultyHitObject& taiko_object = hit_object;
        const preprocessing::ColourData& colour_data = taiko_object.colour;
        double difficulty = 0.0;

        if (colour_data.mono_streak != 0 &&
            colour_data.mono_streak->first_hit_object() == &hit_object) { // Difficulty for MonoStreak
            difficulty += evaluate_mono_streak_difficulty(*colour_data.mono_streak);
        }

        if (colour_data.alternating_mono_pattern != 0 &&
            colour_data.alternating_mono_pattern->first_hit_object() ==
                &hit_object) { // Difficulty for AlternatingMonoPattern
            difficulty += evaluate_alternating_mono_pattern_difficulty(*colour_data.alternating_mono_pattern);
        }

        if (colour_data.repeating_hit_pattern != 0 && colour_data.repeating_hit_pattern->first_hit_object() ==
                                                          &hit_object) { // Difficulty for RepeatingHitPattern
            difficulty += evaluate_repeating_hit_patterns_difficulty(*colour_data.repeating_hit_pattern);
        }

        double consistency_penalty = consistent_ratio_penalty(taiko_object);
        difficulty *= consistency_penalty;

        return difficulty;
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
