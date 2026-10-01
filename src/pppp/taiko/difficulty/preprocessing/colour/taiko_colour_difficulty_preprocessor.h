// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_TAIKO_COLOUR_DIFFICULTY_PREPROCESSOR_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_TAIKO_COLOUR_DIFFICULTY_PREPROCESSOR_H

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace colour {
    void process_and_assign(std::vector<TaikoDifficultyHitObject>& objects,
                            std::vector<data::MonoStreak>& mono_streaks,
                            std::vector<data::AlternatingMonoPattern>& alternating_mono_patterns,
                            std::vector<data::RepeatingHitPatterns>& repeating_hit_patterns);
}}}}} // namespace pppp::taiko::difficulty::preprocessing::colour

#endif
