// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/skills/colour.h"
#include "pppp/taiko/difficulty/evaluators/colour_evaluator.h"

namespace pppp { namespace taiko { namespace difficulty { namespace skills {
    // This is set to decay slower than other skills, due to the fact that only the first note of each
    // encoding class having any difficulty values, and we want to allow colour difficulty to be able to
    // build up even on slower maps.
    Colour::Colour(const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : StrainDecaySkill(mods_in, mod_count_in, 0.12, 0.8) {}

    double Colour::strain_value_of(const pppp::common::preprocessing::DifficultyHitObject& base_current) {
        return evaluators::ColourEvaluator::evaluate_difficulty_of(
            static_cast<const preprocessing::TaikoDifficultyHitObject&>(base_current));
    }
}}}} // namespace pppp::taiko::difficulty::skills
