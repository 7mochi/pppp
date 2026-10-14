// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_SKILLS_STRAIN_SKILL_H
#define PPPP_COMMON_SKILLS_STRAIN_SKILL_H

#include "pppp/common/skills/skill.h"
#include <vector>
namespace pppp { namespace common { namespace skills {
    /// Used to processes strain values of DifficultyHitObjects, keep track of strain levels caused by the
    /// processed objects and to calculate a final difficulty value representing the difficulty of hitting all
    /// the processed objects.
    class StrainSkill : public Skill {
    protected:
        /// The weight by which each strain value decays.
        virtual double decay_weight() const { return 0.9; }

        /// The length of each strain section.
        virtual int section_length() const { return 400; }

    private:
        double current_section_peak; // We also keep track of the peak strain level in the current section.
        double current_section_end;

        std::vector<double> strain_peaks;

    public:
        StrainSkill(const pppp::mods::Mod* mods_in, size_t mod_count_in);

        /// Returns the strain value at DifficultyHitObject. This value is calculated with or without respect
        /// to previous objects.
        virtual double strain_value_at(const preprocessing::DifficultyHitObject& current) = 0;

        /// Retrieves the peak strain at a point in time.
        /// @param time The time to retrieve the peak strain at.
        /// @param current The current hit object.
        /// @returns The peak strain.
        virtual double calculate_initial_strain(double time,
                                                const preprocessing::DifficultyHitObject& current) = 0;

        /// Process a DifficultyHitObject and update current strain values accordingly.
        double process_internal(const preprocessing::DifficultyHitObject& current);

        /// Returns the calculated difficulty value representing all DifficultyHitObjects that have been
        /// processed up to this point.
        double difficulty_value();

        /// Calculates the number of strains weighted against the top strain. The result is scaled by
        /// clock rate as it affects the total number of strains.
        double count_top_weighted_strains(double difficulty_value);

        /// Returns the peak strains for each `section_length` section of the beatmap, including the peak
        /// of the current section.
        void get_current_strain_peaks(std::vector<double>& out) const;

    private:
        /// Saves the current peak strain level to the list of strain peaks, which will be used to
        /// calculate an overall difficulty.
        void save_current_peak();

        /// Sets the initial strain level for a new section.
        /// @param time The beginning of the new section in milliseconds.
        /// @param current The current hit object.
        void start_new_section_from(double time, const preprocessing::DifficultyHitObject& current);
    };
}}} // namespace pppp::common::skills

#endif
