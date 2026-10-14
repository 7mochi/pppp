// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_SKILLS_HARMONIC_SKILL_H
#define PPPP_COMMON_SKILLS_HARMONIC_SKILL_H

#include "pppp/common/skills/skill.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <vector>
namespace pppp { namespace common { namespace skills {
    class HarmonicSkill : public Skill {
    protected:
        /// The sum of object weights, calculated during summation.
        /// Required for any calculations which need to normalise difficulty value.
        double object_weight_sum;

        /// Scaling factor applied as HarmonicScale / (1 + index) during weight calculations.
        /// A higher value will increase the influence of the hardest object difficulties during summation.
        virtual double harmonic_scale() const { return 1.0; }

        /// Exponent that controls the rate of which decay increases as the index increases.
        /// Values closer to 1 decay faster whilst lower values give more weight to lower object difficulties.
        virtual double decay_exponent() const { return 0.9; }

    public:
        HarmonicSkill(const pppp::mods::Mod* mods_in, size_t mod_count_in);

        /// Returns the difficulty value of the current DifficultyHitObject. This value is calculated with or
        /// without respect to previous objects.
        virtual double object_difficulty_of(const preprocessing::DifficultyHitObject& current) = 0;

        /// Transforms the object difficulties specifically for final difficulty summation.
        /// This can be used to decrease weight of certain objects based on a skill-specific criteria.
        virtual std::vector<double>
        get_transformed_difficulties(const std::vector<double>& difficulties) const {
            return difficulties;
        }

        double process_internal(const preprocessing::DifficultyHitObject& current);

        double difficulty_value();

        /// Calculates the number of object difficulties weighted against the top object difficulty.
        virtual double count_top_weighted_object_difficulties(double difficulty_value);

        static double difficulty_to_performance(double difficulty) {
            return 4.0 * pppp::utils::pow(difficulty, 3);
        }
    };
}}} // namespace pppp::common::skills

#endif
