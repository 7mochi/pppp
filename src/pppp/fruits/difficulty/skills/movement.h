// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_SKILLS_MOVEMENT_H
#define PPPP_FRUITS_DIFFICULTY_SKILLS_MOVEMENT_H

#include "pppp/common/skills/strain_decay_skill.h"

namespace pppp { namespace fruits { namespace difficulty { namespace skills {
    typedef pppp::common::skills::StrainDecaySkill StrainDecaySkill;

    class Movement : public StrainDecaySkill {
    protected:
        double skill_multiplier() const { return 1; }
        double strain_decay_base() const { return 0.2; }

        double decay_weight() const { return 0.94; }

        int section_length() const { return 750; }

    public:
        Movement(const pppp::mods::Mod* mods_in, size_t mod_count_in);

        double strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current);
    };
}}}} // namespace pppp::fruits::difficulty::skills

#endif
