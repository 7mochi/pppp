// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_SKILLS_STAMINA_H
#define PPPP_TAIKO_DIFFICULTY_SKILLS_STAMINA_H

#include "pppp/common/skills/strain_skill.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    typedef pppp::common::skills::StrainSkill StrainSkill;

    const double SKILL_MULTIPLIER = 1.1;
    const double STRAIN_DECAY_BASE = 0.4;

    /// Calculates the stamina coefficient of taiko difficulty.
    class Stamina : public StrainSkill {
    public:
        bool single_colour_stamina;
        bool is_convert;

        /// Creates a Stamina skill.
        /// @param mods Mods for use in skill calculations.
        /// @param single_colour Reads when Stamina is from a single coloured pattern.
        /// @param convert Determines if the currently evaluated beatmap is converted.
        Stamina(const pppp::mods::Mod* mods, size_t mod_count, bool single_colour, bool convert);

        double strain_value_at(const pppp::common::preprocessing::DifficultyHitObject& base_current);
        double calculate_initial_strain(double time,
                                        const pppp::common::preprocessing::DifficultyHitObject& base_current);

    private:
        double current_strain;

        double strain_decay(double ms) const;
    };
}}}} // namespace pppp::taiko::difficulty::skills

#endif
