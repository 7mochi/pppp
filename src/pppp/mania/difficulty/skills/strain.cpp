// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/skills/strain.h"
#include "pppp/mania/difficulty/evaluators/individual_strain_evaluator.h"
#include "pppp/mania/difficulty/evaluators/overall_strain_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>

namespace pppp { namespace mania { namespace difficulty { namespace skills {
    Strain::Strain(const pppp::mods::Mod* mods_in, size_t mod_count_in, int total_columns)
        : StrainDecaySkill(mods_in, mod_count_in, 1.0, 1.0),
          individual_strains(static_cast<size_t>(total_columns > 0 ? total_columns : 0), 0.0),
          highest_individual_strain(0.0),
          overall_strain(1.0) {}

    namespace {
        double apply_decay(double value, double delta_time, double decay_base) {
            return value * utils::pow(decay_base, delta_time / 1000);
        }
    } // namespace

    double Strain::strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        const preprocessing::ManiaDifficultyHitObject& current =
            static_cast<const preprocessing::ManiaDifficultyHitObject&>(base_current);
        const size_t column = static_cast<size_t>(current.column);

        individual_strains[column] =
            apply_decay(individual_strains[column], current.column_strain_time, INDIVIDUAL_DECAY_BASE);
        individual_strains[column] += evaluators::IndividualStrainEvaluator::evaluate_difficulty_of(current);

        // Take the hardest individualStrain for notes that happen at the same time (in a chord).
        // This is to ensure the order in which the notes are processed does not affect the resultant total
        // strain.
        highest_individual_strain = current.delta_time <= 1
                                        ? std::max(highest_individual_strain, individual_strains[column])
                                        : individual_strains[column];

        overall_strain = apply_decay(overall_strain, current.delta_time, OVERALL_DECAY_BASE);
        overall_strain += evaluators::OverallStrainEvaluator::evaluate_difficulty_of(current);

        // By subtracting CurrentStrain, this skill effectively only considers the maximum strain of any one
        // hitobject within each strain section.
        return highest_individual_strain + overall_strain - current_strain;
    }

    double Strain::calculate_initial_strain(double time,
                                            const pppp::common::preprocessing::DifficultyHitObject& current) {
        const double previous_start = current.last_object_start_time / current.clock_rate;

        return apply_decay(highest_individual_strain, time - previous_start, INDIVIDUAL_DECAY_BASE) +
               apply_decay(overall_strain, time - previous_start, OVERALL_DECAY_BASE);
    }
}}}} // namespace pppp::mania::difficulty::skills
