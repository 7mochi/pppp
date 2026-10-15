// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/config.h" // IWYU pragma: export
#include <set>

namespace pppp { namespace beatmaps { namespace control_points {
    namespace {
        typedef nonstd::variant<TimingControlPoint, DifficultyControlPoint, EffectControlPoint>
            PendingControlPoint;

        template <typename T>
        void add_to_group(std::vector<T>& points, double time, T point) {
            point.time = time;

            const int i = ControlPointInfo::binary_search(points, time, first_found);

            if (i >= 0) {
                points[static_cast<size_t>(i)] = point;
                return;
            }

            points.insert(points.begin() + static_cast<ptrdiff_t>(~i), point);
        }

        class LegacyBeatmapDecoder {
        public:
            explicit LegacyBeatmapDecoder(ControlPointInfo& control_point_info_in)
                : control_point_info(control_point_info_in),
                  pending_control_points_time(0.0) {}

            void add_control_point(double time, const PendingControlPoint& point, bool timing_change) {
                if (time != pending_control_points_time) {
                    flush_pending_points();
                }

                if (timing_change) {
                    pending_control_points.insert(pending_control_points.begin(), point);
                } else {
                    pending_control_points.push_back(point);
                }

                pending_control_points_time = time;
            }

            void flush_pending_points() {
                // Changes from non-timing-points are added to the end of the list (see addControlPoint()) and
                // should override any changes from timing-points (added to the start of the list).
                for (size_t i = pending_control_points.size(); i > 0; i--) {
                    const PendingControlPoint& point = pending_control_points[i - 1];
                    if (!pending_control_point_types.insert(point.index()).second) {
                        continue;
                    }

                    add(pending_control_points_time, point);
                }

                pending_control_points.clear();
                pending_control_point_types.clear();
            }

        private:
            ControlPointInfo& control_point_info;
            std::vector<PendingControlPoint> pending_control_points;
            std::set<size_t> pending_control_point_types;
            double pending_control_points_time;

            void add(double time, const PendingControlPoint& point) {
                if (const TimingControlPoint* timing = nonstd::get_if<TimingControlPoint>(&point)) {
                    control_point_info.add(time, *timing);
                } else if (const DifficultyControlPoint* difficulty =
                               nonstd::get_if<DifficultyControlPoint>(&point)) {
                    control_point_info.add(time, *difficulty);
                } else if (const EffectControlPoint* effect = nonstd::get_if<EffectControlPoint>(&point)) {
                    control_point_info.add(time, *effect);
                }
            }
        };
    } // namespace

    void ControlPointInfo::build(const Beatmap& beatmap) {
        timing_points.clear();
        difficulty_points.clear();
        effect_points.clear();

        LegacyBeatmapDecoder decoder(*this);

        for (size_t i = 0; i < beatmap.timing_points.size(); i++) {
            const TimingPoint& line = beatmap.timing_points[i];

            const double time = line.time;

            // beatLength is allowed to be NaN to handle an edge case in which some beatmaps use NaN slider
            // velocity to disable slider tick generation.
            const double beat_length = line.beat_length;

            // If beatLength is NaN, speedMultiplier should still be 1 because all comparisons against NaN are
            // false.
            const double speed_multiplier = beat_length < 0 ? 100.0 / -beat_length : 1;

            const bool timing_change = line.uninherited;
            const bool kiai_mode = (line.effects & 1) != 0;

            if (timing_change) {
                decoder.add_control_point(time, TimingControlPoint(time, beat_length, line.meter), true);
            }

            DifficultyControlPoint difficulty_point(time, speed_multiplier);
            difficulty_point.generate_ticks = beat_length == beat_length;
            decoder.add_control_point(time, difficulty_point, timing_change);

            EffectControlPoint effect_point(time, kiai_mode);

            // osu!taiko and osu!mania use effect points rather than difficulty points for scroll speed
            // adjustments.
            if (beatmap.mode == 1 || beatmap.mode == 3) {
                effect_point.set_scroll_speed(speed_multiplier);
            }

            decoder.add_control_point(time, effect_point, timing_change);
        }

        decoder.flush_pending_points();
    }

    bool ControlPointInfo::add(double time, const TimingControlPoint& control_point) {
        if (check_already_existing(time, control_point)) {
            return false;
        }

        add_to_group(timing_points, time, control_point);
        return true;
    }

    bool ControlPointInfo::add(double time, const DifficultyControlPoint& control_point) {
        if (check_already_existing(time, control_point)) {
            return false;
        }

        add_to_group(difficulty_points, time, control_point);
        return true;
    }

    bool ControlPointInfo::add(double time, const EffectControlPoint& control_point) {
        if (check_already_existing(time, control_point)) {
            return false;
        }

        add_to_group(effect_points, time, control_point);
        return true;
    }

    bool ControlPointInfo::check_already_existing(double time, const TimingControlPoint& new_point) const {
        // Timing points are a special case and need to be added regardless of fallback availability.
        const TimingControlPoint* existing = binary_search(timing_points, time);

        return new_point.is_redundant(existing);
    }

    bool ControlPointInfo::check_already_existing(double time,
                                                  const DifficultyControlPoint& new_point) const {
        const DifficultyControlPoint fallback(0.0, 1.0);
        const DifficultyControlPoint* existing = difficulty_point_at(time);
        return new_point.is_redundant(existing != 0 ? existing : &fallback);
    }

    bool ControlPointInfo::check_already_existing(double time, const EffectControlPoint& new_point) const {
        const EffectControlPoint fallback(0.0, false);
        const EffectControlPoint* existing = effect_point_at(time);
        return new_point.is_redundant(existing != 0 ? existing : &fallback);
    }

    ControlPointInfo ControlPointInfo::deep_clone() const {
        ControlPointInfo control_point_info;

        for (size_t i = 0; i < timing_points.size(); i++) {
            control_point_info.add(timing_points[i].time, timing_points[i]);
        }
        for (size_t i = 0; i < difficulty_points.size(); i++) {
            control_point_info.add(difficulty_points[i].time, difficulty_points[i]);
        }
        for (size_t i = 0; i < effect_points.size(); i++) {
            control_point_info.add(effect_points[i].time, effect_points[i]);
        }

        return control_point_info;
    }

    const TimingControlPoint* ControlPointInfo::timing_point_at(double time) const {
        return binary_search_with_fallback(timing_points, time,
                                           timing_points.size() > 0 ? &timing_points[0] : 0);
    }

    const DifficultyControlPoint* ControlPointInfo::difficulty_point_at(double time) const {
        return binary_search(difficulty_points, time);
    }

    const EffectControlPoint* ControlPointInfo::effect_point_at(double time) const {
        return binary_search(effect_points, time);
    }
}}} // namespace pppp::beatmaps::control_points
