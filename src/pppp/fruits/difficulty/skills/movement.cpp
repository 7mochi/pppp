// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/difficulty/skills/movement.h"
#include "pppp/fruits/difficulty/evaluators/movement_evaluator.h"

namespace pppp { namespace fruits { namespace difficulty { namespace skills {
    Movement::Movement(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : StrainDecaySkill(mods_in, mod_count_in, 1.0, 0.2, 0.94, 750) {}

    double Movement::strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        return evaluators::MovementEvaluator::evaluate_difficulty_of(
            static_cast<const preprocessing::CatchDifficultyHitObject&>(base_current));
    }
}}}} // namespace pppp::fruits::difficulty::skills
