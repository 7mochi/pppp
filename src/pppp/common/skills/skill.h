// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_SKILLS_SKILL_H
#define PPPP_COMMON_SKILLS_SKILL_H

#include "pppp/common/preprocessing/difficulty_hit_object.h"
#include "pppp/mods/mod.h"
#include <vector>
namespace pppp { namespace common { namespace skills {
    /// A bare minimal abstract skill for fully custom skill implementations.
    /// @remarks This class should be considered a "processing" class and not persisted.
    class Skill {
    protected:
        /// List of calculated per-object difficulties, populated by process.
        std::vector<double> object_difficulties;

        /// Mods for use in skill calculations.
        const pppp::mods::Mod* mods;
        size_t mod_count;

    public:
        Skill(const pppp::mods::Mod* mods_in, size_t mod_count_in)
            : mods(mods_in),
              mod_count(mod_count_in) {}

        virtual ~Skill() {}
        virtual double process_internal(const preprocessing::DifficultyHitObject& current) = 0;

        /// Returns the calculated difficulty value representing all DifficultyHitObjects that have been
        /// processed up to this point.
        virtual double difficulty_value() = 0;

        /// Process a DifficultyHitObject.
        /// @param current The DifficultyHitObject to process.
        void process(const preprocessing::DifficultyHitObject& current);

        const std::vector<double>& get_object_difficulties() const { return object_difficulties; }
    };
}}} // namespace pppp::common::skills

#endif
