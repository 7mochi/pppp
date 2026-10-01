// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_SKILLS_STRAIN_DECAY_SKILL_H
#define PPPP_COMMON_SKILLS_STRAIN_DECAY_SKILL_H

#include "pppp/common/skills/strain_skill.h"
namespace pppp { namespace common { namespace skills {
    /// Used to processes strain values of DifficultyHitObjects, keep track of strain levels caused by the
    /// processed objects and to calculate a final difficulty value representing the difficulty of hitting all
    /// the processed objects.
    class StrainDecaySkill : public StrainSkill {
    public:
        /// Strain values are multiplied by this number for the given skill. Used to balance the value of
        /// different skills between each other.
        const double skill_multiplier;

        /// Determines how quickly strain decays for the given skill.
        /// For example a value of 0.15 indicates that strain decays to 15% of its original value in one
        /// second.
        const double strain_decay_base;

        /// The current strain level.
        double current_strain;

        StrainDecaySkill(const pppp::mods::Mod* mods_in, size_t mod_count_in,
                         double skill_multiplier_in = 1.0, double strain_decay_base_in = 0.15,
                         double decay_weight_in = 0.9, int section_length_in = 400);

        /// Calculates the strain value of a DifficultyHitObject. This value is affected by previously
        /// processed objects.
        virtual double strain_value_of(const preprocessing::DifficultyHitObject& current) = 0;

        double calculate_initial_strain(double time, const preprocessing::DifficultyHitObject& current);
        double strain_value_at(const preprocessing::DifficultyHitObject& current);
        double strain_decay(double ms);
    };
}}} // namespace pppp::common::skills

#endif
