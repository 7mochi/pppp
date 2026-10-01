// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"
#include "pppp/beatmaps/slider_event_descriptor.h"
#include "pppp/osu/mods/osu_mod_hidden.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/osu/osu_beatmap.h"
#include "pppp/osu/osu_hit_windows.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace pppp { namespace osu { namespace difficulty { namespace preprocessing {
    namespace {
        double calculate_angle(pppp::utils::Vector2 current, pppp::utils::Vector2 last,
                               pppp::utils::Vector2 last_last) {
            pppp::utils::Vector2 v1 = last_last - last;
            pppp::utils::Vector2 v2 = current - last;

            float dot = pppp::utils::dot(v1, v2);
            float det = v1.x * v2.y - v1.y * v2.x;

            return std::fabs(std::atan2(static_cast<double>(det), static_cast<double>(dot)));
        }

        const pppp::beatmaps::SliderEventDescriptor* find_event(const object::PlayableSlider& slider,
                                                                pppp::beatmaps::SliderEventType type) {
            for (size_t j = 0; j < slider.events.size(); j++) {
                if (slider.events[j].type == type) {
                    return &slider.events[j];
                }
            }
            return 0;
        }

        void compute_slider_cursor_position(const OsuBeatmap& pb, size_t ho_index, double slider_radius,
                                            const PositionCtx& pos, pppp::utils::Vector2& lazy_end_out,
                                            double& lazy_travel_distance_out, double& lazy_travel_time_out) {
            const object::OsuHitObject& ho = pb.objects[ho_index];
            const object::PlayableSlider& slider = pb.sliders[ho.slider];
            pppp::utils::Vector2 stacked_pos = pos.stacked[ho_index];

            double duration = ho.end_time - ho.time;
            double tail_leniency = pppp::beatmaps::TAIL_LENIENCY;

            // TODO: This commented version is actually correct by the new lazer implementation, but
            // intentionally held back from difficulty calculator to preserve known behaviour. double
            // tracking_end_time = std::max(
            //     // SliderTailCircle always occurs at the final end time of the slider, but the player only
            //     needs to hold until within a lenience before it. ho.time + duration + tail_leniency,
            //     // There's an edge case where one or more ticks/repeats fall within that leniency range.
            //     // In such a case, the player needs to track until the final tick or repeat.
            //     last_nested_start_time
            // );

            double tracking_end_time = std::max(ho.time + duration + tail_leniency, ho.time + duration / 2.0);

            std::vector<const pppp::beatmaps::SliderEventDescriptor*> nested;
            const pppp::beatmaps::SliderEventDescriptor* last_real_tick = 0;
            for (size_t j = 0; j < slider.events.size(); j++) {
                const pppp::beatmaps::SliderEventDescriptor& evt = slider.events[j];
                if (evt.type == pppp::beatmaps::SLIDER_EVENT_LEGACY_LAST_TICK) {
                    continue;
                }
                nested.push_back(&evt);
                if (evt.type == pppp::beatmaps::SLIDER_EVENT_TICK) {
                    last_real_tick = &evt;
                }
            }

            if (last_real_tick && last_real_tick->time > tracking_end_time) {
                tracking_end_time = last_real_tick->time;

                // When the last tick falls after the tracking end time, we need to re-sort the nested objects
                // based on time. This creates a somewhat weird ordering which is counter to how a user would
                // understand the slider, but allows a zero-diff with known diffcalc output.
                //
                // To reiterate, this is definitely not correct from a difficulty calculation perspective
                // and should be revisited at a later date (likely by replacing this whole code with the
                // commented version above).
                nested.erase(std::find(nested.begin(), nested.end(), last_real_tick));
                nested.push_back(last_real_tick);
            }

            double lazy_travel_time = tracking_end_time - ho.time;

            double span_duration = duration / slider.slides;
            double end_time_min = lazy_travel_time / span_duration;
            if (fmod(end_time_min, 2.0) >= 1.0) {
                end_time_min = 1.0 - fmod(end_time_min, 1.0);
            } else {
                end_time_min = fmod(end_time_min, 1.0);
            }

            pppp::utils::Vector2 lazy_end = slider_position_at(slider, end_time_min);

            // lazySliderDistance is coded to be sensitive to scaling, this makes the maths easier with the
            // thresholds being used.
            double scaling_factor = NORMALISED_RADIUS / slider_radius;

            lazy_end =
                stacked_pos + lazy_end; // temporary lazy end position until a real result can be derived.

            pppp::utils::Vector2 curr_cursor = stacked_pos;
            double lazy_travel_distance = 0.0;

            for (size_t i = 1; i < nested.size(); i++) {
                const pppp::beatmaps::SliderEventDescriptor& evt = *nested[i];

                pppp::utils::Vector2 evt_pos = pos.nested(ho_index, evt.position);
                pppp::utils::Vector2 movement = evt_pos - curr_cursor;
                double movement_length = length(movement) * scaling_factor;

                // Amount of movement required so that the cursor position needs to be updated.
                double required_movement = ASSUMED_SLIDER_RADIUS;

                if (i == nested.size() - 1) {
                    // The end of a slider has special aim rules due to the relaxed time constraint on
                    // position. There is both a lazy end position as well as the actual end slider position.
                    // We assume the player takes the simpler movement. For sliders that are circular, the
                    // lazy end position may actually be farther away than the sliders true end. This code is
                    // designed to prevent buffing situations where lazy end is actually a less efficient
                    // movement.
                    pppp::utils::Vector2 lazy_movement = lazy_end - curr_cursor;
                    if (length(lazy_movement) < length(movement)) {
                        movement = lazy_movement;
                        movement_length = length(movement) * scaling_factor;
                    }
                } else if (evt.type == pppp::beatmaps::SLIDER_EVENT_REPEAT) {
                    // For a slider repeat, assume a tighter movement threshold to better assess repeat
                    // sliders.
                    required_movement = NORMALISED_RADIUS;
                }

                if (movement_length > required_movement) {
                    // this finds the positional delta from the required radius and the current position, and
                    // updates the currCursorPosition accordingly, as well as rewarding distance.
                    double fraction = (movement_length - required_movement) / movement_length;

                    curr_cursor = curr_cursor + movement * static_cast<float>(fraction);
                    lazy_travel_distance += movement_length * fraction;
                }
            }

            lazy_end_out = curr_cursor;
            lazy_travel_distance_out = lazy_travel_distance;
            lazy_travel_time_out = lazy_travel_time;
        }

        void set_distances(OsuDifficultyHitObject& dho, const OsuBeatmap& pb, const PositionCtx& pos,
                           double clock_rate, double cs) {
            const object::OsuHitObject& ho = pb.objects[dho.index + 1];
            const object::OsuHitObject& last_ho = pb.objects[dho.index];

            double radius = object::OsuHitObject::calculate_radius(cs);

            if (ho.slider >= 0) {
                // Bonus for repeat sliders until a better per nested object strain system can be achieved.
                dho.travel_distance =
                    dho.lazy_travel_distance *
                    std::max(1.0, utils::pow(static_cast<double>(pb.sliders[ho.slider].slides - 1), 0.3));
                dho.travel_time = std::max(dho.lazy_travel_time / clock_rate, MIN_DELTA_TIME);
            }

            dho.minimum_jump_time = dho.adjusted_delta_time;

            // We don't need to calculate either angle or distance when one of the last->curr objects is a
            // spinner
            if ((ho.type & 8) || (last_ho.type & 8)) {
                return;
            }

            // We will scale distances by this factor, so we can assume a uniform CircleSize among beatmaps.
            float scaling_factor = static_cast<float>(NORMALISED_RADIUS / static_cast<float>(radius));

            pppp::utils::Vector2 last_cursor_pos = dho.last_stacked_position;

            pppp::utils::Vector2 last_raw_pos = pos.stacked[dho.index];

            pppp::utils::Vector2 base_pos = dho.base_stacked_position;

            dho.jump_distance = length(base_pos - last_raw_pos) * scaling_factor;
            dho.lazy_jump_distance = length(base_pos - last_cursor_pos) * scaling_factor;
            dho.minimum_jump_distance = dho.lazy_jump_distance;

            //
            // There are two types of slider-to-object patterns to consider in order to better approximate the
            // real movement a player will take to jump between the hitobjects.
            //
            // 1. The anti-flow pattern, where players cut the slider short in order to move to the next
            // hitobject.
            //
            //      <======o==>  ← slider
            //             |     ← most natural jump path
            //             o     ← a follow-up hitcircle
            //
            // In this case the most natural jump path is approximated by the lazy jump distance.
            //
            // 2. The flow pattern, where players follow through the slider to its visual extent into the next
            // hitobject.
            //
            //      <======o==>---o
            //                  ↑
            //        most natural jump path
            //
            // In this case the most natural jump path is better approximated by a new distance, the
            // distance between the slider's tail and the next hitobject.
            //
            // Thus, the player is assumed to jump the minimum of these two distances in all cases.
            //
            if (dho.last_is_slider && dho.index > 0) {
                const OsuDifficultyHitObject* prev_dho =
                    static_cast<const OsuDifficultyHitObject*>(dho.previous(0));
                double last_travel_time_raw = prev_dho ? prev_dho->lazy_travel_time : 0.0;
                double last_travel_time = std::max(last_travel_time_raw / clock_rate, MIN_DELTA_TIME);
                dho.minimum_jump_time = std::max(dho.adjusted_delta_time - last_travel_time, MIN_DELTA_TIME);

                pppp::utils::Vector2 tail_pos = last_raw_pos;
                if (last_ho.slider >= 0) {
                    const pppp::beatmaps::SliderEventDescriptor* tail =
                        find_event(pb.sliders[last_ho.slider], pppp::beatmaps::SLIDER_EVENT_TAIL);
                    if (tail) {
                        tail_pos = pos.nested(dho.index, tail->position);
                    }
                }
                float tail_jump_dist = length(tail_pos - base_pos) * scaling_factor;

                dho.minimum_jump_distance = std::max(
                    0.0, std::min(dho.lazy_jump_distance - (MAXIMUM_SLIDER_RADIUS - ASSUMED_SLIDER_RADIUS),
                                  tail_jump_dist - MAXIMUM_SLIDER_RADIUS));
            }

            {
                const OsuDifficultyHitObject* prev2_dho =
                    static_cast<const OsuDifficultyHitObject*>(dho.previous(1));
                bool prev2_not_spinner = prev2_dho && !prev2_dho->base_is_spinner;

                if (prev2_not_spinner) {
                    pppp::utils::Vector2 last_last_pos;
                    if (prev2_dho->lazy_end_position.has_value()) {
                        last_last_pos.x = static_cast<float>(prev2_dho->lazy_end_position.value().x);
                        last_last_pos.y = static_cast<float>(prev2_dho->lazy_end_position.value().y);
                    } else {
                        last_last_pos = pos.stacked[dho.index - 1];
                    }

                    pppp::utils::Vector2 angle_cursor_pos = last_cursor_pos;
                    pppp::utils::Vector2 slider_angle_last_last_pos = last_last_pos;

                    if (dho.last_is_slider && dho.index >= 1) {
                        const OsuDifficultyHitObject* prev_dho =
                            static_cast<const OsuDifficultyHitObject*>(dho.previous(0));
                        if (prev_dho->travel_distance > 0) {
                            angle_cursor_pos = last_raw_pos;

                            if (last_ho.slider >= 0) {
                                const std::vector<pppp::beatmaps::SliderEventDescriptor>& events =
                                    pb.sliders[last_ho.slider].events;
                                int last_non_tail_idx = -1;
                                for (int e = static_cast<int>(events.size()) - 1; e >= 0; e--) {
                                    pppp::beatmaps::SliderEventType t = events[e].type;
                                    if (t != pppp::beatmaps::SLIDER_EVENT_TAIL &&
                                        t != pppp::beatmaps::SLIDER_EVENT_LEGACY_LAST_TICK) {
                                        last_non_tail_idx = e;
                                        break;
                                    }
                                }
                                if (last_non_tail_idx >= 0) {
                                    slider_angle_last_last_pos =
                                        pos.nested(dho.index, events[last_non_tail_idx].position);
                                }
                            }
                        }
                    }

                    double ang = calculate_angle(base_pos, angle_cursor_pos, last_last_pos);
                    double slider_ang =
                        calculate_angle(base_pos, last_cursor_pos, slider_angle_last_last_pos);
                    dho.angle = std::min(ang, slider_ang);

                    pppp::utils::Vector2 v = base_pos - angle_cursor_pos;
                    dho.normalised_vector_angle =
                        std::atan2(static_cast<double>(std::fabs(v.y)), static_cast<double>(std::fabs(v.x)));
                }
            }
        }

    } // namespace

    OsuDifficultyHitObject::OsuDifficultyHitObject(const object::OsuHitObject& object,
                                                   const object::OsuHitObject& last, double rate,
                                                   const std::vector<DifficultyHitObject*>& objects,
                                                   int object_index, const OsuBeatmap& pb,
                                                   const PositionCtx& pos)
        : DifficultyHitObject(object.time, last.time, rate, objects, object_index),
          base_is_slider(false),
          base_is_spinner(false),
          last_is_slider(false),
          adjusted_delta_time(0.0),
          last_object_end_delta_time(0.0),
          preempt(0.0),
          time_fade_in(0.0),
          jump_distance(0.0),
          lazy_jump_distance(0.0),
          minimum_jump_distance(0.0),
          minimum_jump_time(0.0),
          travel_distance(0.0),
          travel_time(0.0),
          lazy_end_position(),
          lazy_travel_distance(0.0),
          lazy_travel_time(0.0),
          angle(),
          normalised_vector_angle(),
          small_circle_bonus(0.0),
          overall_difficulty(0.0),
          circle_size(0.0),
          repeat_count(0) {
        base_stacked_position = pppp::utils::vec2(0.0f, 0.0f);
        base_stacked_end_position = pppp::utils::vec2(0.0f, 0.0f);
        last_stacked_position = pppp::utils::vec2(0.0f, 0.0f);

        double cs = pb.circle_size;
        pppp::osu::OsuHitWindows hit_windows;
        hit_windows.set_difficulty(pb.overall_difficulty);

        size_t i = static_cast<size_t>(object_index) + 1;

        base_stacked_position = pos.stacked[i];
        base_stacked_end_position = pos.stacked[i];
        if (object.slider >= 0) {
            const pppp::beatmaps::SliderEventDescriptor* tail =
                find_event(pb.sliders[object.slider], pppp::beatmaps::SLIDER_EVENT_TAIL);
            if (tail) {
                base_stacked_end_position = pos.nested(i, tail->position);
            }
        }
        base_is_slider = (object.type & 2) != 0;
        base_is_spinner = (object.type & 8) != 0;

        last_stacked_position = pos.stacked[i - 1];
        last_is_slider = (last.type & 2) != 0;

        end_time = object.end_time / rate;
        hit_window_great = 2.0 * hit_windows.window_for(pppp::common::HIT_RESULT_GREAT) / rate;

        adjusted_delta_time = std::max(delta_time, MIN_DELTA_TIME);

        if (object_index >= 1) {
            last_object_end_delta_time = std::max(start_time - last.end_time / rate, MIN_DELTA_TIME);
        } else {
            last_object_end_delta_time = adjusted_delta_time;
        }

        preempt = object.time_preempt / rate;
        time_fade_in = object.time_fade_in;
        small_circle_bonus = std::max(1.0, 1.0 + (30.0 - object::OsuHitObject::calculate_radius(cs)) / 70.0);
        overall_difficulty = (79.5 - hit_window_great / 2.0) / 6.0;
        circle_size = cs;

        lazy_end_position.reset();
        lazy_travel_distance = 0.0;
        lazy_travel_time = 0.0;
        travel_distance = 0.0;
        travel_time = 0.0;
        jump_distance = 0.0;
        lazy_jump_distance = 0.0;
        minimum_jump_distance = 0.0;
        minimum_jump_time = adjusted_delta_time;
        angle.reset();
        normalised_vector_angle.reset();
        repeat_count = (object.slider >= 0) ? pb.sliders[object.slider].slides - 1 : 0;

        if (object.slider >= 0) {
            pppp::utils::Vector2 lazy_end;
            double ltd, ltt;
            compute_slider_cursor_position(pb, i, object::OsuHitObject::calculate_radius(cs), pos, lazy_end,
                                           ltd, ltt);

            lazy_end_position = lazy_end;
            lazy_travel_distance = ltd;
            lazy_travel_time = ltt;
        }

        if (object_index >= 1) {
            const OsuDifficultyHitObject* prev_dho = static_cast<const OsuDifficultyHitObject*>(previous(0));

            last_stacked_position = prev_dho->base_stacked_position;
            if (prev_dho->lazy_end_position.has_value()) {
                last_stacked_position.x = static_cast<float>(prev_dho->lazy_end_position.value().x);
                last_stacked_position.y = static_cast<float>(prev_dho->lazy_end_position.value().y);
            }
        }

        set_distances(*this, pb, pos, rate, cs);
    }

    double calculate_double_tap_feasibility(const OsuDifficultyHitObject& prev_obj,
                                            const OsuDifficultyHitObject& curr_obj) {
        double curr_delta = std::max(1.0, prev_obj.delta_time);
        double next_delta = std::max(1.0, curr_obj.delta_time);

        double delta_difference = std::fabs(next_delta - curr_delta);

        double speed_ratio = curr_delta / std::max(curr_delta, delta_difference);
        double window_ratio = utils::pow(std::min(1.0, curr_delta / prev_obj.hit_window_great), 5);

        // Can't doubletap if circles don't intersect
        double distance_factor = utils::pow(
            utils::reverse_lerp(prev_obj.lazy_jump_distance, NORMALISED_DIAMETER, NORMALISED_RADIUS), 2);

        return 1.0 - utils::pow(speed_ratio, distance_factor * (1.0 - window_ratio));
    }

    double opacity_at(const OsuDifficultyHitObject& obj, double time, bool hidden) {
        double obj_start_time = obj.base_object_start_time;
        double time_preempt = obj.preempt * obj.clock_rate;

        if (time > obj_start_time) {
            // Consider a hitobject as being invisible when its start time is passed.
            // In reality the hitobject will be visible beyond its start time up until its hittable window has
            // passed, but this is an approximation and such a case is unlikely to be hit where this function
            // is used.
            return 0.0;
        }

        double fade_in_start_time = obj_start_time - time_preempt;

        // Equal to the hit object's fade-in time minus any adjustments from the HD mod.
        double fade_in_duration = 400.0 * std::min(1.0, time_preempt / object::PREEMPT_MIN);

        if (hidden) {
            double fade_out_start_time = obj_start_time - time_preempt + obj.time_fade_in;
            double fade_out_duration = time_preempt * pppp::osu::mods::FADE_OUT_DURATION_MULTIPLIER;

            return std::min(
                utils::math::clamp((time - fade_in_start_time) / fade_in_duration, 0.0, 1.0),
                1.0 - utils::math::clamp((time - fade_out_start_time) / fade_out_duration, 0.0, 1.0));
        }

        return utils::math::clamp((time - fade_in_start_time) / fade_in_duration, 0.0, 1.0);
    }
}}}} // namespace pppp::osu::difficulty::preprocessing
