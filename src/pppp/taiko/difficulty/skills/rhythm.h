// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_SKILLS_RHYTHM_H
#define PPPP_TAIKO_DIFFICULTY_SKILLS_RHYTHM_H

#include "pppp/common/skills/strain_decay_skill.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    typedef pppp::common::skills::StrainDecaySkill StrainDecaySkill;

    /// Calculates the rhythm coefficient of taiko difficulty.
    class Rhythm : public StrainDecaySkill {
    public:
        Rhythm(const pppp::mods::Mod* mods, size_t mod_count);

        double strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current);
    };
}}}} // namespace pppp::taiko::difficulty::skills

#endif
