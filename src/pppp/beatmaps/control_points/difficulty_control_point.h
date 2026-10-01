// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_DIFFICULTY_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_DIFFICULTY_CONTROL_POINT_H

#include "pppp/beatmaps/control_points/control_point.h"

namespace pppp { namespace beatmaps { namespace control_points {
    /// @remarks Note that going forward, this control point type should always be assigned directly
    /// to HitObjects.
    class DifficultyControlPoint : public ControlPoint {
    public:
        /// The slider velocity, unclamped, as the timing point states it: 100 / -beat_length, or 1
        /// when the beat length is not negative.
        double slider_velocity;

        DifficultyControlPoint(double time_in, double slider_velocity_in)
            : ControlPoint(time_in),
              slider_velocity(slider_velocity_in) {}

        double clamped_slider_velocity() const {
            if (slider_velocity < 0.1) {
                return 0.1;
            }
            if (slider_velocity > 10.0) {
                return 10.0;
            }
            return slider_velocity;
        }
    };
}}} // namespace pppp::beatmaps::control_points

#endif
