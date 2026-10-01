// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_DIFFICULTY_SKILLS_STRAIN_H
#define PPPP_MANIA_DIFFICULTY_SKILLS_STRAIN_H

#include "pppp/common/skills/strain_decay_skill.h"
#include <vector>

namespace pppp { namespace mania { namespace difficulty { namespace skills {
    const double INDIVIDUAL_DECAY_BASE = 0.125;
    const double OVERALL_DECAY_BASE = 0.30;

    typedef pppp::common::skills::StrainDecaySkill StrainDecaySkill;

    class Strain : public StrainDecaySkill {
    public:
        Strain(const pppp::mods::Mod* mods_in, size_t mod_count_in, int total_columns);

        double strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current);
        double calculate_initial_strain(double time,
                                        const pppp::common::preprocessing::DifficultyHitObject& current);

    private:
        std::vector<double> individual_strains;
        double highest_individual_strain;
        double overall_strain;
    };
}}}} // namespace pppp::mania::difficulty::skills

#endif
