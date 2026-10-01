// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/skills/stamina.h"
#include "pppp/taiko/difficulty/evaluators/stamina_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    Stamina::Stamina(const pppp::mods::Mod* mods_in, size_t mod_count_in, bool single_colour, bool convert)
        : StrainSkill(mods_in, mod_count_in),
          single_colour_stamina(single_colour),
          is_convert(convert),
          current_strain(0.0) {}

    double Stamina::strain_decay(double ms) const { return pppp::utils::pow(STRAIN_DECAY_BASE, ms / 1000.0); }

    double Stamina::strain_value_at(const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        const preprocessing::TaikoDifficultyHitObject& current =
            static_cast<const preprocessing::TaikoDifficultyHitObject&>(base_current);

        current_strain *= strain_decay(current.delta_time);
        double stamina_difficulty =
            evaluators::StaminaEvaluator::evaluate_difficulty_of(current) * SKILL_MULTIPLIER;

        // Safely prevents previous strains from shifting as new notes are added.
        int index = current.colour.mono_streak ? current.colour.mono_streak->index_of(&current) : 0;
        double mono_length_bonus = is_convert ? 1.0 : 1.0 + 0.5 * pppp::utils::reverse_lerp(index, 5.0, 20.0);

        // Mono-streak bonus is only applied to colour-based stamina to reward longer sequences of same-colour
        // hits within patterns.
        if (!single_colour_stamina) {
            stamina_difficulty *= mono_length_bonus;
        }

        current_strain += stamina_difficulty;

        // For converted maps, difficulty often comes entirely from long mono streams with no colour
        // variation.
        // To avoid over-rewarding these maps based purely on stamina strain, we dampen the strain value once
        // the index exceeds 10.
        return single_colour_stamina ? pppp::utils::logistic(-(index - 10) / 2.0, current_strain)
                                     : current_strain;
    }

    double
    Stamina::calculate_initial_strain(double time,
                                      const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        const preprocessing::TaikoDifficultyHitObject& current =
            static_cast<const preprocessing::TaikoDifficultyHitObject&>(base_current);
        if (single_colour_stamina) {
            return 0.0;
        }
        const preprocessing::TaikoDifficultyHitObject* previous =
            static_cast<const preprocessing::TaikoDifficultyHitObject*>(current.previous(0));
        double previous_start = previous ? previous->start_time : 0.0;
        return current_strain * strain_decay(time - previous_start);
    }
}}}} // namespace pppp::taiko::difficulty::skills
