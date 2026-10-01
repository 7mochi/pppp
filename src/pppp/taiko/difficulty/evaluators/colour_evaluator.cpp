// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/evaluators/colour_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <cmath>
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    namespace {
        double evaluate_repeating_hit_patterns(const preprocessing::RepeatingHitPatterns& pattern) {
            return 2.0 * (1.0 - pppp::utils::logistic(E * pattern.repetition_interval - 2.0 * E));
        }

        double evaluate_alternating_mono_pattern(const preprocessing::AlternatingMonoPattern& mono_pattern) {
            return pppp::utils::logistic(E * mono_pattern.index - 2.0 * E) *
                   evaluate_repeating_hit_patterns(*mono_pattern.parent);
        }

        double evaluate_mono_streak(const preprocessing::MonoStreak& mono_streak) {
            return pppp::utils::logistic(E * mono_streak.index - 2.0 * E) *
                   evaluate_alternating_mono_pattern(*mono_streak.parent) * 0.5;
        }

        /// Calculates a consistency penalty based on the number of consecutive consistent intervals,
        /// considering the delta time between each colour sequence.
        /// @param object The current hit object to consider.
        double consistent_ratio_penalty(const preprocessing::TaikoDifficultyHitObject& object) {
            const double threshold = 0.01;
            const int max_objects_to_check = 64;

            int consistent_ratio_count = 0;
            double total_ratio_count = 0.0;
            std::vector<double> recent_ratios;

            const preprocessing::TaikoDifficultyHitObject* current = &object;
            const preprocessing::TaikoDifficultyHitObject* previous =
                static_cast<const preprocessing::TaikoDifficultyHitObject*>(current->previous(1));

            for (int i = 0; i < max_objects_to_check; i++) {
                // Break if there is no valid previous object
                if (current->index <= 1) {
                    break;
                }

                double current_ratio = current->rhythm.ratio;
                double previous_ratio = previous->rhythm.ratio;

                recent_ratios.push_back(current_ratio);

                // A consistent interval is defined as the percentage difference between the two rhythmic
                // ratios with the margin of error.
                if (std::fabs(1.0 - current_ratio / previous_ratio) <= threshold) {
                    consistent_ratio_count++;
                    total_ratio_count += current_ratio;
                    break;
                }

                current = previous;
            }

            // Ensure no division by zero
            if (consistent_ratio_count > 0) {
                return 1.0 - total_ratio_count / (consistent_ratio_count + 1) * 0.80;
            }

            if (recent_ratios.size() <= 1) {
                return 1.0;
            }

            double sum = 0.0;
            for (size_t i = 0; i < recent_ratios.size(); i++) {
                sum += recent_ratios[i];
            }
            double average = sum / static_cast<double>(recent_ratios.size());

            // As a fallback, calculate the maximum deviation from the average of the recent ratios to ensure
            // slightly off-snapped objects don't bypass the penalty.
            double max_deviation = 0.0;
            for (size_t i = 0; i < recent_ratios.size(); i++) {
                double deviation = std::fabs(recent_ratios[i] - average);
                if (i == 0 || deviation > max_deviation) {
                    max_deviation = deviation;
                }
            }

            return 0.7 + 0.3 * pppp::utils::smootherstep(max_deviation, 0.0, 1.0);
        }
    } // namespace

    double ColourEvaluator::evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& object) {
        const preprocessing::ColourData& colour = object.colour;
        double difficulty = 0.0;

        // Only the first object of each encoding carries that encoding's difficulty.
        // Difficulty for MonoStreak
        if (colour.mono_streak && colour.mono_streak->hit_objects[0] == &object) {
            difficulty += evaluate_mono_streak(*colour.mono_streak);
        }

        if (colour.alternating_mono_pattern) {
            // Difficulty for AlternatingMonoPattern
            if (colour.alternating_mono_pattern->mono_streaks[0]->hit_objects[0] == &object) {
                difficulty += evaluate_alternating_mono_pattern(*colour.alternating_mono_pattern);
            }
        }

        if (colour.repeating_hit_pattern) {
            const preprocessing::AlternatingMonoPattern* first =
                colour.repeating_hit_pattern->alternating_mono_patterns[0];
            // Difficulty for RepeatingHitPattern
            if (first->mono_streaks[0]->hit_objects[0] == &object) {
                difficulty += evaluate_repeating_hit_patterns(*colour.repeating_hit_pattern);
            }
        }

        return difficulty * consistent_ratio_penalty(object);
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
