// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/skills/strain_decay_skill.h"
#include "pppp/utils/difficulty_calculation_utils.h"

namespace pppp { namespace common { namespace skills {
    StrainDecaySkill::StrainDecaySkill(const mods::Mod* mods_in, size_t mod_count_in)
        : StrainSkill(mods_in, mod_count_in),
          current_strain(0.0) {}

    double StrainDecaySkill::calculate_initial_strain(double time,
                                                      const preprocessing::DifficultyHitObject& current) {
        return current_strain * strain_decay(time - current.previous(0)->start_time);
    }

    double StrainDecaySkill::strain_value_at(const preprocessing::DifficultyHitObject& current) {
        current_strain *= strain_decay(current.delta_time);
        current_strain += strain_value_of(current) * skill_multiplier();

        return current_strain;
    }

    double StrainDecaySkill::strain_decay(double ms) { return utils::pow(strain_decay_base(), ms / 1000); }
}}} // namespace pppp::common::skills
