// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_CATCH_BEATMAP_CONVERTER_H
#define PPPP_FRUITS_CATCH_BEATMAP_CONVERTER_H

#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/fruits/catch_beatmap.h"

namespace pppp { namespace fruits {
    Result::Value convert(CatchBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                          const pppp::beatmaps::ObjectConverted<object::CatchHitObject>& object_converted =
                              pppp::beatmaps::ObjectConverted<object::CatchHitObject>());
}} // namespace pppp::fruits

#endif
