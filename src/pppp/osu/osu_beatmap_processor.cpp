// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/osu_beatmap_processor.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/utils/vector2.h"

namespace pppp { namespace osu {
    namespace {
        /// Truncation of the preempt time to an int, as well as keeping the result as a float, are
        /// both done for the purposes of stable compatibility. For top-level objects the preempt
        /// time is supposed to be integral anyway; slider ticks and end circles are the exception,
        /// but they do not matter for stacking.
        float calculate_stack_threshold(const OsuBeatmap& pb) {
            double preempt = object::OsuHitObject::time_preempt_for_ar(pb.approach_rate);
            return static_cast<float>(static_cast<int>(preempt)) * static_cast<float>(pb.stack_leniency);
        }

        void apply_stacking_range(std::vector<object::OsuHitObject>& hos,
                                  const std::vector<pppp::utils::Vector2>& raw,
                                  const std::vector<pppp::utils::Vector2>& end_pos, float stack_threshold) {
            const int n = static_cast<int>(hos.size());
            int start_index = 0;
            int extended_end_index = n - 1;
            int extended_start_index = start_index;

            for (int i = extended_end_index; i > start_index; i--) {
                int idx = i;
                int nn = i;

                // We should check every note which has not yet got a stack.
                // Consider the case we have two interwound stacks and this will make sense.
                //
                // o <-1      o <-2
                //  o <-3      o <-4
                //
                // We first process starting from 4 and handle 2,
                // then we come backwards on the i loop iteration until we reach 3 and handle 1.
                // 2 and 1 will be ignored in the i loop because they already have a stack value.
                if (hos[idx].stack_height != 0 || object::OsuHitObject::is_spinner_type(hos[idx].type)) {
                    continue;
                }

                // If this object is a hitcircle, then we enter this "special" case.
                // It either ends with a stack of hitcircles only, or a stack of hitcircles that are
                // underneath a slider. Any other case is handled by the "is Slider" code below this.
                if (!object::OsuHitObject::is_slider_type(hos[idx].type)) {
                    while (--nn >= 0) {
                        if (object::OsuHitObject::is_spinner_type(hos[nn].type)) {
                            continue;
                        }
                        double end_time = hos[nn].end_time;

                        // We are no longer within stacking range of the previous object.
                        if (static_cast<float>(static_cast<int>(hos[idx].time) - static_cast<int>(end_time)) >
                            stack_threshold) {
                            break;
                        }

                        // HitObjects before the specified update range haven't been reset yet
                        if (nn < extended_start_index) {
                            hos[nn].stack_height = 0;
                            extended_start_index = nn;
                        }

                        // This is a special case where hticircles are moved DOWN and RIGHT (negative
                        // stacking) if they are under the *last* slider in a stacked pattern.
                        //    o==o <- slider is at original location
                        //        o <- hitCircle has stack of -1
                        //         o <- hitCircle has stack of -2
                        if (object::OsuHitObject::is_slider_type(hos[nn].type) &&
                            distance(end_pos[nn], raw[idx]) < STACK_DISTANCE) {
                            int offset = hos[idx].stack_height - hos[nn].stack_height + 1;
                            for (int j = nn + 1; j <= i; j++) {
                                // For each object which was declared under this slider, we will offset it to
                                // appear *below* the slider end (rather than above).
                                if (distance(end_pos[nn], raw[j]) < STACK_DISTANCE) {
                                    hos[j].stack_height -= offset;
                                }
                            }

                            // We have hit a slider. We should restart calculation using this as the new base.
                            // Breaking here will mean that the slider still has StackCount of 0, so will be
                            // handled in the i-outer-loop.
                            break;
                        }

                        if (distance(raw[nn], raw[idx]) < STACK_DISTANCE) {
                            // Keep processing as if there are no sliders. If we come across a slider, this
                            // gets cancelled out.
                            // NOTE: Sliders with start positions stacking are a special case that is also
                            // handled here.
                            hos[nn].stack_height = hos[idx].stack_height + 1;
                            idx = nn;
                        }
                    }
                } else {
                    // We have hit the first slider in a possible stack.
                    // From this point on, we ALWAYS stack positive regardless.
                    while (--nn >= start_index) {
                        if (object::OsuHitObject::is_spinner_type(hos[nn].type)) {
                            continue;
                        }

                        // We are no longer within stacking range of the previous object.
                        if (hos[idx].time - hos[nn].time > stack_threshold) {
                            break;
                        }
                        if (distance(end_pos[nn], raw[idx]) < STACK_DISTANCE) {
                            hos[nn].stack_height = hos[idx].stack_height + 1;
                            idx = nn;
                        }
                    }
                }
            }
        }

