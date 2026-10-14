// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_DIFFICULTY_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_DIFFICULTY_CONTROL_POINT_H

#include "pppp/beatmaps/control_points/control_point.h"

namespace pppp { namespace beatmaps { namespace control_points {
    /// @remarks Note that going forward, this control point type should always be assigned directly
    /// to HitObjects.
    class DifficultyControlPoint : public ControlPoint {
    private:
        double slider_velocity_value;

    public:
        /// Whether or not slider ticks should be generated at this control point.
        /// This exists for backwards compatibility with maps that abuse NaN slider velocity behavior on
        /// osu!stable (e.g. /b/2628991).
        bool generate_ticks;

        DifficultyControlPoint(double time_in, double slider_velocity_in)
            : ControlPoint(time_in),
              slider_velocity_value(1),
              generate_ticks(true) {
            set_slider_velocity(slider_velocity_in);
        }

        /// The slider velocity at this control point.
        double slider_velocity() const { return slider_velocity_value; }

        void set_slider_velocity(double value) {
            slider_velocity_value = value < 0.1 ? 0.1 : (value > 10 ? 10 : value);
        }

        bool is_redundant(const DifficultyControlPoint* existing) const {
            return existing != 0 && generate_ticks == existing->generate_ticks &&
                   slider_velocity() == existing->slider_velocity();
        }
    };
}}} // namespace pppp::beatmaps::control_points

#endif
