// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/skills/rhythm.h"
#include "pppp/taiko/difficulty/evaluators/rhythm_evaluator.h"
#include "pppp/taiko/difficulty/evaluators/stamina_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    Rhythm::Rhythm(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : StrainDecaySkill(mods_in, mod_count_in, 1.0, 0.4) {}

    double Rhythm::strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        const preprocessing::TaikoDifficultyHitObject& current =
            static_cast<const preprocessing::TaikoDifficultyHitObject&>(base_current);
        double difficulty = evaluators::RhythmEvaluator::evaluate_difficulty_of(current);

        // To prevent abuse of exceedingly long intervals between awkward rhythms, we penalise its
        // difficulty.
        // Remove base strain
        double stamina_difficulty = evaluators::StaminaEvaluator::evaluate_difficulty_of(current) - 0.5;
        difficulty *= pppp::utils::logistic(stamina_difficulty, 1.0 / 15.0, 50.0);

        return difficulty;
    }
}}}} // namespace pppp::taiko::difficulty::skills
