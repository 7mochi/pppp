// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_TIMING_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_TIMING_CONTROL_POINT_H

#include "pppp/beatmaps/control_points/control_point.h"

namespace pppp { namespace beatmaps { namespace control_points {
    const double DEFAULT_BEAT_LENGTH = 1000.0;

    class TimingControlPoint : public ControlPoint {
    public:
        /// The time signature at this control point.
        int time_signature;

        /// The beat length at this control point.
        double beat_length;

        TimingControlPoint(double time_in, double beat_length_in, int time_signature_in)
            : ControlPoint(time_in),
              time_signature(time_signature_in),
              beat_length(beat_length_in < 6.0 ? 6.0
                                               : (beat_length_in > 60000.0 ? 60000.0 : beat_length_in)) {}
    };
}}} // namespace pppp::beatmaps::control_points

#endif
