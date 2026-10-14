// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_EFFECT_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_EFFECT_CONTROL_POINT_H

#include "pppp/beatmaps/control_points/control_point.h"

namespace pppp { namespace beatmaps { namespace control_points {
    class EffectControlPoint : public ControlPoint {
    private:
        double scroll_speed_value;

    public:
        /// Whether this control point enables Kiai mode.
        bool kiai_mode;

        EffectControlPoint(double time_in, bool kiai_mode_in)
            : ControlPoint(time_in),
              scroll_speed_value(1),
              kiai_mode(kiai_mode_in) {}

        /// The relative scroll speed.
        double scroll_speed() const { return scroll_speed_value; }

        void set_scroll_speed(double value) {
            scroll_speed_value = value < 0.01 ? 0.01 : (value > 10 ? 10 : value);
        }

        bool is_redundant(const EffectControlPoint* existing) const {
            return existing != 0 && kiai_mode == existing->kiai_mode &&
                   scroll_speed() == existing->scroll_speed();
        }
    };
}}} // namespace pppp::beatmaps::control_points

#endif
