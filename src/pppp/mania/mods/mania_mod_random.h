// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_MODS_MANIA_MOD_RANDOM_H
#define PPPP_MANIA_MODS_MANIA_MOD_RANDOM_H

#include "pppp/mania/mania_beatmap.h"

namespace pppp { namespace mania { namespace mods {
    void apply_random(ManiaBeatmap& pb, int seed);
}}} // namespace pppp::mania::mods

#endif
