// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/taiko_beatmap_converter.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/utils/precision.h"
#include <algorithm>

namespace pppp { namespace taiko {
    namespace {
        double precision_adjusted_beat_length(double slider_velocity, double beat_length) {
            double slider_velocity_as_beat_length = -100.0 / slider_velocity;
            double bpm_multiplier = 1.0;
            if (slider_velocity_as_beat_length < 0) {
                // The clamp happens on a float in stable.
                double clamped = utils::f32(-slider_velocity_as_beat_length);
                if (clamped < 10.0) {
                    clamped = 10.0;
                }
                if (clamped > 10000.0) {
                    clamped = 10000.0;
                }
                bpm_multiplier = clamped / 100.0;
            }
            return beat_length * bpm_multiplier;
        }

        double
        slider_velocity_at(const std::vector<pppp::beatmaps::control_points::DifficultyControlPoint>& points,
                           double time) {
            double velocity = 1.0;
            for (size_t i = 0; i < points.size(); i++) {
                if (points[i].time > time) {
                    break;
                }
                velocity = points[i].clamped_slider_velocity();
            }
            return velocity;
        }

        bool should_convert_slider_to_hits(
            const pppp::beatmaps::Beatmap& beatmap,
            const pppp::beatmaps::control_points::ControlPointInfo& info,
            const std::vector<pppp::beatmaps::control_points::DifficultyControlPoint>& difficulty_points,
            bool is_convert, const pppp::beatmaps::HitObject& ho, const pppp::beatmaps::Slider& slider,
            int* taiko_duration, double* tick_spacing) {
            // DO NOT CHANGE OR REFACTOR ANYTHING IN HERE WITHOUT TESTING AGAINST _ALL_ BEATMAPS.
            // Some of these calculations look redundant, but they are not - extremely small floating point
            // errors are introduced to maintain 1:1 compatibility with stable.
            // Rounding cannot be used as an alternative since the error deltas have been observed to be
            // between 1e-2 and 1e-6.

            // The true distance, accounting for any repeats. This ends up being the drum roll distance later
            int spans = slider.slides > 0 ? slider.slides : 1;
            double distance = slider.expected_length;

            // Do not combine the following two lines!
            distance *= VELOCITY_MULTIPLIER;
            distance *= spans;

            const pppp::beatmaps::control_points::TimingControlPoint* timing =
                info.timing_point_at(ho.start_time);
            double timing_beat_length =
                timing != 0 ? timing->beat_length : pppp::beatmaps::control_points::DEFAULT_BEAT_LENGTH;

            double slider_velocity = slider_velocity_at(difficulty_points, ho.start_time);
            double beat_length = precision_adjusted_beat_length(slider_velocity, timing_beat_length);

            double slider_scoring_point_distance =
                OSU_BASE_SCORING_DISTANCE * (beatmap.difficulty.slider_multiplier * VELOCITY_MULTIPLIER) /
                beatmap.difficulty.slider_tick_rate;

            // The velocity and duration of the taiko hit object - calculated as the velocity of a drum roll.
            double taiko_velocity = slider_scoring_point_distance * beatmap.difficulty.slider_tick_rate;
            *taiko_duration = static_cast<int>(distance / taiko_velocity * beat_length);

            if (!is_convert) {
                *tick_spacing = 0.0;
                return false;
            }

            double osu_velocity = taiko_velocity * (utils::f32(1000.0) / beat_length);

            // osu-stable always uses the speed-adjusted beatlength to determine the osu! velocity, but only
            // uses it for conversion if beatmap version < 8
            if (beatmap.format_version >= 8) {
                beat_length = timing_beat_length;
            }

            // If the drum roll is to be split into hit circles, assume the ticks are 1/8 spaced within the
            // duration of one beat
            *tick_spacing = std::min(beat_length / beatmap.difficulty.slider_tick_rate,
                                     static_cast<double>(*taiko_duration) / spans);

            return *tick_spacing > 0 && distance / osu_velocity * 1000.0 < 2.0 * beat_length;
        }

        void add_effect_point(std::vector<pppp::beatmaps::control_points::EffectControlPoint>& points,
                              double time, bool kiai, double scroll_speed) {
            bool existing_kiai = false;
            double existing_scroll = 1.0; // the default scroll speed
            size_t position = 0;
            while (position < points.size() && points[position].time <= time) {
                existing_kiai = points[position].kiai_mode;
                existing_scroll = points[position].scroll_speed;
                position++;
            }

            if (kiai == existing_kiai && scroll_speed == existing_scroll) {
                return;
            }

            if (position > 0 && points[position - 1].time == time) {
                points[position - 1].kiai_mode = kiai;
                points[position - 1].scroll_speed = scroll_speed;
                return;
            }

            pppp::beatmaps::control_points::EffectControlPoint ep(time, kiai);
            ep.scroll_speed = scroll_speed;
            points.insert(points.begin() + static_cast<ptrdiff_t>(position), ep);
        }

        bool kiai_at(const std::vector<pppp::beatmaps::control_points::EffectControlPoint>& points,
                     double time) {
            bool kiai = false;
            for (size_t i = 0; i < points.size(); i++) {
                if (points[i].time > time) {
                    break;
                }
                kiai = points[i].kiai_mode;
            }
            return kiai;
        }

    } // namespace

