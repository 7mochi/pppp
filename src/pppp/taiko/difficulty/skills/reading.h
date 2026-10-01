// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_SKILLS_READING_H
#define PPPP_TAIKO_DIFFICULTY_SKILLS_READING_H

#include "pppp/common/skills/strain_decay_skill.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    typedef pppp::common::skills::StrainDecaySkill StrainDecaySkill;

    /// Calculates the reading coefficient of taiko difficulty.
    class Reading : public StrainDecaySkill {
    public:
        Reading(const pppp::mods::Mod* mods, size_t mod_count);

        double strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current);

    private:
        double current_strain;
    };
}}}} // namespace pppp::taiko::difficulty::skills

#endif
