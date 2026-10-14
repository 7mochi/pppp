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

        /// Add a new `ControlPoint`.
        /// Note that the provided control point may not be added if the correct state is already present at
        /// the provided time.
        /// @param time The time at which the control point should be added.
        /// @param control_point The control point to add.
        /// @returns Whether the control point was added.
        bool add(double time, const TimingControlPoint& control_point);

        /// Add a new `ControlPoint`.
        /// Note that the provided control point may not be added if the correct state is already present at
        /// the provided time.
        /// @param time The time at which the control point should be added.
        /// @param control_point The control point to add.
        /// @returns Whether the control point was added.
        bool add(double time, const DifficultyControlPoint& control_point);

        /// Add a new `ControlPoint`.
        /// Note that the provided control point may not be added if the correct state is already present at
        /// the provided time.
        /// @param time The time at which the control point should be added.
        /// @param control_point The control point to add.
        /// @returns Whether the control point was added.
        bool add(double time, const EffectControlPoint& control_point);

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

        ControlPointInfo deep_clone() const;

    private:
        /// Check whether `new_point` should be added.
        /// @param time The time to find the timing control point at.
        /// @param new_point A point to be added.
        /// @returns Whether the new point should be added.
        bool check_already_existing(double time, const TimingControlPoint& new_point) const;

        /// Check whether `new_point` should be added.
        /// @param time The time to find the timing control point at.
        /// @param new_point A point to be added.
        /// @returns Whether the new point should be added.
        bool check_already_existing(double time, const DifficultyControlPoint& new_point) const;

        /// Check whether `new_point` should be added.
        /// @param time The time to find the timing control point at.
        /// @param new_point A point to be added.
        /// @returns Whether the new point should be added.
        bool check_already_existing(double time, const EffectControlPoint& new_point) const;
    };
}}} // namespace pppp::beatmaps::control_points

#endif
