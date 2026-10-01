// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/skills/strain_decay_skill.h"
#include "pppp/utils/difficulty_calculation_utils.h"

namespace pppp { namespace common { namespace skills {
    StrainDecaySkill::StrainDecaySkill(const pppp::mods::Mod* mods_in, size_t mod_count_in,
                                       double skill_multiplier_in, double strain_decay_base_in,
                                       double decay_weight_in, int section_length_in)
        : StrainSkill(mods_in, mod_count_in, decay_weight_in, section_length_in),
          skill_multiplier(skill_multiplier_in),
          strain_decay_base(strain_decay_base_in),
          current_strain(0.0) {}

    double StrainDecaySkill::calculate_initial_strain(double time,
                                                      const preprocessing::DifficultyHitObject& current) {
        double prev_start = current.last_object_start_time / current.clock_rate;
        return current_strain * strain_decay(time - prev_start);
    }

    double StrainDecaySkill::strain_value_at(const preprocessing::DifficultyHitObject& current) {
        current_strain *= strain_decay(current.delta_time);
        current_strain += strain_value_of(current) * skill_multiplier;

        return current_strain;
    }

    double StrainDecaySkill::strain_decay(double ms) { return utils::pow(strain_decay_base, ms / 1000.0); }
}}} // namespace pppp::common::skills
