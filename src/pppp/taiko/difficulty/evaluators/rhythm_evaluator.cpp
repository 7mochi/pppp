// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/evaluators/rhythm_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    namespace {
        // Finite and in range, which is what the ratio of two delta times has to be before the
        // series below can be evaluated.
        bool is_normal(double x) {
            double magnitude = std::fabs(x);
            return magnitude >= DBL_MIN && magnitude <= DBL_MAX;
        }

        double term_penalty(double ratio, int denominator, double power, double multiplier) {
            return -multiplier * pppp::utils::pow(std::cos(denominator * PI * ratio), power);
        }

        double ratio_difficulty(double ratio) {
            const int terms = 8;
            double difficulty = 0.0;

            // Validate the ratio by ensuring it is a normal number in cases where maps breach regular
            // mapping conditions.
            if (!is_normal(ratio)) {
                ratio = 0.0;
            }

            for (int i = 1; i <= terms; i++) {
                difficulty += term_penalty(ratio, i, 4.0, 1.0);
            }

            difficulty += terms / (1.0 + ratio);

            // Give bonus to near-1 ratios
            difficulty += pppp::utils::bell_curve(ratio, 1.0, 0.5);

            // Penalize ratios that are VERY near 1
            difficulty -= pppp::utils::bell_curve(ratio, 1.0, 0.3);

            difficulty = std::max(difficulty, 0.0);
            difficulty /= std::sqrt(8.0);

            return difficulty;
        }

        // Penalises a grouping whose interval has been seen recently.
        double same_interval(const preprocessing::SameRhythmHitObjectGrouping& start_group,
                             int interval_count, double threshold) {
            std::vector<double> intervals;
            const preprocessing::SameRhythmHitObjectGrouping* current = &start_group;

            for (int i = 0; i < interval_count && current; i++) {
                if (current->hit_object_interval.has_value()) {
                    intervals.push_back(current->hit_object_interval.value());
                }
                current = current->previous;
            }

            if (static_cast<int>(intervals.size()) < interval_count) {
                return 1.0; // No penalty if there aren't enough valid intervals.
            }

            for (size_t i = 0; i < intervals.size(); i++) {
                for (size_t j = i + 1; j < intervals.size(); j++) {
                    double ratio = intervals[i] / intervals[j];
                    // If any two intervals are similar, apply a penalty.
                    if (std::fabs(1.0 - ratio) <= threshold) {
                        return 0.80;
                    }
                }
            }

            return 1.0; // No penalty if all intervals are different.
        }

        double repeated_interval_penalty(const preprocessing::SameRhythmHitObjectGrouping& grouping,
                                         double hit_window) {
            const double threshold = 0.1;

            double long_interval_penalty = same_interval(grouping, 3, threshold);
            // Returns a non-penalty if there are 6 or more notes within an interval.
            double short_interval_penalty =
                grouping.hit_objects.size() < 6 ? same_interval(grouping, 4, threshold) : 1.0;

            // The duration penalty is based on hit object duration relative to hitWindow.
            double duration_penalty = std::max(1.0 - grouping.duration() * 2.0 / hit_window, 0.5);

            return std::min(long_interval_penalty, short_interval_penalty) * duration_penalty;
        }

        double evaluate_same_rhythm_grouping(const preprocessing::SameRhythmHitObjectGrouping& grouping,
                                             double hit_window) {
            double interval_difficulty = ratio_difficulty(grouping.hit_object_interval_ratio);
            interval_difficulty *= repeated_interval_penalty(grouping, hit_window);

            const preprocessing::SameRhythmHitObjectGrouping* previous = grouping.previous;

            // If a previous interval exists and there are multiple hit objects in the sequence:
            if (previous && previous->hit_object_interval.has_value() && grouping.hit_objects.size() > 1) {
                double expected_duration =
                    previous->hit_object_interval.value() * static_cast<double>(grouping.hit_objects.size());
                double duration_difference = grouping.duration() - expected_duration;

                if (duration_difference > 0) {
                    interval_difficulty *=
                        pppp::utils::logistic(duration_difference / hit_window, 0.35, 2.0, 1.0);
                }
            }

            // Penalise patterns that can be hit within a single hit window.
            interval_difficulty *= pppp::utils::logistic(grouping.duration() / hit_window, 0.3, 2.0, 1.0);

            return pppp::utils::pow(interval_difficulty, 0.75);
        }

        double long_gap_penalty(const preprocessing::SameRhythmHitObjectGrouping* previous) {
            if (!previous) {
                return 1.0;
            }

            double gap_interval = previous->hit_objects[0]->delta_time;
            double rhythm_interval = previous->hit_object_interval.has_value()
                                         ? previous->hit_object_interval.value()
                                         : gap_interval;
            double rhythm_length = static_cast<double>(previous->hit_objects.size());

            // The ratio of the gap before this rhythm to the rhythm itself.
            double gap_ratio = gap_interval / std::max(rhythm_interval, 1.0);

            // The gap ratio normalised to represent if the gap is long.
            double gap_factor = pppp::utils::logistic(gap_ratio, 1.75, 20.0);

            // The length in objects of this rhythm normalised to represent if the rhythm change is frequent
            // enough to be penalised.
            double length_factor = pppp::utils::reverse_lerp(rhythm_length, 8.0, 2.0);

            return 1.0 - 0.75 * gap_factor * length_factor;
        }
    } // namespace

    double RhythmEvaluator::evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& object) {
        if (!object.is_hit()) {
            return 0.0;
        }

        double same_rhythm = 0.0;
        double same_pattern = 0.0;
        double interval_penalty = 0.0;
        double gap_penalty = 0.0;
        double hit_window = object.hit_window_great;

        const preprocessing::SameRhythmHitObjectGrouping* rhythm_group = object.rhythm.same_rhythm_grouping;
        // Difficulty for SameRhythmGroupedHitObjects
        if (rhythm_group && rhythm_group->hit_objects[0] == &object) {
            same_rhythm += 10.0 * evaluate_same_rhythm_grouping(*rhythm_group, hit_window);
            interval_penalty = repeated_interval_penalty(*rhythm_group, hit_window);
            gap_penalty = long_gap_penalty(rhythm_group->previous);
        }

        const preprocessing::SamePatternsGroupedHitObjects* pattern_group =
            object.rhythm.same_patterns_grouping;
        if (pattern_group) {
            const preprocessing::TaikoDifficultyHitObject* first_object =
                pattern_group->groups[0]->hit_objects[0];
            if (first_object == &object) { // Difficulty for SamePatternsGroupedHitObjects
                same_pattern += 1.15 * ratio_difficulty(pattern_group->interval_ratio());
            }
        }

        return std::max(same_rhythm, same_pattern) * interval_penalty * gap_penalty;
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
