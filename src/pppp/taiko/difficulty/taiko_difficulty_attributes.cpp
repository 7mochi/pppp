// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/taiko_difficulty_attributes.h"

namespace pppp { namespace taiko { namespace difficulty {
    TaikoDifficultyAttributes::TaikoDifficultyAttributes()
        : mechanical_difficulty(0.0),
          rhythm_difficulty(0.0),
          reading_difficulty(0.0),
          colour_difficulty(0.0),
          stamina_difficulty(0.0),
          mono_stamina_factor(0.0),
          consistency_factor(0.0),
          stamina_top_strains(0.0) {}
}}} // namespace pppp::taiko::difficulty