        void apply_stacking_old(std::vector<object::OsuHitObject>& hos,
                                const std::vector<pppp::utils::Vector2>& raw,
                                const std::vector<pppp::utils::Vector2>& path_end_pos,
                                float stack_threshold) {
            const int n = static_cast<int>(hos.size());

            for (int i = 0; i < n; i++) {
                if (hos[i].stack_height != 0 && !object::OsuHitObject::is_slider_type(hos[i].type)) {
                    continue;
                }
                double start_time = hos[i].end_time;
                int slider_stack = 0;
                for (int j = i + 1; j < n; j++) {
                    if (hos[j].time - stack_threshold > start_time) {
                        break;
                    }
                    pppp::utils::Vector2 position2 =
                        object::OsuHitObject::is_slider_type(hos[i].type) ? path_end_pos[i] : raw[i];

                    // Note the use of StartTime in the code below doesn't match stable's use of EndTime.
                    // This is because in the stable implementation, `UpdateCalculations` is not called on the
                    // inner-loop hitobject (j) and therefore it does not have a correct `EndTime`, but
                    // instead the default of `EndTime = StartTime`.
                    //
                    // Effects of this can be seen on https://osu.ppy.sh/beatmapsets/243#osu/1146 at sliders
                    // around 86647 ms, where if we use `EndTime` here it would result in unexpected stacking.

                    if (distance(raw[j], raw[i]) < STACK_DISTANCE) {
                        hos[i].stack_height++;
                        start_time = hos[j].time;
                    } else if (distance(raw[j], position2) < STACK_DISTANCE) {
                        // Case for sliders - bump notes down and right, rather than up and left.
                        slider_stack++;
                        hos[j].stack_height -= slider_stack;
                        start_time = hos[j].time;
                    }
                }
            }
        }
    } // namespace

    void update_combo_information(OsuBeatmap& pb) {
        const object::OsuHitObject* last = 0;
        for (size_t i = 0; i < pb.objects.size(); i++) {
            object::OsuHitObject& o = pb.objects[i];

            // For sanity, ensures that both the first hitobject and the first hitobject after a spinner
            // start a new combo. This is normally enforced by the legacy decoder, but is not enforced by
            // the editor.
            bool spinner = (o.type & 8) != 0;
            bool last_spinner = last && (last->type & 8) != 0;
            if (!spinner && (!last || last_spinner)) {
                o.new_combo = true;
            }

            int index = last ? last->combo_index : 0;
            int in_current = last ? last->index_in_current_combo + 1 : 0;

            // - For the purpose of combo colours, spinners never start a new combo even if they are flagged
            //   as doing so.
            // - At decode time, the first hitobject in the beatmap and the first hitobject after a spinner
            //   are both enforced to be a new combo, but this isn't directly enforced by the editor so the
            //   extra checks against the last hitobject are duplicated here.
            if (!spinner && (o.new_combo || !last || last_spinner)) {
                in_current = 0;
                index++;
                if (last) {
                    pb.objects[i - 1].last_in_combo = true;
                }
            }
            o.combo_index = index;
            o.index_in_current_combo = in_current;
            o.last_in_combo = false;
            last = &pb.objects[i];
        }
    }

    void apply_stacking(OsuBeatmap& pb) {
        const int n = static_cast<int>(pb.objects.size());

        // Reset stacking
        for (int i = 0; i < n; i++) {
            pb.objects[i].stack_height = 0;
        }
        if (n == 0) {
            return;
        }

        std::vector<pppp::utils::Vector2> raw(n), end_pos(n), path_end_pos(n);
        for (int i = 0; i < n; i++) {
            raw[i] = pb.objects[i].position;
            end_pos[i] = object_end_position(pb, i);
            path_end_pos[i] = raw[i];
            if (pb.objects[i].slider >= 0 && !pb.sliders[pb.objects[i].slider].path.empty()) {
                path_end_pos[i] = raw[i] + pb.sliders[pb.objects[i].slider].path.back();
            }
        }

        const float stack_threshold = calculate_stack_threshold(pb);

        if (pb.format_version >= 6) {
            apply_stacking_range(pb.objects, raw, end_pos, stack_threshold);
        } else {
            apply_stacking_old(pb.objects, raw, path_end_pos, stack_threshold);
        }
    }
}} // namespace pppp::osu
