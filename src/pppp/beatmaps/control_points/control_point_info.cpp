// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/beatmaps/control_points/control_point_info.h"

namespace pppp { namespace beatmaps { namespace control_points {
    void ControlPointInfo::build(const Beatmap& beatmap) {
        timing_points.clear();
        difficulty_points.clear();
        effect_points.clear();

        for (size_t i = 0; i < beatmap.timing_points.size(); i++) {
            const TimingPoint& tp = beatmap.timing_points[i];

            const double velocity = tp.beat_length < 0 ? 100.0 / -tp.beat_length : 1.0;
            const bool kiai = (tp.effects & 1) != 0;

            if (tp.uninherited) {
                if (timing_points.empty() || timing_points.back().time != tp.time) {
                    timing_points.push_back(TimingControlPoint(tp.time, tp.beat_length, tp.meter));
                } else {
                    timing_points.back().beat_length = tp.beat_length;
                    timing_points.back().time_signature = tp.meter;
                }
            }

            if (difficulty_points.empty() || difficulty_points.back().time != tp.time) {
                difficulty_points.push_back(DifficultyControlPoint(tp.time, velocity));
            } else {
                difficulty_points.back().slider_velocity = velocity;
            }

            if (effect_points.empty() || effect_points.back().time != tp.time) {
                effect_points.push_back(EffectControlPoint(tp.time, kiai));
            } else {
                effect_points.back().kiai_mode = kiai;
            }
        }
    }

    const TimingControlPoint* ControlPointInfo::timing_point_at(double time) const {
        const TimingControlPoint* first = 0;
        const TimingControlPoint* found = 0;

        for (size_t i = 0; i < timing_points.size(); i++) {
            if (first == 0) {
                first = &timing_points[i];
            }
            if (timing_points[i].time > time) {
                break;
            }
            found = &timing_points[i];
        }

        return found != 0 ? found : first;
    }

    const DifficultyControlPoint* ControlPointInfo::difficulty_point_at(double time) const {
        const DifficultyControlPoint* found = 0;

        for (size_t i = 0; i < difficulty_points.size(); i++) {
            if (difficulty_points[i].time > time) {
                break;
            }
            found = &difficulty_points[i];
        }
        return found;
    }

    const EffectControlPoint* ControlPointInfo::effect_point_at(double time) const {
        const EffectControlPoint* found = 0;

        for (size_t i = 0; i < effect_points.size(); i++) {
            if (effect_points[i].time > time) {
                break;
            }
            found = &effect_points[i];
        }
        return found;
    }
}}} // namespace pppp::beatmaps::control_points
