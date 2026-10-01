// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/skills/speed.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/evaluators/speed/rhythm_evaluator.h"
#include "pppp/osu/difficulty/evaluators/speed/speed_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    Speed::Speed(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : HarmonicSkill(mods_in, mod_count_in, 20.0, 0.9),
          current_strain(0.0) {}

    double Speed::strain_decay(double ms) const { return utils::pow(0.3, ms / 1000.0); }

    double Speed::object_difficulty_of(const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        const double skill_multiplier = 1.16;

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_RX)) {
            return 0;
        }

        double decay = strain_decay(current.adjusted_delta_time);

        current_strain *= decay;
        current_strain += calculate_adjusted_difficulty(current) * (1.0 - decay) * skill_multiplier;

        double current_rhythm = evaluators::speed::RhythmEvaluator::evaluate_difficulty_of(current);

        double total_strain = current_strain * current_rhythm;

        if (current.base_is_slider) {
            slider_strains.push_back(total_strain);
        }

        return total_strain;
    }

    double Speed::calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current) {
        double difficulty = evaluators::speed::SpeedEvaluator::evaluate_difficulty_of(current);

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_AP)) {
            difficulty *= 0.5;
        }

        return difficulty;
    }

    double Speed::relevant_object_count() {
        if (object_difficulties.empty()) {
            return 0;
        }

        double max_strain = *std::max_element(object_difficulties.begin(), object_difficulties.end());

        if (max_strain == 0) {
            return 0;
        }

        double sum = 0.0;
        for (size_t i = 0; i < object_difficulties.size(); i++) {
            sum += utils::logistic(object_difficulties[i] / max_strain, 0.5, 12.0);
        }
        return sum;
    }

    double Speed::count_top_weighted_sliders(double difficulty_value) {
        if (slider_strains.empty()) {
            return 0;
        }

        if (object_weight_sum == 0) {
            return 0.0;
        }

        double consistent_top_object =
            difficulty_value /
            object_weight_sum; // What would the top note be if all note values were identical

        if (consistent_top_object == 0) {
            return 0;
        }

        // Use a weighted sum of all notes. Constants are arbitrary and give nice values
        double sum = 0.0;
        for (size_t i = 0; i < slider_strains.size(); i++) {
            sum += utils::logistic(slider_strains[i] / consistent_top_object, 0.88, 10.0, 1.1);
        }
        return sum;
    }
}}}} // namespace pppp::osu::difficulty::skills
