// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/skills/reading.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/evaluators/reading_evaluator.h"
#include "pppp/osu/mods/osu_mod_hidden.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/linq/csharp.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    namespace linq = ::pppp::utils::linq;
    namespace math = ::pppp::utils::math;

    Reading::Reading(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : HarmonicSkill(mods_in, mod_count_in),
          has_hidden_mod(false),
          current_strain(0.0),
          reduced_note_count(0.0),
          reduced_duration() {
        has_hidden_mod = mods::has_hidden_body_fade(mods, mod_count);
    }

    double Reading::strain_decay(double ms) const { return utils::pow(0.8, ms / 1000.0); }

    double Reading::object_difficulty_of(const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        const double skill_multiplier = 2.5;
        const double reduced_difficulty_duration = 60.0 * 1000.0;

        double decay = strain_decay(current.delta_time);

        current_strain *= decay;
        current_strain += calculate_adjusted_difficulty(current) * (1.0 - decay) * skill_multiplier;

        // This operates under the assumption that the object difficulty is calculated once per object and
        // in order. Under that assumption, the current object's start time refers to the start time of the
        // first object while the reduced duration is yet to be set.
        if (!reduced_duration.has_value()) {
            reduced_duration = current.start_time + reduced_difficulty_duration;
        }

        // This relies on the same assumption, as calling in order means that we can safely increase the note
        // count until we reach the first object after the reduced duration.
        if (current.start_time <= reduced_duration.value()) {
            reduced_note_count++;
        }

        return current_strain;
    }

    double Reading::calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current) {
        double difficulty = evaluators::ReadingEvaluator::evaluate_difficulty_of(current, has_hidden_mod);

        if (linq::from(mods, mod_count)
                .any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_TD)) {
            difficulty = utils::pow(difficulty, 0.89);
        }

        if (linq::from(mods, mod_count)
                .any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_MG)) {
            double magnetised_strength =
                linq::from(mods, mod_count)
                    .first(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_MG)
                    .magnetised.strength;
            difficulty *= 1.0 - magnetised_strength;
        }

        if (linq::from(mods, mod_count)
                .any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_RX)) {
            difficulty *= 0.4;
        }

        if (linq::from(mods, mod_count)
                .any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_AP)) {
            difficulty *= 0.1;
        }

        double od = std::max(0.0, current.overall_difficulty);
        difficulty *= 0.825 + utils::pow(od, 2.2) / 1125.0;

        return difficulty;
    }

    std::vector<double> Reading::get_transformed_difficulties(const std::vector<double>& difficulties) const {
        std::vector<double> result = linq::from(difficulties).where(linq::_1 > 0).to_list();

        const double reduced_difficulty_base_line = 0.0; // Assume the first seconds are completely memorised

        int count = static_cast<int>(std::min(result.size(), static_cast<size_t>(reduced_note_count)));
        for (int i = 0; i < count; i++) {
            double t =
                math::clamp(static_cast<double>(i) / reduced_note_count, reduced_difficulty_base_line, 1.0);
            double scale = std::log10(1.0 + 9.0 * t);
            result[i] *= scale;
        }

        return result;
    }

    double Reading::count_top_weighted_object_difficulties(double difficulty_value) {
        if (object_difficulties.empty()) {
            return 0.0;
        }

        if (object_weight_sum == 0) {
            return 0.0;
        }

        double consistent_top_note =
            difficulty_value /
            object_weight_sum; // What would the top difficulty be if all object difficulties were identical

        if (consistent_top_note == 0) {
            return 0;
        }

        return linq::from(object_difficulties)
            .sum(linq::bind(&utils::logistic, linq::_1 / consistent_top_note, 1.15, 5.0, 1.1));
    }
}}}} // namespace pppp::osu::difficulty::skills
