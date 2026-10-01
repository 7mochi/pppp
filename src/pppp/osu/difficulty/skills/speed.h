// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_SKILLS_SPEED_H
#define PPPP_OSU_DIFFICULTY_SKILLS_SPEED_H

#include "pppp/common/skills/harmonic_skill.h"
#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"
#include <vector>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    typedef pppp::common::skills::HarmonicSkill HarmonicSkill;
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    /// Represents the skill required to press keys with regards to keeping up with the speed at which objects
    /// need to be hit.
    class Speed : public HarmonicSkill {
    public:
        Speed(const pppp::mods::Mod* mods, size_t mod_count);

        double object_difficulty_of(const DifficultyHitObject& base_current);

        double relevant_object_count();
        double count_top_weighted_sliders(double difficulty_value);

    private:
        double current_strain;
        std::vector<double> slider_strains;

        double strain_decay(double ms) const;
        double calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current);
    };
}}}} // namespace pppp::osu::difficulty::skills

#endif
