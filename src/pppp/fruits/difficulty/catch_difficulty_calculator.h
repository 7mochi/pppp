// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_CATCH_DIFFICULTY_CALCULATOR_H
#define PPPP_FRUITS_DIFFICULTY_CATCH_DIFFICULTY_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/fruits/difficulty/catch_difficulty_attributes.h"
#include "pppp/fruits/difficulty/catch_strains.h"
#include "pppp/mods/mod.h"
#include "pppp/status.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace fruits { namespace difficulty {
    const double DIFFICULTY_MULTIPLIER = 4.59;

    Status calculate_difficulty(CatchDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count);

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<CatchDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count);

    Status calculate_strains(CatchStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::fruits::difficulty

#endif
