// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/mods/osu_mod_target_practice.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/osu/object/osu_hit_object_generation_utils.h"
#include "pppp/osu/object/osu_playfield.h"
#include "pppp/utils/linq/csharp.h"
#include "pppp/utils/precision.h"
#include "pppp/utils/random/csharp.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace pppp { namespace osu { namespace mods {
    namespace control_points = ::pppp::beatmaps::control_points;
    namespace linq = ::pppp::utils::linq;
    namespace timing = ::pppp::beatmaps::timing;

    namespace {
        bool almost_bigger(double v1, double v2) { return utils::almost_bigger(v1, v2, TIMING_PRECISION); }

        bool definitely_bigger(double v1, double v2) {
            return utils::definitely_bigger(v1, v2, TIMING_PRECISION);
        }

        /// Check if a given time is inside a BreakPeriod.
        /// @remarks The given time is also considered to be inside a break if it is earlier than the
        /// start time of the first original hit object after the break.
        /// @param pb The beatmap whose breaks are checked.
        /// @param original Hit objects ordered by time.
        /// @param time The time to be checked.
        bool is_inside_break_period(const OsuBeatmap& pb, const std::vector<object::OsuHitObject>& original,
                                    double time) {
            for (size_t b = 0; b < pb.breaks.size(); b++) {
                const timing::BreakPeriod& br = pb.breaks[b];
                const object::OsuHitObject* first_after = 0;
                for (size_t k = 0; k < original.size(); k++) {
                    if (almost_bigger(original[k].time, br.end_time)) {
                        first_after = &original[k];
                        break;
                    }
                }
                // There should never really be a break section with no objects after it, but we've seen
                // crashes from users with malformed beatmaps, so it's best to guard against this.
                if (almost_bigger(time, br.start_time) &&
                    (!first_after || definitely_bigger(first_after->time, time))) {
                    return true;
                }
            }
            return false;
        }

        void get_beats_for_timing_point(const OsuBeatmap& pb, size_t tp_index, double map_end_time,
                                        std::vector<double>& beats) {
            const control_points::TimingControlPoint& tp = pb.info.timing_points[tp_index];
            int i = 0;
            double current_time = tp.time;
            while (!definitely_bigger(current_time, map_end_time) &&
                   timing_point_at(pb, current_time) == &tp) {
                beats.push_back(std::floor(current_time));
                i++;
                current_time = tp.time + i * tp.beat_length;
            }
        }

        std::vector<double> generate_beats(const OsuBeatmap& pb,
                                           const std::vector<object::OsuHitObject>& original) {
            double start_time = linq::from(pb.objects).first().time;
            double end_time = last_object_time(pb);
            std::vector<double> beats;
            for (size_t t = 0; t < pb.info.timing_points.size(); t++) {
                // Ignore timing points after endTime
                if (definitely_bigger(pb.info.timing_points[t].time, end_time)) {
                    continue;
                }
                // Generate the beats
                std::vector<double> tp_beats;
                get_beats_for_timing_point(pb, t, end_time, tp_beats);
                for (size_t k = 0; k < tp_beats.size(); k++) {
                    double beat = tp_beats[k];
                    // Remove beats before startTime
                    if (!almost_bigger(beat, start_time)) {
                        continue;
                    }
                    // Remove beats during breaks
                    if (is_inside_break_period(pb, original, beat)) {
                        continue;
                    }
                    beats.push_back(beat);
                }
            }

            // Remove beats that are too close to the next one (e.g. due to timing point changes)
            for (int i = static_cast<int>(beats.size()) - 2; i >= 0; i--) {
                double beat = beats[i];
                const control_points::TimingControlPoint* tp = timing_point_at(pb, beat);
                double beat_length = tp ? tp->beat_length : 1000.0;
                if (!definitely_bigger(beats[i + 1] - beat, beat_length / 2)) {
                    beats.erase(beats.begin() + i);
                }
            }
            return beats;
        }

        /// Re-maps a number from one range to another.
        /// @param value The number to be re-mapped.
        /// @param from_low Beginning of the original range.
        /// @param from_high End of the original range.
        /// @param to_low Beginning of the new range.
        /// @param to_high End of the new range.
        /// @returns The re-mapped number.
        float map_range(float value, float from_low, float from_high, float to_low, float to_high) {
            return (value - from_low) * (to_high - to_low) / (from_high - from_low) + to_low;
        }

        /// Move the hit object into playfield, taking its radius into account.
        /// @param obj The hit object to be clamped.
        /// @param radius The radius of the hit object.
        void clamp_to_playfield(object::OsuHitObject& obj, float radius) {
            utils::Vector2 p = obj.position;
            if (p.y < radius) {
                p.y = radius;
            } else if (p.y > object::PLAYFIELD_HEIGHT - radius) {
                p.y = object::PLAYFIELD_HEIGHT - radius;
            }
            if (p.x < radius) {
                p.x = radius;
            } else if (p.x > object::PLAYFIELD_WIDTH - radius) {
                p.x = object::PLAYFIELD_WIDTH - radius;
            }
            obj.position = p;
        }

        bool check_for_overlap(const linq::Enumerable<object::OsuHitObject>& objects_to_check,
                               const object::OsuHitObject& target, double target_radius) {
            return objects_to_check.any(linq::bind(&utils::distance,
                                                   linq::bind(&object::OsuHitObject::position, linq::_1),
                                                   target.position) < target_radius * 2);
        }

        void randomize_circle_pos(OsuBeatmap& pb, std::vector<object::OsuHitObject>& objs,
                                  utils::DotNetRandom& rng) {
            if (objs.empty()) {
                return;
            }
            const float two_pi = utils::PI_F * 2;
            double radius_d = object_radius(pb);
            float radius = static_cast<float>(radius_d);

            float direction = two_pi * static_cast<float>(rng.next_double() * 1.0f);
            int max_combo_index = linq::from(objs).last().combo_index;

            for (size_t i = 0; i < objs.size(); i++) {
                object::OsuHitObject& obj = objs[i];
                utils::Vector2 last_pos =
                    i == 0 ? utils::vec2(object::PLAYFIELD_WIDTH / 2, object::PLAYFIELD_HEIGHT / 2)
                           : objs[i - 1].position;

                float distance = max_combo_index == 0 ? radius
                                                      : map_range(static_cast<float>(obj.combo_index), 0,
                                                                  static_cast<float>(max_combo_index), radius,
                                                                  MAX_BASE_DISTANCE);
                if (obj.new_combo) {
                    distance *= 1.5f;
                }
                if (obj.kiai) {
                    distance *= 1.2f;
                }
                distance = std::min(DISTANCE_CAP, distance);

                // Attempt to place the circle at a place that does not overlap with previous ones
                int try_count = 0;

                // for checking overlap
                const linq::Enumerable<object::OsuHitObject> preceding_objects =
                    linq::from(objs)
                        .skip_last(static_cast<int>(objs.size() - i))
                        .take_last(OVERLAP_CHECK_COUNT);

                do {
                    if (try_count > 0) {
                        direction = two_pi * static_cast<float>(rng.next_double() * 1.0f);
                    }
                    utils::Vector2 relative =
                        utils::vec2(distance * std::cos(direction), distance * std::sin(direction));
                    // Rotate the new circle away from playfield border
                    relative = object::rotate_away_from_edge(last_pos, relative, EDGE_ROTATION_MULTIPLIER);
                    direction = std::atan2(relative.y, relative.x);
                    obj.position = last_pos + relative;
                    clamp_to_playfield(obj, radius);
                    try_count++;
                    if (try_count % 10 == 0) {
                        distance *= 0.9f;
                    }
                } while (distance >= radius_d * 2 && check_for_overlap(preceding_objects, obj, radius_d));

                if (obj.last_in_combo) {
                    direction = two_pi * static_cast<float>(rng.next_double() * 1.0f);
                } else {
                    direction += distance / DISTANCE_CAP *
                                 (static_cast<float>(rng.next_double() * 1.0f) * two_pi - utils::PI_F);
                }
            }
        }
    } // namespace

    Status apply_target_practice(OsuBeatmap& pb, const pppp::mods::Mod& mod) {
        if (pb.objects.empty()) {
            return StatusCode::OK;
        }
        utils::DotNetRandom rng(mod.target.seed.value());

        std::vector<object::OsuHitObject> original =
            linq::from(pb.objects).order_by(linq::bind(&object::OsuHitObject::time, linq::_1)).to_list();

        std::vector<double> beats = generate_beats(pb, original);

        double preempt = object::OsuHitObject::time_preempt_for_ar(pb.approach_rate);
        bool kiai_at_zero = kiai_at(pb, 0.0 + 1.0);
        std::vector<object::OsuHitObject> objs(beats.size());
        for (size_t i = 0; i < beats.size(); i++) {
            object::OsuHitObject& o = objs[i];
            o.type = 1;
            o.time = beats[i];
            o.end_time = beats[i];
            o.position = utils::vec2(0.0f, 0.0f);
            o.new_combo = false;
            o.combo_offset = 0;
            o.combo_index = 0;
            o.index_in_current_combo = 0;
            o.last_in_combo = false;
            o.kiai = kiai_at_zero;
            o.stack_height = 0;
            o.time_preempt = preempt;
            o.time_fade_in = object::OsuHitObject::time_fade_in_for_preempt(preempt);
            o.slider = -1;
        }

        for (size_t i = 0; i < objs.size(); i++) {
            int combo_index = 0;
            for (int k = static_cast<int>(original.size()) - 1; k >= 0; k--) {
                const object::OsuHitObject& orig = original[k];
                if (almost_bigger(objs[i].time, orig.time)) {
                    combo_index = orig.combo_index;
                    break;
                }
            }
            objs[i].combo_index = combo_index;
        }
        {
            std::vector<int> keys;
            std::vector<std::vector<size_t> > groups;
            for (size_t i = 0; i < objs.size(); i++) {
                size_t g = 0;
                for (; g < keys.size(); g++) {
                    if (keys[g] == objs[i].combo_index) {
                        break;
                    }
                }
                if (g == keys.size()) {
                    keys.push_back(objs[i].combo_index);
                    groups.push_back(std::vector<size_t>());
                }
                groups[g].push_back(i);
            }
            for (size_t g = 0; g < groups.size(); g++) {
                objs[groups[g].front()].new_combo = true;
                objs[groups[g].back()].last_in_combo = true;
                for (size_t j = 0; j < groups[g].size(); j++) {
                    objs[groups[g][j]].combo_index = static_cast<int>(g);
                    objs[groups[g][j]].index_in_current_combo = static_cast<int>(j);
                }
            }
        }

        randomize_circle_pos(pb, objs, rng);

        pb.objects.swap(objs);
        pb.sliders.clear();
        return StatusCode::OK;
    }
}}} // namespace pppp::osu::mods
