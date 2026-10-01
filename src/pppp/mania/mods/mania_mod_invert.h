// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_MODS_MANIA_MOD_INVERT_H
#define PPPP_MANIA_MODS_MANIA_MOD_INVERT_H

#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/mania/mania_beatmap.h"

namespace pppp { namespace mania { namespace mods {
    void apply_invert(ManiaBeatmap& pb, const pppp::beatmaps::control_points::ControlPointInfo& info);
}}} // namespace pppp::mania::mods

#endif
