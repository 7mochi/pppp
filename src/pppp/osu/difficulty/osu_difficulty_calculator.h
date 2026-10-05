// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_OSU_DIFFICULTY_CALCULATOR_H
#define PPPP_OSU_DIFFICULTY_OSU_DIFFICULTY_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/osu_strains.h"
#include "pppp/status.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace osu { namespace difficulty {
    Status calculate_difficulty(OsuDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count);

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<OsuDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count);

    Status calculate_strains(OsuStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::osu::difficulty

#endif
