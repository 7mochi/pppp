// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/skills/reading.h"
#include "pppp/taiko/difficulty/evaluators/reading_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    Reading::Reading(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : StrainDecaySkill(mods_in, mod_count_in, 1.0, 0.4),
          current_strain(0.0) {}

    double Reading::strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        const preprocessing::TaikoDifficultyHitObject& current =
            static_cast<const preprocessing::TaikoDifficultyHitObject&>(base_current);

        // Drum Rolls and Swells are exempt.
        if (!current.is_hit()) {
            return 0.0;
        }

        int index = current.colour.mono_streak ? current.colour.mono_streak->index_of(&current) : 0;

        current_strain *= pppp::utils::logistic(index, 4.0, -1.0 / 25.0, 0.5) + 0.5;
        current_strain *= strain_decay_base;
        current_strain += evaluators::ReadingEvaluator::evaluate_difficulty_of(current) * skill_multiplier;

        return current_strain;
    }
}}}} // namespace pppp::taiko::difficulty::skills
