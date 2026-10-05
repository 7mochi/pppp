// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_DIFFICULTY_MANIA_DIFFICULTY_CALCULATOR_H
#define PPPP_MANIA_DIFFICULTY_MANIA_DIFFICULTY_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/mania/difficulty/mania_difficulty_attributes.h"
#include "pppp/mania/difficulty/mania_strains.h"
#include "pppp/mods/mod.h"
#include "pppp/status.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace difficulty {
    const double DIFFICULTY_MULTIPLIER = 0.018;

    Status calculate_difficulty(ManiaDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count);

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<ManiaDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count);

    Status calculate_strains(ManiaStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::mania::difficulty

#endif
