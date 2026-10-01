// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_MODS_OSU_MOD_RANDOM_H
#define PPPP_OSU_MODS_OSU_MOD_RANDOM_H

#include "pppp/config.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/osu_beatmap.h"

namespace pppp { namespace osu { namespace mods {
    Result::Value apply_random(pppp::osu::OsuBeatmap& pb, const pppp::mods::Mod& mod);
}}} // namespace pppp::osu::mods

#endif
