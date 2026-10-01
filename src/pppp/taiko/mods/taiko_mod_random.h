// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_MODS_TAIKO_MOD_RANDOM_H
#define PPPP_TAIKO_MODS_TAIKO_MOD_RANDOM_H

#include "pppp/taiko/taiko_beatmap.h"

namespace pppp { namespace taiko { namespace mods {
    void apply_random(TaikoBeatmap& pb, int seed);
}}} // namespace pppp::taiko::mods

#endif
