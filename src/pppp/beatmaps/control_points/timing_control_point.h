// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_TIMING_CONTROL_POINT_H
#define PPPP_BEATMAPS_CONTROL_POINTS_TIMING_CONTROL_POINT_H

#include "pppp/beatmaps/control_points/control_point.h"

namespace pppp { namespace beatmaps { namespace control_points {
    const double DEFAULT_BEAT_LENGTH = 1000.0;

    class TimingControlPoint : public ControlPoint {
    private:
        double beat_length_value;

    public:
        /// The time signature at this control point.
        int time_signature;

        TimingControlPoint(double time_in, double beat_length_in, int time_signature_in)
            : ControlPoint(time_in),
              beat_length_value(DEFAULT_BEAT_LENGTH),
              time_signature(time_signature_in) {
            set_beat_length(beat_length_in);
        }

        /// The beat length at this control point.
        double beat_length() const { return beat_length_value; }

        void set_beat_length(double value) {
            beat_length_value = value < 6 ? 6 : (value > 60000 ? 60000 : value);
        }

        // Timing points are never redundant as they can change the time signature.
        bool is_redundant(const TimingControlPoint* existing) const {
            (void)existing;
            return false;
        }
    };
}}} // namespace pppp::beatmaps::control_points

#endif
