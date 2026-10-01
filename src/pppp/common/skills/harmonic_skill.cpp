// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/skills/harmonic_skill.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>
#include <functional>

namespace pppp { namespace common { namespace skills {
    HarmonicSkill::HarmonicSkill(const pppp::mods::Mod* mods_in, size_t mod_count_in,
                                 double harmonic_scale_in, double decay_exponent_in)
        : Skill(mods_in, mod_count_in),
          object_weight_sum(0.0),
          harmonic_scale(harmonic_scale_in),
          decay_exponent(decay_exponent_in) {}

    double HarmonicSkill::process_internal(const preprocessing::DifficultyHitObject& current) {
        return object_difficulty_of(current);
    }

    double HarmonicSkill::difficulty_value() {
        object_weight_sum = 0.0;

        if (object_difficulties.empty()) {
            return 0.0;
        }

        std::vector<double> difficulties = get_transformed_difficulties(object_difficulties);

        std::vector<double> sorted_diffs;
        for (size_t i = 0; i < difficulties.size(); i++) {
            if (difficulties[i] > 0) {
                sorted_diffs.push_back(difficulties[i]);
            }
        }

        double difficulty = 0.0;

        // Objects with 0 difficulty are excluded to avoid worst-case time complexity of the following sort
        // (e.g. /b/2351871). These objects will not contribute to the difficulty.
        std::sort(sorted_diffs.begin(), sorted_diffs.end(), std::greater<double>());
        for (size_t i = 0; i < sorted_diffs.size(); i++) {
            // Use a harmonic sum that considers each object of the map according to a predefined
            // weight.
            double weight = (1.0 + harmonic_scale / (1.0 + static_cast<double>(i))) /
                            (utils::pow(static_cast<double>(i), decay_exponent) + 1.0 +
                             harmonic_scale / (1.0 + static_cast<double>(i)));

            object_weight_sum += weight;

            difficulty += sorted_diffs[i] * weight;
        }

        return difficulty;
    }

    double HarmonicSkill::count_top_weighted_object_difficulties(double difficulty_value) {
        if (object_difficulties.empty() || object_weight_sum == 0) {
            return 0.0;
        }

        double consistent_top_object =
            difficulty_value /
            object_weight_sum; // What would the top difficulty be if all object difficulties were identical

        if (consistent_top_object == 0) {
            return 0.0;
        }

        double sum = 0.0;
        for (size_t i = 0; i < object_difficulties.size(); i++) {
            sum += utils::logistic(object_difficulties[i] / consistent_top_object, 0.88, 10.0, 1.1);
        }
        return sum;
    }
}}} // namespace pppp::common::skills
