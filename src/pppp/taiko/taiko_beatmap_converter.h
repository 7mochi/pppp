// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_TAIKO_BEATMAP_CONVERTER_H
#define PPPP_TAIKO_TAIKO_BEATMAP_CONVERTER_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/taiko/taiko_beatmap.h"

namespace pppp { namespace taiko {
    /// A speed multiplier applied globally to osu!taiko.
    /// @remarks osu! is generally slower than taiko, so a factor was historically added to increase
    /// speed for converts. This must be used everywhere slider length or beat length is used in
    /// taiko.
    ///
    /// Of note, this has never been exposed to the end user, and is considered a hidden internal
    /// multiplier.
    const double VELOCITY_MULTIPLIER = 1.399999976158142;
    /// Base osu! slider scoring distance.
    const double OSU_BASE_SCORING_DISTANCE = 100.0;

    void convert(TaikoBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                 const pppp::beatmaps::ObjectConverted<TaikoHitObject>& object_converted);
}} // namespace pppp::taiko

#endif
