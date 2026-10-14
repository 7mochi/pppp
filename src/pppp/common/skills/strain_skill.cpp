// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/skills/strain_skill.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/linq/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace common { namespace skills {
    namespace linq = ::pppp::utils::linq;

    StrainSkill::StrainSkill(const mods::Mod* mods_in, size_t mod_count_in)
        : Skill(mods_in, mod_count_in),
          current_section_peak(0.0),
          current_section_end(0.0) {}

    double StrainSkill::process_internal(const preprocessing::DifficultyHitObject& current) {
        // The first object doesn't generate a strain, so we begin with an incremented section end
        if (current.index == 0) {
            current_section_end = std::ceil(current.start_time / section_length()) * section_length();
        }

        while (current.start_time > current_section_end) {
            save_current_peak();
            start_new_section_from(current_section_end, current);
            current_section_end += section_length();
        }

        double strain = strain_value_at(current);
        current_section_peak = std::max(strain, current_section_peak);

        return strain;
    }

    double StrainSkill::difficulty_value() {
        double difficulty = 0.0;
        double weight = 1.0;

        // Sections with 0 strain are excluded to avoid worst-case time complexity of the following
        // sort (e.g. /b/2351871). These sections will not contribute to the difficulty.
        std::vector<double> current_strain_peaks;
        get_current_strain_peaks(current_strain_peaks);
        const linq::Enumerable<double> peaks = linq::from(current_strain_peaks).where(linq::_1 > 0);

        // Difficulty is the weighted sum of the highest strains from every section.
        // We're sorting from highest to lowest strain.
        const std::vector<double> strains = peaks.order_descending().to_list();
        for (size_t i = 0; i < strains.size(); i++) {
            difficulty += strains[i] * weight;
            weight *= decay_weight();
        }

        return difficulty;
    }

    void StrainSkill::save_current_peak() { strain_peaks.push_back(current_section_peak); }

    void StrainSkill::start_new_section_from(double time, const preprocessing::DifficultyHitObject& current) {
        // The maximum strain of the new section is not zero by default
        // This means we need to capture the strain level at the beginning of the new section, and use that as
        // the initial peak level.
        current_section_peak = calculate_initial_strain(time, current);
    }

    void StrainSkill::get_current_strain_peaks(std::vector<double>& out) const {
        std::vector<double> peaks = linq::from(strain_peaks).append(current_section_peak).to_list();
        out.swap(peaks);
    }

    double StrainSkill::count_top_weighted_strains(double difficulty_value) {
        if (object_difficulties.empty()) {
            return 0.0;
        }

        double consistent_top_strain =
            difficulty_value *
            (1.0 - decay_weight()); // What would the top strain be if all strain values were identical

        if (consistent_top_strain == 0) {
            return static_cast<double>(object_difficulties.size());
        }

        // Use a weighted sum of all strains. Constants are arbitrary and give nice values
        return linq::from(object_difficulties)
            .sum(linq::bind(&utils::logistic, linq::_1 / consistent_top_strain, 0.88, 10.0, 1.1));
    }
}}} // namespace pppp::common::skills
