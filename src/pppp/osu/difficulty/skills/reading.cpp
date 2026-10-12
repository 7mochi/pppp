// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/skills/reading.h"
#include "pppp/common/aggregation/harmonic_series.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/evaluators/reading_evaluator.h"
#include "pppp/osu/mods/osu_mod_hidden.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    Reading::Reading(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : Skill(mods_in, mod_count_in),
          has_hidden_mod(false),
          current_strain(0.0),
          harmonic_weight_sum(0.0),
          first_object_start_time() {
        has_hidden_mod = mods::has_hidden_body_fade(mods, mod_count);
    }

    double Reading::strain_decay(double ms) const { return utils::pow(0.8, ms / 1000.0); }

    double Reading::process_internal(const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        const double skill_multiplier = 2.5;
        const double reduced_difficulty_duration = 40 * 1000;

        double decay = strain_decay(current.delta_time);

        // This currently operates under the assumption that `ObjectDifficultyOf` is called once per object,
        // and in order. Under that assumption, we can trust that `current.StartTime` refers to the start time
        // of the first object in the case that `firstObjectStartTime` is yet to be set.
        if (!first_object_start_time.has_value()) {
            first_object_start_time = current.start_time;
        }

        const double reduced_difficulty_base_line = 0.2; // Assume that even with full memorisation, skill is
                                                         // still required to read and play the first objects.

        double current_object_strain =
            calculate_adjusted_difficulty(current) * (1.0 - decay) * skill_multiplier;

        if (current.start_time <= first_object_start_time.value() + reduced_difficulty_duration) {
            double scale = std::log10(utils::math::lerp_dotnet(
                1.0, 10.0,
                utils::math::clamp((current.start_time - first_object_start_time.value()) /
                                       reduced_difficulty_duration,
                                   0.0, 1.0)));
            current_object_strain *= utils::math::lerp_dotnet(reduced_difficulty_base_line, 1.0, scale);
        }

        current_strain *= decay;
        current_strain += current_object_strain;

        return current_strain;
    }

    double Reading::calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current) {
        double difficulty = evaluators::ReadingEvaluator::evaluate_difficulty_of(current, has_hidden_mod);

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_TD)) {
            difficulty = utils::pow(difficulty, 0.89);
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_MG)) {
            for (size_t i = 0; i < mod_count; i++) {
                if (mods[i].id == pppp::mods::MOD_MG) {
                    difficulty *= 1.0 - mods[i].magnetised.strength;
                    break;
                }
            }
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_RX)) {
            difficulty *= 0.4;
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_AP)) {
            difficulty *= 0.1;
        }

        double od = std::max(0.0, current.overall_difficulty);
        difficulty *= 0.825 + utils::pow(od, 2.2) / 1125.0;

        return difficulty;
    }

    double Reading::difficulty_value() {
        if (object_difficulties.empty()) {
            return 0;
        }

        pppp::common::aggregation::HarmonicSeries::Result result =
            pppp::common::aggregation::HarmonicSeries::aggregate(object_difficulties);

        harmonic_weight_sum = result.weight_sum;

        return result.difficulty;
    }

    double Reading::count_top_weighted_object_difficulties(double difficulty_value) {
        if (object_difficulties.empty()) {
            return 0.0;
        }

        if (harmonic_weight_sum == 0) {
            return 0.0;
        }

        double consistent_top_note =
            difficulty_value /
            harmonic_weight_sum; // What would the top difficulty be if all object difficulties were identical

        if (consistent_top_note == 0) {
            return 0;
        }

        double sum = 0.0;
        for (size_t i = 0; i < object_difficulties.size(); i++) {
            sum += utils::logistic(object_difficulties[i] / consistent_top_note, 1.15, 5.0, 1.1);
        }
        return sum;
    }
}}}} // namespace pppp::osu::difficulty::skills
