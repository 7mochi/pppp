// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/osu_legacy_score_miss_calculator.h"
#include "pppp/osu/difficulty/legacy_score_simulator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>

namespace pppp { namespace osu { namespace difficulty {
    int OsuLegacyScoreMissCalculator::count_of(int result) const { return score.statistics[result]; }

    double OsuLegacyScoreMissCalculator::calculate() const {
        if (attributes.max_combo == 0 || !score.legacy_total_score.has_value()) {
            return 0;
        }

        double score_v1_multiplier = attributes.legacy_score_base_multiplier *
                                     get_legacy_score_multiplier(score.mods.data(), score.mods.size());
        double relevant_combo_per_object = calculate_relevant_score_combo_per_object();

        double maximum_miss_count = calculate_maximum_combo_based_miss_count();

        double score_obtained_during_max_combo =
            calculate_score_at_combo(score.max_combo, relevant_combo_per_object, score_v1_multiplier);
        double remaining_score =
            static_cast<double>(score.legacy_total_score.value()) - score_obtained_during_max_combo;

        if (remaining_score <= 0) {
            return maximum_miss_count;
        }

        double remaining_combo = attributes.max_combo - score.max_combo;
        double expected_remaining_score =
            calculate_score_at_combo(remaining_combo, relevant_combo_per_object, score_v1_multiplier);

        double score_based_miss_count = expected_remaining_score / remaining_score;

        // If there's less then one miss detected - let combo-based miss count decide if this is FC or not
        score_based_miss_count = std::max(score_based_miss_count, 1.0);

        // Cap result by very harsh version of combo-based miss count
        return std::min(score_based_miss_count, maximum_miss_count);
    }

    double OsuLegacyScoreMissCalculator::calculate_score_at_combo(double combo,
                                                                  double relevant_combo_per_object,
                                                                  double score_v1_multiplier) const {
        int count_miss = count_of(pppp::common::HIT_RESULT_MISS);

        int total_hits = count_of(pppp::common::HIT_RESULT_GREAT) + count_of(pppp::common::HIT_RESULT_OK) +
                         count_of(pppp::common::HIT_RESULT_MEH) + count_miss;

        double estimated_objects = combo / relevant_combo_per_object - 1;

        // The combo portion of ScoreV1 follows arithmetic progression
        // Therefore, we calculate the combo portion of score using the combo per object and our current
        // combo.
        double combo_score = relevant_combo_per_object > 0
                                 ? (2 * (relevant_combo_per_object - 1) +
                                    (estimated_objects - 1) * relevant_combo_per_object) *
                                       estimated_objects / 2
                                 : 0;

        // We then apply the accuracy and ScoreV1 multipliers to the resulting score.
        combo_score *= score.accuracy * 300 / 25 * score_v1_multiplier;

        double objects_hit = (total_hits - count_miss) * combo / attributes.max_combo;

        // Score also has a non-combo portion we need to create the final score value.
        double non_combo_score = (300 + attributes.nested_score_per_object) * score.accuracy * objects_hit;

        return combo_score + non_combo_score;
    }

    double OsuLegacyScoreMissCalculator::calculate_relevant_score_combo_per_object() const {
        double combo_score = attributes.maximum_legacy_combo_score;

        // We then reverse apply the ScoreV1 multipliers to get the raw value.
        combo_score /= 300.0 / 25.0 * attributes.legacy_score_base_multiplier;

        // Reverse the arithmetic progression to work out the amount of combo per object based on
        // the score.
        unsigned int wrapped = static_cast<unsigned int>(attributes.max_combo - 2) *
                               static_cast<unsigned int>(attributes.max_combo);
        double result = static_cast<int>(wrapped);
        result /= std::max(attributes.max_combo + 2 * (combo_score - 1), 1.0);

        return result;
    }

    double OsuLegacyScoreMissCalculator::calculate_maximum_combo_based_miss_count() const {
        int count_miss = count_of(pppp::common::HIT_RESULT_MISS);
        int count_ok = count_of(pppp::common::HIT_RESULT_OK);
        int count_meh = count_of(pppp::common::HIT_RESULT_MEH);

        if (attributes.slider_count <= 0) {
            return count_miss;
        }

        int total_imperfect_hits = count_ok + count_meh + count_miss;

        double miss_count = 0;

        // If sliders in the map are hard - it's likely for player to drop sliderends
        // If map has easy sliders - it's more likely for player to sliderbreak
        double likely_missed_sliderend_portion =
            0.04 + 0.06 * utils::pow(std::min(attributes.aim_top_weighted_slider_factor, 1.0), 2);

        // Consider that full combo is maximum combo minus dropped slider tails since they don't
        // contribute to combo but also don't break it. In classic scores we can't know the amount
        // of dropped sliders so we estimate it
        double full_combo_threshold =
            attributes.max_combo - std::min(4 + likely_missed_sliderend_portion * attributes.slider_count,
                                            static_cast<double>(attributes.slider_count));

        if (score.max_combo < full_combo_threshold) {
            miss_count =
                utils::pow(full_combo_threshold / std::max(1.0, static_cast<double>(score.max_combo)), 2.5);
        }

        // In classic scores there can't be more misses than a sum of all non-perfect judgements
        miss_count = std::min(miss_count, static_cast<double>(total_imperfect_hits));

        // Every slider has *at least* 2 combo attributed in classic mechanics.
        // If they broke on a slider with a tick, then this still works since they would have lost
        // at least 2 combo (the tick and the end) Using this as a max means a score that loses 1
        // combo on a map can't possibly have been a slider break. It must have been a slider end.
        int max_possible_slider_breaks =
            std::min(attributes.slider_count, (attributes.max_combo - score.max_combo) / 2);

        double slider_breaks = miss_count - count_miss;

        if (slider_breaks > max_possible_slider_breaks) {
            miss_count = count_miss + max_possible_slider_breaks;
        }

        return miss_count;
    }
}}} // namespace pppp::osu::difficulty
