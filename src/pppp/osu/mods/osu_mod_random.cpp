// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/mods/osu_mod_random.h"
#include "pppp/osu/object/osu_hit_object_generation_utils.h"
#include "pppp/osu/object/osu_playfield.h"
#include "pppp/osu/osu_beatmap_converter.h"
#include "pppp/utils/precision.h"
#include "pppp/utils/random/csharp.h"
#include "pppp/utils/vector2.h"
#include <cmath>
#include <vector>

namespace pppp { namespace osu { namespace mods {
    namespace {
        float playfield_diagonal() {
            return utils::length_fast(pppp::utils::vec2(object::PLAYFIELD_WIDTH, object::PLAYFIELD_HEIGHT));
        }

        struct RandomState {
            utils::DotNetRandom rng;
            float angle_sharpness;
            RandomState(int seed, float sharpness)
                : rng(seed),
                  angle_sharpness(sharpness) {}

            float get_random_offset(float std_dev) {
                // Range: [0.5, 2]
                // Higher angle sharpness -> lower multiplier
                const float max_value = 10.0f, default_value = 7.0f;
                float custom_multiplier =
                    (1.5f * max_value - angle_sharpness) / (1.5f * max_value - default_value);
                return object::random_gaussian(rng, 0, std_dev * custom_multiplier);
            }

            /// @param target_distance The target distance between the previous and the current OsuHitObject.
            /// @param offset The angle (in rad) by which the target angle should be offset.
            /// @param flow_direction Whether the relative angle should be positive or negative.
            float get_relative_target_angle(float target_distance, float offset, bool flow_direction) {
                // Range: [0.1, 1]
                float angle_sharpness_ratio = angle_sharpness / 10.0f;
                // Range: [0, 0.9]
                float angle_wideness = 1 - angle_sharpness_ratio;

                // Range: [-60, 30]
                float custom_offset_x = angle_sharpness_ratio * 100 - 70;
                // Range: [-0.075, 0.15]
                float custom_offset_y = angle_wideness * 0.25f - 0.075f;
                target_distance += custom_offset_x;
                float angle = static_cast<float>(
                    2.16 / (1 + 200 * std::exp(0.036 * (target_distance - 310 + custom_offset_x))) + 0.5);
                angle += offset + custom_offset_y;
                float relative_angle = utils::PI_F - angle;
                return flow_direction ? -relative_angle : relative_angle;
            }

            /// @returns Whether a new section should be started at the current OsuHitObject.
            bool should_start_new_section(const OsuBeatmap& pb,
                                          const std::vector<object::ObjectPositionInfo>& infos, size_t i) {
                if (i == 0) {
                    return true;
                }
                // Exclude new-combo-spam and 1-2-combos.
                const object::OsuHitObject& two_back = pb.objects[infos[i >= 2 ? i - 2 : 0].object];
                const object::OsuHitObject& prev = pb.objects[infos[i - 1].object];
                bool previous_started_combo = two_back.index_in_current_combo > 1 && prev.new_combo;
                bool on_downbeat = object::is_hit_object_on_beat(pb, prev, true);
                bool on_beat = object::is_hit_object_on_beat(pb, prev, false);

                if (previous_started_combo && rng.next_double() < 0.6f) {
                    return true;
                }
                if (on_downbeat) {
                    return true;
                }
                return on_beat && rng.next_double() < 0.4f;
            }

            /// @returns Whether a flow change should be applied at the current OsuHitObject.
            bool should_apply_flow_change(const OsuBeatmap& pb,
                                          const std::vector<object::ObjectPositionInfo>& infos, size_t i) {
                // Exclude new-combo-spam and 1-2-combos.
                const object::OsuHitObject& two_back = pb.objects[infos[i >= 2 ? i - 2 : 0].object];
                const object::OsuHitObject& prev = pb.objects[infos[i - 1].object];
                bool previous_started_combo = two_back.index_in_current_combo > 1 && prev.new_combo;
                return previous_started_combo && rng.next_double() < 0.6f;
            }
        };
    } // namespace

    Result::Value apply_random(OsuBeatmap& pb, const pppp::mods::Mod& mod) {
        if (pb.objects.empty()) {
            return Result::OK;
        }
        if (!pb.sliders.empty() && !can_recompute_sliders(pb)) {
            return Result::NO_SLIDER_PATH_BACKEND;
        }

        int seed = mod.random.seed.value();
        float sharpness = static_cast<float>(mod.random.angle_sharpness);
        if (sharpness < 1) {
            sharpness = 1;
        }
        if (sharpness > 10) {
            sharpness = 10;
        }
        RandomState st(seed, sharpness);

        std::vector<object::ObjectPositionInfo> infos = object::generate_position_infos(pb);

        // Offsets the angles of all hit objects in a "section" by the same amount.
        float section_offset = 0;

        // Whether the angles are positive or negative (clockwise or counter-clockwise flow).
        bool flow_direction = false;
        const float diagonal = playfield_diagonal();

        for (size_t i = 0; i < infos.size(); i++) {
            if (st.should_start_new_section(pb, infos, i)) {
                section_offset = st.get_random_offset(0.0008f);
                flow_direction = !flow_direction;
            }

            object::OsuHitObject& ho = pb.objects[infos[i].object];
            if (ho.slider >= 0 && st.rng.next_double() < 0.5) {
                object::flip_slider_in_place_horizontally(pb, ho.slider);
            }

            if (i == 0) {
                infos[i].distance_from_previous =
                    static_cast<float>(st.rng.next_double() * object::PLAYFIELD_HEIGHT / 2);
                infos[i].relative_angle = static_cast<float>(st.rng.next_double() * 2 * M_PI - M_PI);
            } else {
                // Offsets only the angle of the current hit object if a flow change occurs.
                float flow_change_offset = 0;

                // Offsets only the angle of the current hit object.
                float one_time_offset = st.get_random_offset(0.002f);
                if (st.should_apply_flow_change(pb, infos, i)) {
                    flow_change_offset = st.get_random_offset(0.002f);
                    flow_direction = !flow_direction;
                }
                float total_offset =
                    // sectionOffset and oneTimeOffset should mainly affect patterns with large spacing.
                    (section_offset + one_time_offset) * infos[i].distance_from_previous +
                    // flowChangeOffset should mainly affect streams.
                    flow_change_offset * (diagonal - infos[i].distance_from_previous);
                infos[i].relative_angle = st.get_relative_target_angle(infos[i].distance_from_previous,
                                                                       total_offset, flow_direction);
            }
        }

        object::reposition_hit_objects(pb, infos);
        return Result::OK;
    }
}}} // namespace pppp::osu::mods
