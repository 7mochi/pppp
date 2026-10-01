// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/skills/skill.h"

namespace pppp { namespace common { namespace skills {
    void Skill::process(const preprocessing::DifficultyHitObject& current) {
        double difficulty_value = process_internal(current);
        object_difficulties.push_back(difficulty_value);
    }
}}} // namespace pppp::common::skills
