// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OBJECT_OSU_HIT_OBJECT_H
#define PPPP_OSU_OBJECT_OSU_HIT_OBJECT_H

#include "pppp/beatmaps/slider_event_descriptor.h"
#include "pppp/utils/difficulty_range.h"
#include "pppp/utils/vector2.h"
#include <vector>

namespace pppp { namespace osu { namespace object {
    /// The radius of hit objects (ie. the radius of a HitCircle).
    const double OBJECT_RADIUS = 64.0;

    /// Minimum preempt time at AR=10.
    const double PREEMPT_MIN = 450.0;

    /// Median preempt time at AR=5.
    const double PREEMPT_MID = 1200.0;

    /// Maximum preempt time at AR=0.
    const double PREEMPT_MAX = 1800.0;

    const utils::DifficultyRange PREEMPT_RANGE(PREEMPT_MAX, PREEMPT_MID, PREEMPT_MIN);

    struct OsuHitObject {
        unsigned type;
        double time;
        double end_time;
        pppp::utils::Vector2 position;
        bool new_combo;
        int combo_offset;
        int combo_index;
        int index_in_current_combo;
        bool last_in_combo;
        bool kiai;
        int stack_height;
        double time_preempt;
        double time_fade_in;
        int slider;

        static bool is_slider_type(unsigned type_in) { return (type_in & 2) != 0; }
        static bool is_spinner_type(unsigned type_in) { return (type_in & 8) != 0; }

        static double time_preempt_for_ar(double ar);
        static double time_fade_in_for_preempt(double preempt);

        /// Calculates scale from a CS value, with the fudge that was historically applied to the osu!
        /// ruleset.
        static double calculate_scale_from_cs(double cs);

        static double calculate_radius(double cs);
    };

    struct PlayableSlider {
        unsigned owner_object;
        int slides;
        double length;
        std::vector<pppp::utils::Vector2> control_points;
        std::vector<pppp::utils::Vector2> path;
        std::vector<double> cumulative_lengths;
        std::vector<pppp::beatmaps::SliderEventDescriptor> events;
    };
}}} // namespace pppp::osu::object

#endif
