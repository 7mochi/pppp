// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/object/osu_hit_object_generation_utils.h"
#include "pppp/osu/object/osu_playfield.h"
#include "pppp/osu/osu_beatmap.h"
#include "pppp/osu/osu_beatmap_converter.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace object {
    namespace math = ::pppp::utils::math;

    namespace {
        /// Rotate a vector by the specified angle.
        /// @param v The vector to be rotated.
        /// @param rotation The angle, measured in radians, to rotate the vector by.
        /// @returns The rotated vector.
        utils::Vector2 rotate_vector(utils::Vector2 v, float rotation) {
            float angle = math::atan2f(v.y, v.x) + rotation;
            float len = length(v);
            return utils::vec2(len * math::cosf(angle), len * math::sinf(angle));
        }

        struct WorkingObject {
            float rotation_original;
            utils::Vector2 position_modified;
            utils::Vector2 end_position_modified;
            size_t info;
        };

        struct Bounds {
            float left, top, width, height;
            float right() const { return left + width; }
            float bottom() const { return top + height; }
        };

        /// Get the absolute rotation of a slider, defined as the angle from its start position to the end of
        /// its path.
        /// @param sl The slider to process.
        /// @returns The angle in radians.
        float get_slider_rotation(const PlayableSlider& sl) {
            utils::Vector2 end = slider_position_at(sl, 1);
            return math::atan2f(end.y, end.x);
        }

        /// Get the absolute difference between 2 angles measured in Radians.
        /// @param a1 The first angle
        /// @param a2 The second angle
        /// @returns The absolute difference with interval [0, PI)
        float get_angle_difference(float a1, float a2) {
            float diff = std::fmod(std::fabs(a1 - a2), utils::PI_F * 2);
            return std::min(diff, utils::PI_F * 2 - diff);
        }

        /// Clamp a position to playfield, keeping a specified distance from the edges.
        /// @param position The position to be clamped.
        /// @param padding The minimum distance allowed from playfield edges.
        /// @returns The clamped position.
        utils::Vector2 clamp_to_playfield_with_padding(utils::Vector2 position, float padding) {
            return utils::vec2(
                static_cast<float>(math::clamp(position.x, padding, PLAYFIELD_WIDTH - padding)),
                static_cast<float>(math::clamp(position.y, padding, PLAYFIELD_HEIGHT - padding)));
        }

        /// Estimate the centre of mass of a slider relative to its start position.
        /// @param sl The slider to process.
        /// @returns The centre of mass of the slider.
        utils::Vector2 calculate_centre_of_mass(const PlayableSlider& sl) {
            const double sample_step = 50;
            double path_distance = sl.cumulative_lengths.empty() ? 0.0 : sl.cumulative_lengths.back();
            if (path_distance <= sample_step) {
                utils::Vector2 end = slider_position_at(sl, 1);
                return utils::vec2(end.x / 2, end.y / 2);
            }
            int count = 0;
            utils::Vector2 sum = utils::vec2(0.0f, 0.0f);
            // NOLINTNEXTLINE(cert-flp30-c)
            for (double i = 0; i < path_distance; i += sample_step) {
                sum = sum + slider_position_at(sl, i / path_distance);
                count++;
            }
            return utils::vec2(sum.x / static_cast<float>(count), sum.y / static_cast<float>(count));
        }

        /// Calculates a rectangle which contains all of the possible movements of the slider (in relative X/Y
        /// coordinates) such that the entire slider is inside the playfield.
        /// @param pb The beatmap the slider belongs to, for its hit object radius.
        /// @param sl The slider for which to calculate a movement bounding box.
        /// @returns A rectangle which contains all of the possible movements of the slider such that the
        /// entire slider is inside the playfield.
        /// @remarks If the slider is larger than the playfield, the returned rectangle may have negative
        /// width/height.
        Bounds calculate_possible_movement_bounds(const OsuBeatmap& pb, const PlayableSlider& sl) {
            float min_x = INFINITY, max_x = -INFINITY, min_y = INFINITY, max_y = -INFINITY;
            for (size_t i = 0; i < sl.path.size(); i++) {
                min_x = std::min(min_x, sl.path[i].x);
                max_x = std::max(max_x, sl.path[i].x);
                min_y = std::min(min_y, sl.path[i].y);
                max_y = std::max(max_y, sl.path[i].y);
            }
            float radius = static_cast<float>(object_radius(pb));
            min_x -= radius;
            min_y -= radius;
            max_x += radius;
            max_y += radius;
            float left = -min_x, right = PLAYFIELD_WIDTH - max_x, top = -min_y,
                  bottom = PLAYFIELD_HEIGHT - max_y;
            Bounds b = {left, top, right - left, bottom - top};
            return b;
        }
    } // namespace

    utils::Vector2 rotate_vector_towards_vector(utils::Vector2 initial, utils::Vector2 destination,
                                                float rotation_ratio) {
        float initial_angle = math::atan2f(initial.y, initial.x);
        float dest_angle = math::atan2f(destination.y, destination.x);
        float diff = dest_angle - initial_angle;
        while (diff < -utils::PI_F) {
            diff += 2 * utils::PI_F;
        }
        while (diff > utils::PI_F) {
            diff -= 2 * utils::PI_F;
        }
        float final_angle = initial_angle + rotation_ratio * diff;
        float len = length(initial);
        return utils::vec2(len * math::cosf(final_angle), len * math::sinf(final_angle));
    }

    utils::Vector2 rotate_away_from_edge(utils::Vector2 prev_object_pos, utils::Vector2 pos_relative_to_prev,
                                         float rotation_ratio) {
        float relative_rotation_distance = 0.0f;
        utils::Vector2 middle = playfield_centre();
        if (prev_object_pos.x < middle.x) {
            relative_rotation_distance = std::max((BORDER_DISTANCE_X - prev_object_pos.x) / BORDER_DISTANCE_X,
                                                  relative_rotation_distance);
        } else {
            relative_rotation_distance =
                std::max((prev_object_pos.x - (PLAYFIELD_WIDTH - BORDER_DISTANCE_X)) / BORDER_DISTANCE_X,
                         relative_rotation_distance);
        }
        if (prev_object_pos.y < middle.y) {
            relative_rotation_distance = std::max((BORDER_DISTANCE_Y - prev_object_pos.y) / BORDER_DISTANCE_Y,
                                                  relative_rotation_distance);
        } else {
            relative_rotation_distance =
                std::max((prev_object_pos.y - (PLAYFIELD_HEIGHT - BORDER_DISTANCE_Y)) / BORDER_DISTANCE_Y,
                         relative_rotation_distance);
        }
        return rotate_vector_towards_vector(pos_relative_to_prev, middle - prev_object_pos,
                                            std::min(1.0f, relative_rotation_distance * rotation_ratio));
    }

    namespace {
        void compute_modified_position(OsuBeatmap& pb, const std::vector<ObjectPositionInfo>& infos,
                                       WorkingObject& current, const WorkingObject* previous,
                                       const WorkingObject* before_previous) {
            const ObjectPositionInfo& pi = infos[current.info];
            OsuHitObject& ho = pb.objects[pi.object];

            float previous_absolute_angle = 0.0f;
            if (previous) {
                const OsuHitObject& prev_ho = pb.objects[infos[previous->info].object];
                if (prev_ho.slider >= 0) {
                    previous_absolute_angle = get_slider_rotation(pb.sliders[prev_ho.slider]);
                } else {
                    utils::Vector2 earliest =
                        before_previous ? object_end_position(pb, infos[before_previous->info].object)
                                        : playfield_centre();
                    utils::Vector2 relative = prev_ho.position - earliest;
                    previous_absolute_angle = math::atan2f(relative.y, relative.x);
                }
            }

            float absolute_angle = previous_absolute_angle + pi.relative_angle;
            utils::Vector2 pos_relative_to_prev =
                utils::vec2(pi.distance_from_previous * math::cosf(absolute_angle),
                            pi.distance_from_previous * math::sinf(absolute_angle));
            utils::Vector2 last_end_position =
                previous ? previous->end_position_modified : playfield_centre();
            pos_relative_to_prev = rotate_away_from_edge(last_end_position, pos_relative_to_prev, 0.5f);
            current.position_modified = last_end_position + pos_relative_to_prev;

            if (ho.slider < 0) {
                return;
            }

            absolute_angle = math::atan2f(pos_relative_to_prev.y, pos_relative_to_prev.x);
            const PlayableSlider& sl = pb.sliders[ho.slider];
            utils::Vector2 com_original = calculate_centre_of_mass(sl);
            utils::Vector2 com_modified =
                rotate_vector(com_original, pi.rotation + absolute_angle - get_slider_rotation(sl));
            com_modified = rotate_away_from_edge(current.position_modified, com_modified, 0.5f);
            float relative_rotation =
                math::atan2f(com_modified.y, com_modified.x) - math::atan2f(com_original.y, com_original.x);
            if (!utils::almost_equals(relative_rotation, 0.0f)) {
                rotate_slider(pb, ho.slider, relative_rotation);
            }
        }

        utils::Vector2 clamp_hit_circle_to_playfield(OsuBeatmap& pb,
                                                     const std::vector<ObjectPositionInfo>& infos,
                                                     WorkingObject& wo) {
            OsuHitObject& ho = pb.objects[infos[wo.info].object];
            utils::Vector2 previous_position = wo.position_modified;
            wo.end_position_modified = wo.position_modified =
                clamp_to_playfield_with_padding(wo.position_modified, static_cast<float>(object_radius(pb)));
            ho.position = wo.position_modified;
            return wo.position_modified - previous_position;
        }

        utils::Vector2 clamp_slider_to_playfield(OsuBeatmap& pb, const std::vector<ObjectPositionInfo>& infos,
                                                 WorkingObject& wo) {
            size_t obj = infos[wo.info].object;
            OsuHitObject& ho = pb.objects[obj];
            int s = ho.slider;
            Bounds bounds = calculate_possible_movement_bounds(pb, pb.sliders[s]);

            if (bounds.width < 0 || bounds.height < 0) {
                float current_rotation = get_slider_rotation(pb.sliders[s]);
                float diff1 = get_angle_difference(wo.rotation_original, current_rotation);
                float diff2 = get_angle_difference(wo.rotation_original + utils::PI_F, current_rotation);
                if (diff1 < diff2) {
                    rotate_slider(pb, s, wo.rotation_original - get_slider_rotation(pb.sliders[s]));
                } else {
                    rotate_slider(pb, s,
                                  wo.rotation_original + utils::PI_F - get_slider_rotation(pb.sliders[s]));
                }
                bounds = calculate_possible_movement_bounds(pb, pb.sliders[s]);
            }

            utils::Vector2 previous_position = wo.position_modified;
            float new_x =
                bounds.width < 0
                    ? static_cast<float>(math::clamp(bounds.left, 0, PLAYFIELD_WIDTH))
                    : static_cast<float>(math::clamp(previous_position.x, bounds.left, bounds.right()));
            float new_y =
                bounds.height < 0
                    ? static_cast<float>(math::clamp(bounds.top, 0, PLAYFIELD_HEIGHT))
                    : static_cast<float>(math::clamp(previous_position.y, bounds.top, bounds.bottom()));
            ho.position = wo.position_modified = utils::vec2(new_x, new_y);
            wo.end_position_modified = object_end_position(pb, obj);
            return wo.position_modified - previous_position;
        }

        void apply_decreasing_shift(OsuBeatmap& pb, const std::vector<size_t>& objects,
                                    utils::Vector2 shift) {
            float radius = static_cast<float>(object_radius(pb));
            int count = static_cast<int>(objects.size());
            for (int i = 0; i < count; i++) {
                OsuHitObject& ho = pb.objects[objects[i]];
                utils::Vector2 position =
                    ho.position + shift * (static_cast<float>(count - i) / static_cast<float>(count + 1));
                ho.position = clamp_to_playfield_with_padding(position, radius);
            }
        }
    } // namespace

    void flip_slider_in_place_horizontally(OsuBeatmap& pb, int slider_index) {
        PlayableSlider& sl = pb.sliders[slider_index];
        for (size_t k = 0; k < sl.control_points.size(); k++) {
            utils::Vector2 flipped = {-sl.control_points[k].x, sl.control_points[k].y};
            set_control_point(sl.control_points[k], flipped);
        }
        slider_recompute(pb, slider_index);
    }

    void rotate_slider(OsuBeatmap& pb, int slider_index, float rotation) {
        PlayableSlider& sl = pb.sliders[slider_index];
        for (size_t k = 0; k < sl.control_points.size(); k++) {
            set_control_point(sl.control_points[k], rotate_vector(sl.control_points[k], rotation));
        }
        slider_recompute(pb, slider_index);
    }

    bool is_hit_object_on_beat(const OsuBeatmap& pb, const OsuHitObject& ho, bool downbeats_only) {
        const beatmaps::control_points::TimingControlPoint* tp = timing_point_at(pb, ho.time);
        double tp_time = tp ? tp->time : 0.0;
        double beat_length = tp ? tp->beat_length() : 1000.0;
        int meter = tp ? tp->time_signature : 4;
        double time_since = ho.time - tp_time;
        if (downbeats_only) {
            beat_length *= meter;
        }

        return std::fmod(std::fabs(time_since + 1), beat_length) < 2;
    }

    float random_gaussian(utils::DotNetRandom& rng, float mean, float std_dev) {
        double x1 = 1 - rng.next_double();
        double x2 = 1 - rng.next_double();
        double std_normal = std::sqrt(-2 * std::log(x1)) * std::sin(2 * utils::PI_D * x2);
        return mean + std_dev * static_cast<float>(std_normal);
    }

    std::vector<ObjectPositionInfo> generate_position_infos(const OsuBeatmap& pb) {
        std::vector<ObjectPositionInfo> infos;
        utils::Vector2 previous_position = playfield_centre();
        float previous_angle = 0.0f;
        for (size_t i = 0; i < pb.objects.size(); i++) {
            const OsuHitObject& ho = pb.objects[i];
            utils::Vector2 relative = ho.position - previous_position;
            float absolute_angle = math::atan2f(relative.y, relative.x);
            float relative_angle = absolute_angle - previous_angle;
            ObjectPositionInfo info;
            info.relative_angle = relative_angle;
            info.distance_from_previous = length(relative);
            info.rotation = 0.0f;
            info.object = i;
            if (ho.slider >= 0) {
                float absolute_rotation = get_slider_rotation(pb.sliders[ho.slider]);
                info.rotation = absolute_rotation - absolute_angle;
                absolute_angle = absolute_rotation;
            }
            infos.push_back(info);
            previous_position = object_end_position(pb, i);
            previous_angle = absolute_angle;
        }
        return infos;
    }

    void reposition_hit_objects(OsuBeatmap& pb, const std::vector<ObjectPositionInfo>& infos) {
        std::vector<WorkingObject> working(infos.size());
        for (size_t i = 0; i < infos.size(); i++) {
            const OsuHitObject& ho = pb.objects[infos[i].object];
            working[i].info = i;
            working[i].rotation_original = ho.slider >= 0 ? get_slider_rotation(pb.sliders[ho.slider]) : 0.0f;
            working[i].position_modified = ho.position;
            working[i].end_position_modified = object_end_position(pb, infos[i].object);
        }

        WorkingObject* previous = 0;
        for (size_t i = 0; i < working.size(); i++) {
            WorkingObject& current = working[i];
            OsuHitObject& ho = pb.objects[infos[i].object];
            if (ho.type & 8) {
                previous = &current;
                continue;
            }
            compute_modified_position(pb, infos, current, previous, i > 1 ? &working[i - 2] : 0);

            utils::Vector2 shift = utils::vec2(0.0f, 0.0f);
            if (ho.slider >= 0) {
                shift = clamp_slider_to_playfield(pb, infos, current);
            } else {
                shift = clamp_hit_circle_to_playfield(pb, infos, current);
            }

            if (shift.x != 0.0f || shift.y != 0.0f) {
                std::vector<size_t> to_be_shifted;
                for (int j = static_cast<int>(i) - 1;
                     j >= static_cast<int>(i) - PRECEDING_HITOBJECTS_TO_SHIFT && j >= 0; j--) {
                    const OsuHitObject& pj = pb.objects[infos[j].object];
                    // only shift hit circles
                    if (pj.slider >= 0 || (pj.type & 8)) {
                        break;
                    }
                    to_be_shifted.push_back(infos[j].object);
                }
                if (!to_be_shifted.empty()) {
                    apply_decreasing_shift(pb, to_be_shifted, shift);
                }
            }
            previous = &current;
        }
    }
}}} // namespace pppp::osu::object
