// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/skills/variable_length_strain_skill.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/search/csharp.h"
#include <algorithm>

namespace pppp { namespace common { namespace skills {
    VariableLengthStrainSkill::VariableLengthStrainSkill(const pppp::mods::Mod* mods_in, size_t mod_count_in,
                                                         double decay_weight_in, int max_section_length_in)
        : Skill(mods_in, mod_count_in),
          decay_weight(decay_weight_in),
          max_section_length(max_section_length_in),
          max_stored_length(11.0 / (1.0 - decay_weight_in)),
          current_section_peak(0.0),
          current_section_begin(0.0),
          current_section_end(0.0),
          total_length(0.0),
          final_peak(0) {}

    double VariableLengthStrainSkill::process_internal(const preprocessing::DifficultyHitObject& current) {
        // If we're on the first object, set up the first section to end `MaxSectionLength` after it.
        if (current.index == 0) {
            current_section_begin = current.start_time;
            current_section_end = current_section_begin + max_section_length;

            // No work is required for first object after calculating difficulty
            current_section_peak = strain_value_at(current);
            return current_section_peak;
        }

        backfill_peaks(current);

        double current_strain = strain_value_at(current);

        // If the current strain is larger than the current peak, begin a new peak
        // Otherwise, add the current strain to the queue
        if (current_strain > current_section_peak) {
            // Clear the queue since none of the strains inside of it will be contributing to the
            // difficulty.
            queued_strains.clear();

            // End the current section with the new peak
            save_current_peak(current.start_time - current_section_begin);

            // Set up the new section to start at the current object with the current strain
            current_section_begin = current.start_time;
            current_section_end = current_section_begin + max_section_length;
            current_section_peak = current_strain;
        } else {
            // Empty the queue of smaller elements as they won't be relevant to difficulty
            while (!queued_strains.empty() && queued_strains.back().strain_value < current_strain) {
                queued_strains.pop_back();
            }

            QueuedStrain qs;
            qs.strain_value = current_strain;
            qs.start_time = current.start_time;
            queued_strains.push_back(qs);
        }

        return current_strain;
    }

    double VariableLengthStrainSkill::count_top_weighted_strains(double difficulty_value) {
        if (object_difficulties.empty()) {
            return 0.0;
        }

        double consistent_top_strain =
            difficulty_value *
            (1.0 - decay_weight); // What would the top strain be if all strain values were identical

        if (consistent_top_strain == 0) {
            return static_cast<double>(object_difficulties.size());
        }

        // Use a weighted sum of all strains. Constants are arbitrary and give nice values
        double sum = 0.0;
        for (size_t i = 0; i < object_difficulties.size(); i++) {
            sum += utils::logistic(object_difficulties[i] / consistent_top_strain, 0.88, 10.0, 1.1);
        }
        return sum;
    }

    const std::vector<StrainPeak>& VariableLengthStrainSkill::get_current_strain_peaks() {
        if (final_peak == 0) {
            StrainPeak fp;
            fp.value = current_section_peak;
            fp.section_length = utils::math::round_half_even(current_section_end - current_section_begin);
            size_t pos = add_in_place(fp);
            final_peak = &strain_peaks[pos];
        }

        return strain_peaks;
    }

    void VariableLengthStrainSkill::backfill_peaks(const preprocessing::DifficultyHitObject& current) {
        // If the current object starts after the current section ends, then we want to start a new
        // section without any harsh drop-off. If we have previous strains that influence the current
        // difficulty we will prioritise those first. Otherwise, start with the current object's
        // initial strain.
        while (current.start_time > current_section_end) {
            // Save the current peak, marking the end of the section.
            save_current_peak(current_section_end - current_section_begin);
            current_section_begin = current_section_end;

            // If we have any strains queued, then we will use those until the object falls into the
            // new section.
            if (!queued_strains.empty()) {
                QueuedStrain qs = queued_strains[0];
                queued_strains.erase(queued_strains.begin());

                // We want the section to end `MaxSectionLength` after the strain we're using as an
                // influence. This effectively means the queued strain will exist in its own section
                // if the gap between the queued strain and current object is large enough. This is
                // required to make sure there's no harsh difficulty difference between 2 sections
                // if there was a large gap.
                current_section_end = qs.start_time + max_section_length;
                start_new_section_from(current_section_begin, current);

                // If the current object's peak was higher, we don't want to override it with a
                // lower strain. Only use the queued strain if it contributes more difficulty.
                current_section_peak = std::max(current_section_peak, qs.strain_value);
            } else {
                // We don't have any prior strains to take as a reference, so end the new section
                // `MaxSectionLength` after it starts.
                current_section_end = current_section_begin + max_section_length;
                start_new_section_from(current_section_begin, current);
            }
        }
    }

    void VariableLengthStrainSkill::save_current_peak(double section_length) {
        if (final_peak != 0) {
            for (size_t i = 0; i < strain_peaks.size(); i++) {
                if (&strain_peaks[i] == final_peak) {
                    strain_peaks.erase(strain_peaks.begin() + static_cast<ptrdiff_t>(i));
                    break;
                }
            }
            final_peak = 0;
        }

        StrainPeak peak;
        peak.value = current_section_peak;
        peak.section_length = utils::math::round_half_even(section_length);

        add_in_place(peak);
        total_length += section_length;

        // Remove from the back of our strain peaks if there's any which are too deep to contribute
        // to difficulty. `maxStoredLength` dictates for us how many sections will preserve at least
        // 99.999% of the difficulty value.
        while (total_length > max_stored_length * max_section_length && !strain_peaks.empty()) {
            total_length -= strain_peaks.back().section_length;
            strain_peaks.pop_back();
        }
    }

    size_t VariableLengthStrainSkill::add_in_place(const StrainPeak& peak) {
        int index = utils::search::binary_search(strain_peaks, peak);
        if (index < 0) {
            index = ~index;
        }
        strain_peaks.insert(strain_peaks.begin() + index, peak);
        return static_cast<size_t>(index);
    }

    void
    VariableLengthStrainSkill::start_new_section_from(double time,
                                                      const preprocessing::DifficultyHitObject& current) {
        // The maximum strain of the new section is not zero by default. This means we need to
        // capture the strain level at the beginning of the new section, and use that as the initial
        // peak level.
        current_section_peak = calculate_initial_strain(time, current);
    }
}}} // namespace pppp::common::skills
