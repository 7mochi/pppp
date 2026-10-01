// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_SKILLS_FLASHLIGHT_H
#define PPPP_OSU_DIFFICULTY_SKILLS_FLASHLIGHT_H

#include "pppp/common/skills/strain_skill.h"
#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    typedef pppp::common::skills::StrainSkill StrainSkill;
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    /// Represents the skill required to memorise and hit every object in a map with the Flashlight mod
    /// enabled.
    class Flashlight : public StrainSkill {
    public:
        int total_objects;

        Flashlight(const pppp::mods::Mod* mods, size_t mod_count, int total_objects);

        double strain_value_at(const DifficultyHitObject& base_current);
        double calculate_initial_strain(double time, const DifficultyHitObject& base_current);
        double difficulty_value();

    private:
        double current_strain;

        double strain_decay(double ms) const;
        double calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current);
    };
}}}} // namespace pppp::osu::difficulty::skills

#endif
