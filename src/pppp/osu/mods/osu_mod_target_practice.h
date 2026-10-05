// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_MODS_OSU_MOD_TARGET_PRACTICE_H
#define PPPP_OSU_MODS_OSU_MOD_TARGET_PRACTICE_H

#include "pppp/mods/mod.h"
#include "pppp/osu/osu_beatmap.h"
#include "pppp/status.h"

namespace pppp { namespace osu { namespace mods {
    /// Jump distance for circles in the last combo
    const float MAX_BASE_DISTANCE = 333.0f;

    /// The maximum allowed jump distance after multipliers are applied
    const float DISTANCE_CAP = 380.0f;

    /// The extent of rotation towards playfield centre when a circle is near the edge
    const float EDGE_ROTATION_MULTIPLIER = 0.75f;

    /// Number of recent circles to check for overlap
    const int OVERLAP_CHECK_COUNT = 5;

    /// Acceptable difference for timing comparisons
    const double TIMING_PRECISION = 1;

    Status apply_target_practice(pppp::osu::OsuBeatmap& pb, const pppp::mods::Mod& mod);
}}} // namespace pppp::osu::mods

#endif
