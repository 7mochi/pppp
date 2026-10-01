// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_CATCH_DIFFICULTY_CALCULATOR_H
#define PPPP_FRUITS_DIFFICULTY_CATCH_DIFFICULTY_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/config.h"
#include "pppp/fruits/difficulty/catch_difficulty_attributes.h"
#include "pppp/mods/mod.h"
#include <cstddef>

namespace pppp { namespace fruits { namespace difficulty {
    const double DIFFICULTY_MULTIPLIER = 4.59;

    Result::Value calculate_difficulty(CatchDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                       const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::fruits::difficulty

#endif
