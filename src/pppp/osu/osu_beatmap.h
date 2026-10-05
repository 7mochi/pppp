// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OSU_BEATMAP_H
#define PPPP_OSU_OSU_BEATMAP_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/status.h"
#include "pppp/utils/vector2.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace osu {

    struct OsuBeatmap {
        const pppp::beatmaps::Beatmap* source;
        int format_version;
        double stack_leniency;
        double circle_size;
        double approach_rate;
        double overall_difficulty;
        double drain_rate;
        double clock_rate;
        std::vector<object::OsuHitObject> objects;
        std::vector<object::PlayableSlider> sliders;
        pppp::beatmaps::control_points::ControlPointInfo info;
        std::vector<pppp::beatmaps::timing::BreakPeriod> breaks;

        OsuBeatmap();
    };

    const pppp::beatmaps::control_points::TimingControlPoint* timing_point_at(const OsuBeatmap& pb,
                                                                              double time);
    bool kiai_at(const OsuBeatmap& pb, double time);

    void set_control_point(pppp::utils::Vector2& current, pppp::utils::Vector2 value);

    /// Computes the position on the slider at a given progress that ranges from 0 (beginning of the
    /// path) to 1 (end of the path).
    /// @param slider The slider whose path is sampled.
    /// @param progress Ranges from 0 (beginning of the path) to 1 (end of the path).
    pppp::utils::Vector2 slider_position_at(const object::PlayableSlider& slider, double progress);

    pppp::utils::Vector2 object_end_position(const OsuBeatmap& pb, size_t object_index);
    double object_radius(const OsuBeatmap& pb);

    /// Find the absolute end time of the latest HitObject in a beatmap.
    /// @remarks This correctly accounts for objects which may have durations, causing the last object
    /// not necessarily to have the latest end time.
    double last_object_time(const OsuBeatmap& pb);

    /// Finds the maximum achievable combo by hitting all HitObjects in a beatmap.
    int max_combo(const OsuBeatmap& pb);

    Status build(OsuBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                 size_t mod_count,
                 const pppp::beatmaps::ObjectConverted<object::OsuHitObject>& object_converted =
                     pppp::beatmaps::ObjectConverted<object::OsuHitObject>());
}} // namespace pppp::osu

#endif
