// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_MODS_OSU_MOD_HIDDEN_H
#define PPPP_OSU_MODS_OSU_MOD_HIDDEN_H

#include "pppp/mods/mod.h"
#include <cstddef>

namespace pppp { namespace osu { namespace mods {
    const double FADE_IN_DURATION_MULTIPLIER = 0.4;
    const double FADE_OUT_DURATION_MULTIPLIER = 0.3;

    bool has_hidden_body_fade(const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::osu::mods

#endif
