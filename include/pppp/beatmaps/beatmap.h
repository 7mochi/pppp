// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_BEATMAP_H
#define PPPP_BEATMAPS_BEATMAP_H

#include "pppp/beatmaps/slider_event_descriptor.h"
#include "pppp/beatmaps/timing/break_period.h"
#include "pppp/utils/vector2.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace beatmaps {
    /// A representation of all top-level difficulty settings for a beatmap.
    struct BeatmapDifficulty {
        /// The drain rate of the associated beatmap.
        double drain_rate;

        /// The circle size of the associated beatmap.
        double circle_size;

        /// The overall difficulty of the associated beatmap.
        double overall_difficulty;

        /// The approach rate of the associated beatmap.
        double approach_rate;

        /// The base slider velocity of the associated beatmap.
        /// This was known as "SliderMultiplier" in the .osu format and stable editor.
        double slider_multiplier;

        /// The slider tick rate of the associated beatmap.
        double slider_tick_rate;
    };

    struct Slider {
        int slides;
        double expected_length;

        std::vector<unsigned> node_sounds;
        std::vector<pppp::utils::Vector2> control_points;
        std::vector<pppp::utils::Vector2> path;
        std::vector<double> cumulative_lengths;
        std::vector<pppp::utils::Vector2> undecimated_path;
        std::vector<double> undecimated_cumulative_lengths;
        std::vector<pppp::beatmaps::SliderEventDescriptor> events;
        std::vector<pppp::beatmaps::SliderEventDescriptor> catch_events;
    };

    /// A HitObject describes an object in a Beatmap.
    struct HitObject {
        pppp::utils::Vector2 position;
        unsigned type;
        unsigned hitsound;

        /// The time at which the HitObject starts.
        double start_time;

        double end_time;
        bool new_combo;
        int combo_offset;
        int slider;
    };

    // The raw timing point line the decoder reads, before the control point classes.
    struct TimingPoint {
        double time;

        /// The beat length at this control point.
        double beat_length;

        /// The time signature at this control point.
        int meter;

        bool uninherited;
        unsigned effects;
    };

    struct SliderPathOps {
        int (*recompute)(void* ctx, unsigned hit_object_index, const pppp::utils::Vector2* relative_points,
                         size_t count, std::vector<pppp::utils::Vector2>* path,
                         std::vector<double>* cumulative_lengths);
        void (*release)(void* ctx);
        void* ctx;
    };

    class Beatmap {
    public:
        int format_version;
        int mode;
        double stack_leniency;

        BeatmapDifficulty difficulty;
        std::vector<HitObject> hit_objects;
        std::vector<Slider> sliders;
        std::vector<TimingPoint> timing_points;

        std::vector<pppp::beatmaps::timing::BreakPeriod> breaks;

        SliderPathOps slider_path;

        Beatmap();
        ~Beatmap();

        void clear();

    private:
        Beatmap(const Beatmap&);
        Beatmap& operator=(const Beatmap&);
    };
}} // namespace pppp::beatmaps

#endif
