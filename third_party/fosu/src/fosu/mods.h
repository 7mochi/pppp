#ifndef FOSU_MODS_H
#define FOSU_MODS_H

#include <algorithm>

#include <fosu/beatmap.h>
#include <fosu/compiler.h>

namespace fosu {

    struct Mods {
        enum Value {
            None = 0,
            NoFail = 1u << 0,
            Easy = 1u << 1,
            TouchDevice = 1u << 2, // previously NoVideo
            Hidden = 1u << 3,
            HardRock = 1u << 4,
            SuddenDeath = 1u << 5,
            DoubleTime = 1u << 6,
            Relax = 1u << 7,
            HalfTime = 1u << 8,
            Nightcore = 1u << 9,
            Flashlight = 1u << 10,
            Autoplay = 1u << 11,
            SpunOut = 1u << 12,
            AutoPilot = 1u << 13,
            Perfect = 1u << 14,
            Key4 = 1u << 15,
            Key5 = 1u << 16,
            Key6 = 1u << 17,
            Key7 = 1u << 18,
            Key8 = 1u << 19,
            FadeIn = 1u << 20,
            Random = 1u << 21,
            Cinema = 1u << 22,
            TargetPractice = 1u << 23,
            Key9 = 1u << 24,
            KeyCoop = 1u << 25,
            Key1 = 1u << 26,
            Key3 = 1u << 27,
            Key2 = 1u << 28,
            ScoreV2 = 1u << 29,
            Mirror = 1u << 30
        };
    };

    inline bool has_mod(fosu_uint32 mods, Mods::Value mod) {
        return (mods & static_cast<fosu_uint32>(mod)) != 0;
    }

    namespace internal {

        const fosu_uint32 kKnownMods =
            static_cast<fosu_uint32>(Mods::Easy) | static_cast<fosu_uint32>(Mods::HardRock) |
            static_cast<fosu_uint32>(Mods::DoubleTime) | static_cast<fosu_uint32>(Mods::HalfTime) |
            static_cast<fosu_uint32>(Mods::Nightcore);

        inline bool invalid_mods(fosu_uint32 mods) {
            const bool speed_up = has_mod(mods, Mods::DoubleTime) || has_mod(mods, Mods::Nightcore);
            return (mods & ~kKnownMods) != 0 ||
                   (has_mod(mods, Mods::Easy) && has_mod(mods, Mods::HardRock)) ||
                   (speed_up && has_mod(mods, Mods::HalfTime));
        }

        inline bool has_difficulty_mod(fosu_uint32 mods) {
            return has_mod(mods, Mods::Easy) || has_mod(mods, Mods::HardRock);
        }

        inline double clock_rate(fosu_uint32 mods) {
            if (has_mod(mods, Mods::DoubleTime) || has_mod(mods, Mods::Nightcore)) {
                return 1.5;
            }
            if (has_mod(mods, Mods::HalfTime)) {
                return 0.75;
            }
            return 1;
        }

        inline bool apply_mods_before_calculations(Beatmap& map, fosu_uint32 mods) {
            const bool easy = has_mod(mods, Mods::Easy);
            const bool hard_rock = has_mod(mods, Mods::HardRock);
            if (!easy && !hard_rock) {
                return true;
            }
            if (map.mode == 2 || map.mode == 3) {
                return false;
            }

            if (easy) {
                map.hp *= 0.5;
                map.cs *= 0.5;
                map.od *= 0.5;
                map.ar *= 0.5;
                if (map.mode == 1) {
                    map.slider_multiplier *= 0.8;
                }
                return true;
            }

            map.hp = std::min(map.hp * 1.4, 10.0);
            map.od = std::min(map.od * 1.4, 10.0);
            if (map.mode == 0) {
                map.cs = std::min(map.cs * 1.3, 10.0);
                map.ar = std::min(map.ar * 1.4, 10.0);
                for (size_t i = 0; i < map.hit_objects.size(); i++) {
                    map.hit_objects[i].y = 384 - map.hit_objects[i].y;
                }
                for (size_t i = 0; i < map.slider_points.size(); i++) {
                    map.slider_points[i].y = 384 - map.slider_points[i].y;
                }
            } else {
                map.slider_multiplier *= 1.4 * 4 / 3;
            }
            return true;
        }

        inline void apply_clock_rate(Beatmap& map, fosu_uint32 mods) {
            const double rate = clock_rate(mods);
            if (rate == 1) {
                return;
            }
            for (size_t i = 0; i < map.hit_objects.size(); i++) {
                map.hit_objects[i].time /= rate;
                map.hit_objects[i].end_time /= rate;
            }
            for (size_t i = 0; i < map.timing_points.size(); i++) {
                map.timing_points[i].time /= rate;
                if (map.timing_points[i].uninherited) {
                    map.timing_points[i].beat_length /= rate;
                }
            }
            for (size_t i = 0; i < map.breaks.size(); i++) {
                map.breaks[i].start /= rate;
                map.breaks[i].end /= rate;
            }
            for (size_t s = 0; s < map.slider_events.size(); s++) {
                Span<SliderEvent> events = map.slider_events[s];
                for (size_t e = 0; e < events.size(); e++) {
                    events[e].time /= rate;
                    events[e].span_start_time /= rate;
                }
            }
            for (size_t s = 0; s < map.catch_slider_events.size(); s++) {
                Span<SliderEvent> events = map.catch_slider_events[s];
                for (size_t e = 0; e < events.size(); e++) {
                    events[e].time /= rate;
                    events[e].span_start_time /= rate;
                }
            }
        }

    } // namespace internal
} // namespace fosu

#endif
