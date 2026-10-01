// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/skills/flashlight.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/evaluators/flashlight_evaluator.h"
#include "pppp/osu/mods/osu_mod_hidden.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    Flashlight::Flashlight(const pppp::mods::Mod* mods_in, size_t mod_count_in, int total_objects_in)
        : StrainSkill(mods_in, mod_count_in),
          total_objects(total_objects_in),
          current_strain(0.0) {}

    double Flashlight::strain_decay(double ms) const { return utils::pow(0.15, ms / 1000.0); }

    double Flashlight::calculate_initial_strain(double time, const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        double prev_start = current.last_object_start_time / current.clock_rate;
        return current_strain * strain_decay(time - prev_start);
    }

    double Flashlight::strain_value_at(const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        const double skill_multiplier = 0.058;

        if (!pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_FL)) {
            return 0;
        }

        current_strain *= strain_decay(current.delta_time);
        current_strain += calculate_adjusted_difficulty(current) * skill_multiplier;

        return current_strain;
    }

    double Flashlight::calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current) {
        bool any_hidden = pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_HD);
        bool has_hidden = mods::has_hidden_body_fade(mods, mod_count);
        double difficulty = evaluators::FlashlightEvaluator::evaluate_difficulty_of(
            current, has_hidden, current.circle_size, any_hidden);

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_TD)) {
            difficulty = utils::pow(difficulty, 0.9);
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_MG)) {
            for (size_t i = 0; i < mod_count; i++) {
                if (mods[i].id == pppp::mods::MOD_MG) {
                    difficulty *= 1.0 - mods[i].magnetised.strength;
                    break;
                }
            }
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_DF)) {
            for (size_t i = 0; i < mod_count; i++) {
                if (mods[i].id == pppp::mods::MOD_DF) {
                    double deflate_scale = mods[i].scale.start_scale;
                    double t = utils::reverse_lerp(deflate_scale, 11.0, 1.0);
                    difficulty *= utils::math::clamp(t, 0.1, 1.0);
                    break;
                }
            }
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_RX)) {
            difficulty *= 0.7;
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_AP)) {
            difficulty *= 0.4;
        }

        double od = std::max(0.0, current.overall_difficulty);
        difficulty *= 0.985 + utils::pow(od, 2) / 4000.0;

        return difficulty;
    }

    double Flashlight::difficulty_value() {
        std::vector<double> peaks;
        get_current_strain_peaks(peaks);

        double sum = 0.0;
        for (size_t i = 0; i < peaks.size(); i++) {
            sum += peaks[i];
        }

        // Account for shorter maps having a higher ratio of 0 combo/100 combo flashlight radius.
        sum *= 0.7 + 0.1 * std::min(1.0, static_cast<double>(total_objects) / 200.0) +
               (total_objects > 200 ? 0.2 * std::min(1.0, static_cast<double>(total_objects - 200) / 200.0)
                                    : 0.0);

        return sum;
    }
}}}} // namespace pppp::osu::difficulty::skills
