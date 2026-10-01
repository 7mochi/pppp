// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_EFFECT_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_EFFECT_CONTROL_POINT_H

#include "pppp/beatmaps/control_points/control_point.h"

namespace pppp { namespace beatmaps { namespace control_points {
    class EffectControlPoint : public ControlPoint {
    public:
        /// Whether this control point enables Kiai mode.
        bool kiai_mode;

        /// The relative scroll speed.
        double scroll_speed;

        EffectControlPoint(double time_in, bool kiai_mode_in)
            : ControlPoint(time_in),
              kiai_mode(kiai_mode_in),
              scroll_speed(1.0) {}

        double clamped_scroll_speed() const {
            return scroll_speed < 0.01 ? 0.01 : (scroll_speed > 10.0 ? 10.0 : scroll_speed);
        }
    };
}}} // namespace pppp::beatmaps::control_points

#endif
