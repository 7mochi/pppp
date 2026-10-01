// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_SKILLS_VARIABLE_LENGTH_STRAIN_SKILL_H
#define PPPP_COMMON_SKILLS_VARIABLE_LENGTH_STRAIN_SKILL_H

#include "pppp/common/skills/skill.h"
#include "pppp/utils/math/csharp.h"
#include <vector>
namespace pppp { namespace common { namespace skills {
    /// Used to store the difficulty of a section of a map.
    struct StrainPeak {
        double value;
        double section_length;

        /// Reverse sort, highest is first.
        int compare_to(const StrainPeak& other) const { return utils::math::compare_to(other.value, value); }
    };

    /// Reverse sort, highest is first.
    inline bool strain_peak_greater(const StrainPeak& a, const StrainPeak& b) { return a.value > b.value; }

    /// Stores previous strains so that, if a high difficulty hit object is followed by a lower
    /// difficulty hit object, the high difficulty hit object gets a full strain instead of being cut short.
    struct QueuedStrain {
        double strain_value;
        double start_time;
    };

    /// Similar to StrainSkill, but instead of strains having a fixed length, strains can be any length.
    /// A new StrainPeak is created for each DifficultyHitObject.
    /// @remarks This class intends to replace StrainSkill eventually as it fixes bugs with that
    /// implementation.
    /// Has not yet been applied globally as it changes resultant PP values in ways which may require
    /// discretion.
    class VariableLengthStrainSkill : public Skill {
    public:
        /// The weight by which each strain value decays.
        const double decay_weight;

        /// The maximum length of each strain section.
        const int max_section_length;

        /// The number of `MaxSectionLength` sections calculated such that enough of the difficulty value is
        /// preserved. This variable operates under the assumption that final difficulty calculation uses a
        /// standard geometric sum, and that strain_peaks is not required for any other purpose.
        const double max_stored_length;

        // We also keep track of the peak strain in the current section.
        double current_section_peak;
        double current_section_begin;
        double current_section_end;

        std::vector<StrainPeak> strain_peaks;
        double total_length;

        /// Stores previous strains so that, if a high difficulty hit object is followed by a lower
        /// difficulty hit object, the high difficulty hit object gets a full strain instead of being cut
        /// short.
        std::vector<QueuedStrain> queued_strains;
        StrainPeak* final_peak;

        /// Create a new VariableLengthStrainSkill.
        /// @param mods_in The mods.
        /// @param mod_count_in The number of mods.
        /// @param decay_weight_in The weight by which each strain value decays.
        /// @param max_section_length_in The maximum length of each strain section.
        VariableLengthStrainSkill(const pppp::mods::Mod* mods_in, size_t mod_count_in,
                                  double decay_weight_in = 0.9, int max_section_length_in = 400);

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

        /// Calculates the number of strains weighted against the top strain. The result is scaled by
        /// clock rate as it affects the total number of strains.
        double count_top_weighted_strains(double difficulty_value);

        /// Returns the peak strains for each MaxSectionLength section of the beatmap, including the
        /// peak of the current section.
        const std::vector<StrainPeak>& get_current_strain_peaks();

    private:
        /// Fills the space between the end of the current section and the current object, if there is any.
        /// @param current The object who's `start_time` is backfilled to.
        void backfill_peaks(const preprocessing::DifficultyHitObject& current);

        /// Saves the current peak strain level to the list of strain peaks, which will be used to
        /// calculate an overall difficulty.
        void save_current_peak(double section_length);

        /// Inserts a peak keeping the list ordered by value, highest first.
        size_t add_in_place(const StrainPeak& peak);

        /// Sets the initial strain level for a new section.
        /// @param time The beginning of the new section in milliseconds.
        /// @param current The current hit object.
        void start_new_section_from(double time, const preprocessing::DifficultyHitObject& current);
    };
}}} // namespace pppp::common::skills

#endif
