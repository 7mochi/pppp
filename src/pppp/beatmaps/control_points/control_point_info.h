// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_CONTROL_POINTS_CONTROL_POINT_INFO_H
#define PPPP_BEATMAPS_CONTROL_POINTS_CONTROL_POINT_INFO_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/control_points/difficulty_control_point.h"
#include "pppp/beatmaps/control_points/effect_control_point.h"
#include "pppp/beatmaps/control_points/timing_control_point.h"
#include <vector>

namespace pppp { namespace beatmaps { namespace control_points {
    class ControlPointInfo {
    public:
        std::vector<TimingControlPoint> timing_points;
        std::vector<DifficultyControlPoint> difficulty_points;
        std::vector<EffectControlPoint> effect_points;

        void build(const Beatmap& beatmap);

        /// Finds the timing control point that is active at `time`.
        /// @param time The time to find the timing control point at.
        /// @returns The timing control point.
        const TimingControlPoint* timing_point_at(double time) const;

        /// The difficulty point in force at time.
        const DifficultyControlPoint* difficulty_point_at(double time) const;

        /// Finds the effect control point that is active at `time`.
        /// @param time The time to find the effect control point at.
        /// @returns The effect control point.
        const EffectControlPoint* effect_point_at(double time) const;
    };
}}} // namespace pppp::beatmaps::control_points

#endif
