// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/catch_beatmap_processor.h"
#include "pppp/fruits/catch_beatmap.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include "pppp/utils/random/osu.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace fruits {
    void apply_offset(float& position, float amount) {
        if (amount > 0) {
            // Clamp to the right bound
            if (position + amount < object::PLAYFIELD_WIDTH) {
                position += amount;
            }
        } else {
            // Clamp to the left bound
            if (position + amount > 0) {
                position += amount;
            }
        }
    }

    void apply_random_offset(float& position, double max_offset, utils::LegacyRandom& rng) {
        bool right = rng.next_bool();
        float rand =
            std::min(20.0f, static_cast<float>(utils::f32(rng.next(0.0, std::max(0.0, max_offset)))));

        if (right) {
            // Clamp to the right bound
            if (position + rand <= object::PLAYFIELD_WIDTH) {
                position += rand;
            } else {
                position -= rand;
            }
        } else {
            // Clamp to the left bound
            if (position - rand >= 0) {
                position -= rand;
            } else {
                position += rand;
            }
        }
    }

    void apply_hard_rock_offset(object::CatchHitObject& object, bool& has_last, float& last_position,
                                double& last_start_time, utils::LegacyRandom& rng) {
        float offset_position = static_cast<float>(object.original_x);
        double start_time = object.time;

        // some objects can get assigned position zero, making stable incorrectly go inside this if branch on
        // the next object. to maintain behaviour and compatibility, do the same here. reference:
        // https://github.com/peppy/osu-stable-reference/blob/3ea48705eb67172c430371dcfc8a16a002ed0d3d/osu!/GameplayElements/HitObjects/Fruits/HitFactoryFruits.cs#L45-L50
        // todo: should be revisited and corrected later probably.
        if (!has_last || last_position == 0) {
            has_last = true;
            last_position = offset_position;
            last_start_time = start_time;
            return;
        }

        float position_diff = offset_position - last_position;

        // Todo: BUG!! Stable calculated time deltas as ints, which affects randomisation. This should be
        // changed to a double.
        int time_diff = static_cast<int>(start_time - last_start_time);

        if (time_diff > 1000) {
            last_position = offset_position;
            last_start_time = start_time;
            return;
        }

        if (position_diff == 0) {
            apply_random_offset(offset_position, time_diff / 4.0, rng);
            object.x_offset = offset_position - static_cast<float>(object.original_x);
            return;
        }

        // NOLINTNEXTLINE(bugprone-integer-division)
        if (std::fabs(position_diff) < static_cast<float>(time_diff / 3)) {
            apply_offset(offset_position, position_diff);
        }

        object.x_offset = offset_position - static_cast<float>(object.original_x);

        last_position = offset_position;
        last_start_time = start_time;
    }

    void initialise_hyper_dash(CatchBeatmap& pb) {
        std::vector<object::CatchHitObject*> palpable;
        palpable_objects(palpable, pb);

        double half_catcher_width = object::CatchHitObject::calculate_catch_width(pb.circle_size) / 2;

        // Todo: This is wrong. osu!stable calculated hyperdashes using the full catcher size, excluding the
        // margins. This should theoretically cause impossible scenarios, but practically, likely due to the
        // size of the playfield, it doesn't seem possible. For now, to bring gameplay (and diffcalc!)
        // completely in-line with stable, this code also uses the full catcher size.
        half_catcher_width /= object::CATCHER_ALLOWED_CATCH_RANGE;

        int last_direction = 0;
        double last_excess = half_catcher_width;

        for (size_t i = 0; i + 1 < palpable.size(); i++) {
            object::CatchHitObject& current = *palpable[i];
            const object::CatchHitObject& next = *palpable[i + 1];

            // Reset variables in-case values have changed (e.g. after applying HR)
            current.hyper_dash = false;
            current.distance_to_hyper_dash = 0.0;

            int this_direction = next.effective_x() > current.effective_x() ? 1 : -1;

            // Int truncation added to match osu!stable.
            double time_to_next =
                static_cast<float>(static_cast<int>(next.time) - static_cast<int>(current.time)) -
                (1000.0f / 60.0f / 4); // 1/4th of a frame of grace time, taken from osu-stable
            float position_gap =
                static_cast<float>(next.effective_x()) - static_cast<float>(current.effective_x());
            double distance_to_next = (position_gap < 0 ? -position_gap : position_gap) -
                                      (last_direction == this_direction ? last_excess : half_catcher_width);
            double distance_to_hyper =
                utils::f32(time_to_next * object::CATCHER_BASE_DASH_SPEED - distance_to_next);

            if (distance_to_hyper < 0) {
                current.hyper_dash = true;
                last_excess = half_catcher_width;
            } else {
                current.distance_to_hyper_dash = distance_to_hyper;
                last_excess = utils::math::clamp(distance_to_hyper, 0.0, half_catcher_width);
            }

            last_direction = this_direction;
        }
    }

    void apply_position_offsets(CatchBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                                bool hard_rock_offsets) {
        utils::LegacyRandom rng(RNG_SEED);

        bool has_last = false;
        float last_position = 0.0f;
        double last_start_time = 0.0;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            object::CatchHitObject& object = pb.objects[i];
            object.x_offset = 0.0;

            switch (object.kind) {
            case object::OBJECT_FRUIT:
                if (hard_rock_offsets) {
                    apply_hard_rock_offset(object, has_last, last_position, last_start_time, rng);
                }
                break;

            case object::OBJECT_BANANA_SHOWER:
                for (size_t k = 0; k < object.nested.size(); k++) {
                    object.nested[k].x_offset =
                        static_cast<float>(rng.next_double() * object::PLAYFIELD_WIDTH);
                    rng.next(); // osu!stable retrieved a random banana type
                    rng.next(); // osu!stable retrieved a random banana rotation
                    rng.next(); // osu!stable retrieved a random banana colour
                }
                break;

            case object::OBJECT_JUICE_STREAM: {
                const pppp::beatmaps::Slider& slider = beatmap.sliders[object.slider_index];

                if (!slider.control_points.empty()) {
                    has_last = true;
                    // Todo: BUG!! Stable used the last control point as the final position of the path, but
                    // it should use the computed path instead.
                    last_position =
                        static_cast<float>(slider.control_points[slider.control_points.size() - 1].x);
                }
                // Todo: BUG!! Stable attempted to use the end time of the stream, but referenced it too early
                // in execution and used the start time instead.
                last_start_time = object.time;

                for (size_t k = 0; k < object.nested.size(); k++) {
                    object::CatchHitObject& nested = object.nested[k];
                    nested.x_offset = 0.0;

                    if (nested.kind == object::OBJECT_TINY_DROPLET) {
                        nested.x_offset = utils::math::clamp(rng.next(-20, 20), -nested.original_x,
                                                             object::PLAYFIELD_WIDTH - nested.original_x);
                    } else if (nested.kind == object::OBJECT_DROPLET) {
                        rng.next(); // osu!stable retrieved a random droplet rotation
                    }
                }
                break;
            }

            default: break;
            }
        }

        initialise_hyper_dash(pb);
    }
}} // namespace pppp::fruits