    void convert(TaikoBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                 const pppp::beatmaps::ObjectConverted<TaikoHitObject>& object_converted) {
        pb = TaikoBeatmap();
        pb.overall_difficulty = beatmap.difficulty.overall_difficulty;
        pb.drain_rate = beatmap.difficulty.drain_rate;
        pb.slider_multiplier = beatmap.difficulty.slider_multiplier;
        pb.is_convert = beatmap.mode == 0;

        pb.info.build(beatmap);

        // An effect point is dropped when neither the kiai flag nor the scroll speed differ from
        // whatever is already in force at that time, which is why most of a convert's control lines
        // leave no effect point at all.
        std::vector<pppp::beatmaps::control_points::EffectControlPoint> effect_points;
        for (size_t i = 0; i < pb.info.effect_points.size(); i++) {
            // Only taiko and mania read the point's velocity into the effect point; for a convert
            // it stays at 1 and the slider reconstruction below inserts the real values on top.
            double scroll_speed = 1.0;
            if (!pb.is_convert) {
                const pppp::beatmaps::control_points::DifficultyControlPoint* dp =
                    pb.info.difficulty_point_at(pb.info.effect_points[i].time);
                scroll_speed = dp != 0 ? dp->slider_velocity : 1.0;
                if (scroll_speed < 0.01) {
                    scroll_speed = 0.01;
                }
                if (scroll_speed > 10.0) {
                    scroll_speed = 10.0;
                }
            }
            add_effect_point(effect_points, pb.info.effect_points[i].time, pb.info.effect_points[i].kiai_mode,
                             scroll_speed);
        }
        pb.info.effect_points.swap(effect_points);

        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            const pppp::beatmaps::HitObject& ho = beatmap.hit_objects[i];
            const size_t first = pb.objects.size();

            // Old osu! used hit sounding to determine various hit type information
            if (ho.slider >= 0 && static_cast<size_t>(ho.slider) < beatmap.sliders.size()) {
                const pppp::beatmaps::Slider& slider = beatmap.sliders[ho.slider];
                int taiko_duration = 0;
                double tick_spacing = 0.0;

                if (should_convert_slider_to_hits(beatmap, pb.info, pb.info.difficulty_points, pb.is_convert,
                                                  ho, slider, &taiko_duration, &tick_spacing)) {
                    // The node sounds cycle over the slider's nodes; a node the map left out falls
                    // back to the slider's own hit sound.
                    size_t node_count = static_cast<size_t>(slider.slides > 0 ? slider.slides : 1) + 1;
                    size_t node = 0;

                    for (double j = ho.start_time; j <= ho.start_time + taiko_duration + tick_spacing / 8.0;
                         // NOLINTNEXTLINE(cert-flp30-c)
                         j += tick_spacing) {
                        unsigned sound =
                            node < slider.node_sounds.size() ? slider.node_sounds[node] : ho.hitsound;

                        TaikoHitObject out;
                        out.kind = object::OBJECT_HIT;
                        out.type = TaikoHitObject::hit_type_of(sound);
                        out.time = j;
                        out.duration = 0.0;
                        out.is_strong = (sound & object::HIT_SOUND_FINISH) != 0;
                        pb.objects.push_back(out);

                        node = (node + 1) % node_count;

                        if (utils::almost_equals(0.0, tick_spacing, 1e-7)) {
                            break;
                        }
                    }
                } else {
                    // TODO: stable makes the last tick of a drumroll non-required when the next
                    // object is too close. This probably needs to be reimplemented: if the object
                    // after this one is closer than a tick spacing past the drumroll's end, the
                    // last tick was not required in stable.
                    TaikoHitObject out;
                    out.kind = object::OBJECT_DRUM_ROLL;
                    out.type = object::HIT_TYPE_CENTRE;
                    out.time = ho.start_time;
                    out.duration = taiko_duration;
                    out.is_strong = (ho.hitsound & object::HIT_SOUND_FINISH) != 0;
                    pb.objects.push_back(out);
                }
            } else if ((ho.type & 8) != 0) {
                // RequiredHits is left out: swell ticks are bonus judgements, so they reach neither
                // the max combo nor any difficulty skill.
                TaikoHitObject out;
                out.kind = object::OBJECT_SWELL;
                out.type = object::HIT_TYPE_CENTRE;
                out.time = ho.start_time;
                out.duration = ho.end_time - ho.start_time;
                out.is_strong = false;
                pb.objects.push_back(out);
            } else {
                TaikoHitObject out;
                out.kind = object::OBJECT_HIT;
                out.type = TaikoHitObject::hit_type_of(ho.hitsound);
                out.time = ho.start_time;
                out.duration = 0.0;
                out.is_strong = (ho.hitsound & object::HIT_SOUND_FINISH) != 0;
                pb.objects.push_back(out);
            }

            if (object_converted.invoke != 0) {
                object_converted.invoke(i, pb.objects.size() > first ? &pb.objects[first] : 0,
                                        pb.objects.size() - first, object_converted.context);
            }
        }

        // Objects that a slider burst produced share start times with whatever follows, so the
        // order has to be preserved exactly.
        std::stable_sort(pb.objects.begin(), pb.objects.end(), hit_object_less);

        // Post processing step to transform standard slider velocity changes into scroll speed changes
        if (pb.is_convert) {
            double last_scroll_speed = 1.0;
            for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
                const pppp::beatmaps::HitObject& ho = beatmap.hit_objects[i];
                if (ho.slider < 0) {
                    continue; // only sliders carry a slider velocity
                }

                double next_scroll_speed = slider_velocity_at(pb.info.difficulty_points, ho.start_time);
                if (last_scroll_speed != next_scroll_speed) {
                    last_scroll_speed = next_scroll_speed;
                    add_effect_point(pb.info.effect_points, ho.start_time,
                                     kiai_at(pb.info.effect_points, ho.start_time), next_scroll_speed);
                }
            }
        }

        // Post processing step to transform mania hit objects with the same start time into strong hits
    }
}} // namespace pppp::taiko
