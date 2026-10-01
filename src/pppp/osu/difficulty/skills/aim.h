// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_SKILLS_AIM_H
#define PPPP_OSU_DIFFICULTY_SKILLS_AIM_H

#include "pppp/common/skills/variable_length_strain_skill.h"
#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"
#include <vector>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    typedef pppp::common::skills::VariableLengthStrainSkill VariableLengthStrainSkill;
    typedef pppp::common::skills::StrainPeak StrainPeak;
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    /// Represents the skill required to correctly aim at every object in the map with a uniform CircleSize
    /// and normalized distances.
    class Aim : public VariableLengthStrainSkill {
    public:
        bool include_sliders;

        Aim(const pppp::mods::Mod* mods, size_t mod_count, bool include_sliders);

        double strain_value_at(const DifficultyHitObject& base_current);
        double calculate_initial_strain(double time, const DifficultyHitObject& base_current);
        double difficulty_value();

        double get_difficult_sliders();
        double count_top_weighted_sliders(double difficulty_value);

    private:
        double current_strain;
        std::vector<double> slider_strains;

        double strain_decay(double ms) const;
        double calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current);
        double calculate_total_value(double snap_difficulty, double agility_difficulty,
                                     double flow_difficulty);
        /// Returns a sorted vector of strain peaks with the highest values reduced.
        std::vector<StrainPeak> get_reduced_strain_peaks();

        // A function that turns the ratio of snap : flow into the probability of snapping/flowing
        // It has the constraints:
        // P(snap) + P(flow) = 1 (the object is always either snapped or flowed)
        // P(snap) = f(snap/flow), P(flow) = f(flow/snap) (ie snap and flow are symmetric and reversible)
        // Therefore: f(x) + f(1/x) = 1
        // 0 <= f(x) <= 1 (cannot have negative or greater than 100% probability of snapping or flowing)
        // This logistic function is a solution, which fits nicely with the general idea of interpolation and
        // provides a tuneable constant
        static double calculate_snap_flow_probability(double ratio);
    };
}}}} // namespace pppp::osu::difficulty::skills

#endif
