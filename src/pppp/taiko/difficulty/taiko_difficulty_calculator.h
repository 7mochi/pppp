// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_TAIKO_DIFFICULTY_CALCULATOR_H
#define PPPP_TAIKO_DIFFICULTY_TAIKO_DIFFICULTY_CALCULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/mods/mod.h"
#include "pppp/status.h"
#include "pppp/taiko/difficulty/taiko_difficulty_attributes.h"
#include "pppp/taiko/difficulty/taiko_strains.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace taiko { namespace difficulty {
    const double DIFFICULTY_MULTIPLIER = 0.084375;
    const double RHYTHM_SKILL_MULTIPLIER = 0.770 * DIFFICULTY_MULTIPLIER;
    const double READING_SKILL_MULTIPLIER = 0.100 * DIFFICULTY_MULTIPLIER;
    const double COLOUR_SKILL_MULTIPLIER = 0.375 * DIFFICULTY_MULTIPLIER;
    const double STAMINA_SKILL_MULTIPLIER = 0.445 * DIFFICULTY_MULTIPLIER;

    Status calculate_difficulty(TaikoDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count);

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<TaikoDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count);

    Status calculate_strains(TaikoStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::taiko::difficulty

#endif
