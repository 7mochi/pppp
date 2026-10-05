// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OSU_BEATMAP_CONVERTER_H
#define PPPP_OSU_OSU_BEATMAP_CONVERTER_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/osu_beatmap.h"
#include "pppp/status.h"
#include <cstddef>

namespace pppp { namespace osu {
    Status convert(OsuBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                   const pppp::beatmaps::ObjectConverted<object::OsuHitObject>& object_converted =
                       pppp::beatmaps::ObjectConverted<object::OsuHitObject>());
    void apply_defaults(OsuBeatmap& pb);
    Status reflect(OsuBeatmap& pb, int axes);
    Status apply_beatmap_mods(OsuBeatmap& pb, const pppp::mods::Mod* mods, size_t mod_count);

    bool can_recompute_sliders(const OsuBeatmap& pb);
    Status slider_recompute(OsuBeatmap& pb, size_t slider_index);
}} // namespace pppp::osu

#endif
