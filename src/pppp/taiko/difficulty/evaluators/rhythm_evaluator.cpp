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
        bool is_normal(double x) {
            double magnitude = std::fabs(x);
            return magnitude >= DBL_MIN && magnitude <= DBL_MAX;
        }

        /// Multiplier for a given denominator term.
        double term_penalty(double ratio, int denominator, double power, double multiplier) {
            return -multiplier * pppp::utils::pow(std::cos(denominator * PI * ratio), power);
        }

        /// Calculates the difficulty of a given ratio using a combination of periodic penalties and bonuses.
        double ratio_difficulty(double ratio, int terms = 8) {
            double difficulty = 0;

            // Validate the ratio by ensuring it is a normal number in cases where maps breach regular mapping
            // conditions.
            ratio = is_normal(ratio) ? ratio : 0;

            for (int i = 1; i <= terms; ++i) {
                difficulty += term_penalty(ratio, i, 4, 1);
            }

            difficulty += terms / (1 + ratio);

            // Give bonus to near-1 ratios
            difficulty += pppp::utils::bell_curve(ratio, 1, 0.5);

            // Penalize ratios that are VERY near 1
            difficulty -= pppp::utils::bell_curve(ratio, 1, 0.3);

            difficulty = std::max(difficulty, 0.0);
            difficulty /= std::sqrt(8.0);

            return difficulty;
        }

        double same_interval(const preprocessing::SameRhythmHitObjectGrouping* start_object,
                             int interval_count, double threshold) {
            std::vector<nonstd::optional<double> > intervals;
            const preprocessing::SameRhythmHitObjectGrouping* current_object = start_object;

            for (int i = 0; i < interval_count && current_object != 0; i++) {
                intervals.push_back(current_object->hit_object_interval);
                current_object = current_object->previous;
            }

            std::vector<nonstd::optional<double> > valid;
            for (size_t i = 0; i < intervals.size(); i++) {
                if (intervals[i].has_value()) {
                    valid.push_back(intervals[i]);
                }
            }
            intervals.swap(valid);

            if (static_cast<int>(intervals.size()) < interval_count) {
                return 1.0; // No penalty if there aren't enough valid intervals.
            }

            for (size_t i = 0; i < intervals.size(); i++) {
                for (size_t j = i + 1; j < intervals.size(); j++) {
                    double ratio = intervals[i].value() / intervals[j].value();
                    if (std::fabs(1 - ratio) <=
                        threshold) { // If any two intervals are similar, apply a penalty.
                        return 0.80;
                    }
                }
            }

            return 1.0; // No penalty if all intervals are different.
        }

        /// Determines if the changes in hit object intervals is consistent based on a given threshold.
        double repeated_interval_penalty(
            const preprocessing::SameRhythmHitObjectGrouping& same_rhythm_grouped_hit_objects,
            double hit_window, double threshold = 0.1) {
            double long_interval_penalty = same_interval(&same_rhythm_grouped_hit_objects, 3, threshold);

            double short_interval_penalty =
                same_rhythm_grouped_hit_objects.hit_objects.size() < 6
                    ? same_interval(&same_rhythm_grouped_hit_objects, 4, threshold)
                    : 1.0; // Returns a non-penalty if there are 6 or more notes within an interval.

            // The duration penalty is based on hit object duration relative to hitWindow.
            double duration_penalty =
                std::max(1 - same_rhythm_grouped_hit_objects.duration() * 2 / hit_window, 0.5);

            return std::min(long_interval_penalty, short_interval_penalty) * duration_penalty;
        }

        double evaluate_difficulty_of(
            const preprocessing::SameRhythmHitObjectGrouping& same_rhythm_grouped_hit_objects,
            double hit_window) {
            double interval_difficulty =
                ratio_difficulty(same_rhythm_grouped_hit_objects.hit_object_interval_ratio);
            nonstd::optional<double> previous_interval =
                same_rhythm_grouped_hit_objects.previous != 0
                    ? same_rhythm_grouped_hit_objects.previous->hit_object_interval
                    : nonstd::optional<double>();

            interval_difficulty *= repeated_interval_penalty(same_rhythm_grouped_hit_objects, hit_window);

            // If a previous interval exists and there are multiple hit objects in the sequence:
            if (previous_interval.has_value() && same_rhythm_grouped_hit_objects.hit_objects.size() > 1) {
                double expected_duration_from_previous =
                    previous_interval.value() *
                    static_cast<double>(same_rhythm_grouped_hit_objects.hit_objects.size());
                double duration_difference =
                    same_rhythm_grouped_hit_objects.duration() - expected_duration_from_previous;

                if (duration_difference > 0) {
                    interval_difficulty *=
                        pppp::utils::logistic(duration_difference / hit_window, 0.35, 2, 1);
                }
            }

            // Penalise patterns that can be hit within a single hit window.
            interval_difficulty *=
                pppp::utils::logistic(same_rhythm_grouped_hit_objects.duration() / hit_window, 0.3, 2, 1);

            return pppp::utils::pow(interval_difficulty, 0.75);
        }

        /// Frequent rhythm changes containing long gaps (i.e. 1/4 + 1/6 with 1/2 gaps) award more difficulty
        /// than expected. Due to limitations of the current rhythm evaluation, these cases are targeted and
        /// penalised. The previous hit object grouping is used as often the rhythm change *two* rhythms after
        /// a long gap awards the unexpected difficulty.
        double long_gap_penalty(const preprocessing::SameRhythmHitObjectGrouping* previous) {
            if (previous == 0) {
                return 1.0;
            }

            double gap_interval = previous->first_hit_object()->delta_time;
            double rhythm_interval = previous->hit_object_interval.has_value()
                                         ? previous->hit_object_interval.value()
                                         : gap_interval;
            double rhythm_length = static_cast<double>(previous->hit_objects.size());

            // The ratio of the gap before this rhythm to the rhythm itself.
            double gap_ratio = gap_interval / std::max(rhythm_interval, 1.0);

            // The gap ratio normalised to represent if the gap is long.
            double gap_factor = pppp::utils::logistic(gap_ratio, 1.75, 20);

            // The length in objects of this rhythm normalised to represent if the rhythm change is frequent
            // enough to be penalised.
            double length_factor = pppp::utils::reverse_lerp(rhythm_length, 8, 2);

            return 1.0 - 0.75 * gap_factor * length_factor;
        }
    } // namespace

    double
    RhythmEvaluator::evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& hit_object) {
        if (!hit_object.is_hit()) {
            return 0;
        }

        const preprocessing::RhythmData& rhythm_data = hit_object.rhythm;
        double difficulty = 0.0;

        double same_rhythm = 0;
        double same_pattern = 0;
        double interval_penalty = 0;
        double gap_penalty = 0;

        double hit_window = hit_object.hit_window_great;

        if (rhythm_data.same_rhythm_grouping != 0 &&
            rhythm_data.same_rhythm_grouping->first_hit_object() ==
                &hit_object) { // Difficulty for SameRhythmGroupedHitObjects
            same_rhythm +=
                10.0 * evaluators::evaluate_difficulty_of(*rhythm_data.same_rhythm_grouping, hit_window);
            interval_penalty = repeated_interval_penalty(*rhythm_data.same_rhythm_grouping, hit_window);
            gap_penalty = long_gap_penalty(rhythm_data.same_rhythm_grouping->previous);
        }

        if (rhythm_data.same_patterns_grouping != 0 &&
            rhythm_data.same_patterns_grouping->first_hit_object() ==
                &hit_object) { // Difficulty for SamePatternsGroupedHitObjects
            same_pattern += 1.15 * ratio_difficulty(rhythm_data.same_patterns_grouping->interval_ratio());
        }

        difficulty += std::max(same_rhythm, same_pattern) * interval_penalty * gap_penalty;

        return difficulty;
    }
}}}} // namespace pppp::taiko::difficulty::evaluators
