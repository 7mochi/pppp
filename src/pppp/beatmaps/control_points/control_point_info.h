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
    enum EqualitySelection { first_found, leftmost, rightmost };

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

        /// Binary searches one of the control point lists to find the active control point at `time`.
        /// Includes logic for returning a specific point when no matching point is found.
        /// @param list The list to search.
        /// @param time The time to find the control point at.
        /// @param fallback The control point to use when `time` is before any control points.
        /// @returns The active control point at `time`, or a fallback `ControlPoint` if none found.
        template <typename T>
        static const T* binary_search_with_fallback(const std::vector<T>& list, double time,
                                                    const T* fallback) {
            const T* found = binary_search(list, time);
            return found != 0 ? found : fallback;
        }

        /// Binary searches one of the control point lists to find the active control point at `time`.
        /// @param list The list to search.
        /// @param time The time to find the control point at.
        /// @returns The active control point at `time`. Will return `null` if there are no control points, or
        /// if the time is before the first control point.
        template <typename T>
        static const T* binary_search(const std::vector<T>& list, double time) {
            int index = binary_search(list, time, rightmost);

            if (index < 0) {
                index = ~index - 1;
            }

            return index >= 0 ? &list[static_cast<size_t>(index)] : 0;
        }

        /// Binary searches one of the control point lists to find the active control point at `time`.
        /// @param list The list to search.
        /// @param time The time to find the control point at.
        /// @param equality_selection Determines which index to return if there are multiple exact matches.
        /// @returns The index of the control point at `time`. Will return the complement of the index of the
        /// control point after `time` if no exact match is found.
        template <typename T>
        static int binary_search(const std::vector<T>& list, double time,
                                 EqualitySelection equality_selection) {
            const int n = static_cast<int>(list.size());

            if (n == 0) {
                return -1;
            }

            if (time < list[0].time) {
                return -1;
            }

            if (time > list[list.size() - 1].time) {
                return ~n;
            }

            int l = 0;
            int r = n - 1;
            bool equality_found = false;

            while (l <= r) {
                const int pivot = l + ((r - l) >> 1);

                if (list[static_cast<size_t>(pivot)].time < time) {
                    l = pivot + 1;
                } else if (list[static_cast<size_t>(pivot)].time > time) {
                    r = pivot - 1;
                } else {
                    equality_found = true;

                    switch (equality_selection) {
                    case leftmost: r = pivot - 1; break;

                    case rightmost: l = pivot + 1; break;

                    default:
                    case first_found: return pivot;
                    }
                }
            }

            if (!equality_found) {
                return ~l;
            }

            switch (equality_selection) {
            case leftmost: return l;

            default:
            case rightmost: return l - 1;
            }
        }

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
