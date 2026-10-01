// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/object/catch_hit_object.h"
#include "pppp/utils/difficulty_range.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"

namespace pppp { namespace fruits { namespace object {
    namespace {
        /// Calculates scale from a CS value, with an optional fudge that was historically applied to the osu!
        /// ruleset.
        double scale_from_circle_size(double circle_size) {
            double range = utils::difficulty_range(utils::f32(circle_size));

            return utils::f32(1.0f - 0.7f * range) / 2.0;
        }
    } // namespace

    float CatchHitObject::calculate_catch_width(double circle_size) {
        float scale = static_cast<float>(scale_from_circle_size(circle_size)) * 2.0f;
        return CATCHER_BASE_SIZE * (scale < 0 ? -scale : scale) * CATCHER_ALLOWED_CATCH_RANGE;
    }

    double CatchHitObject::effective_x() const {
        return utils::math::clamp(static_cast<float>(original_x) + static_cast<float>(x_offset), 0.0f,
                                  static_cast<float>(PLAYFIELD_WIDTH));
    }
}}} // namespace pppp::fruits::object
