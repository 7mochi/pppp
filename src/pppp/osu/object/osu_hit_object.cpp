// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/utils/difficulty_range.h"
#include <algorithm>

namespace pppp { namespace osu { namespace object {
    double OsuHitObject::time_preempt_for_ar(double ar) {
        return utils::difficulty_range_int(static_cast<float>(ar), PREEMPT_RANGE);
    }

    double OsuHitObject::time_fade_in_for_preempt(double preempt) {
        return 400.0 * std::min(1.0, preempt / PREEMPT_MIN);
    }

    double OsuHitObject::calculate_scale_from_cs(double cs) {
        // The following comment is copied verbatim from osu-stable:
        //
        //   Builds of osu! up to 2013-05-04 had the gamefield being rounded down, which caused incorrect
        //   radius calculations in widescreen cases. This ratio adjusts to allow for old replays to work
        //   post-fix, which in turn increases the lenience for all plays, but by an amount so small it should
        //   only be effective in replays.
        //
        // To match expectations of gameplay we need to apply this multiplier to circle scale. It's weird but
        // is what it is. It works out to under 1 game pixel and is generally not meaningful to gameplay, but
        // is to replay playback accuracy.
        const float broken_gamefield_rounding_allowance = 1.00041f;

        double range = utils::difficulty_range(static_cast<float>(cs));
        float scale = static_cast<float>(1.0f - 0.7f * range) / 2 * broken_gamefield_rounding_allowance;
        return scale;
    }

    double OsuHitObject::calculate_radius(double cs) { return OBJECT_RADIUS * calculate_scale_from_cs(cs); }
}}} // namespace pppp::osu::object
