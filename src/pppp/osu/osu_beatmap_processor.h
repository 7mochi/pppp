// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OSU_BEATMAP_PROCESSOR_H
#define PPPP_OSU_OSU_BEATMAP_PROCESSOR_H

#include "pppp/osu/osu_beatmap.h"

namespace pppp { namespace osu {
    /// The maximum distance between the end of one object and the start of another which allows
    /// the objects to be stacked on top of another.
    const float STACK_DISTANCE = 3.0f;

    void update_combo_information(OsuBeatmap& pb);
    void apply_stacking(OsuBeatmap& pb);
}} // namespace pppp::osu

#endif
