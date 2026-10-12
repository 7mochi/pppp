// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_SKILLS_READING_H
#define PPPP_OSU_DIFFICULTY_SKILLS_READING_H

#include "pppp/common/skills/skill.h"
#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    typedef pppp::common::skills::Skill Skill;
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    class Reading : public Skill {
    public:
        Reading(const pppp::mods::Mod* mods_in, size_t mod_count_in);

        double process_internal(const DifficultyHitObject& base_current);

        double difficulty_value();

        double count_top_weighted_object_difficulties(double difficulty_value);

    private:
        bool has_hidden_mod;
        double current_strain;
        double harmonic_weight_sum;
        nonstd::optional<double> first_object_start_time;

        double strain_decay(double ms) const;
        double calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current);
    };
}}}} // namespace pppp::osu::difficulty::skills

#endif
