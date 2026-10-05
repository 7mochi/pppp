// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_OBJECT_JUICE_STREAM_H
#define PPPP_FRUITS_OBJECT_JUICE_STREAM_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/fruits/object/catch_hit_object.h"

namespace pppp { namespace fruits { namespace object {
    CatchHitObject create_juice_stream(const pppp::beatmaps::Slider& slider, double head_x, double time,
                                       double end_time, int slider_index);
}}} // namespace pppp::fruits::object

#endif
