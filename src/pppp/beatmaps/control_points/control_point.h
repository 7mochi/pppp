// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_CONTROL_POINT_H

namespace pppp { namespace beatmaps { namespace control_points {
    class ControlPoint {
    public:
        double time;

        explicit ControlPoint(double time_in)
            : time(time_in) {}
        virtual ~ControlPoint() {}
    };
}}} // namespace pppp::beatmaps::control_points

#endif
