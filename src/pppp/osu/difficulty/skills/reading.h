// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_SKILLS_READING_H
#define PPPP_OSU_DIFFICULTY_SKILLS_READING_H

#include "pppp/common/skills/harmonic_skill.h"
#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"
#include <vector>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    typedef pppp::common::skills::HarmonicSkill HarmonicSkill;
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    class Reading : public HarmonicSkill {
    public:
        Reading(const pppp::mods::Mod* mods, size_t mod_count);

        double object_difficulty_of(const DifficultyHitObject& base_current);

        std::vector<double> get_transformed_difficulties(const std::vector<double>& difficulties) const;

        double count_top_weighted_object_difficulties(double difficulty_value);

    private:
        bool has_hidden_mod;
        double current_strain;
        double reduced_note_count;
        nonstd::optional<double> reduced_duration;

        double strain_decay(double ms) const;
        double calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current);
    };
}}}} // namespace pppp::osu::difficulty::skills

#endif
