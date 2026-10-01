// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_SKILLS_MOVEMENT_H
#define PPPP_FRUITS_DIFFICULTY_SKILLS_MOVEMENT_H

#include "pppp/common/skills/strain_decay_skill.h"

namespace pppp { namespace fruits { namespace difficulty { namespace skills {
    typedef pppp::common::skills::StrainDecaySkill StrainDecaySkill;

    class Movement : public StrainDecaySkill {
    public:
        Movement(const pppp::mods::Mod* mods, size_t mod_count);

        double strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current);
    };
}}}} // namespace pppp::fruits::difficulty::skills

#endif
