// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/flashlight_evaluator.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/vector2.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators {
    double FlashlightEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                                       bool has_hidden, double cs,
                                                       bool hidden_bonus_applies) {
        if (current.base_is_spinner) {
            return 0.0;
        }

        const double max_opacity_bonus = 0.4;
        const double hidden_bonus = 0.2;

        const double min_velocity = 0.5;
        const double slider_multiplier = 1.3;

        const double min_angle_multiplier = 0.2;

        double scaling_factor = 52.0 / object::OsuHitObject::calculate_radius(cs);
        double small_dist_nerf = 1.0;
        double cumulative_strain_time = 0.0;

        double flashlight_difficulty = 0.0;

        const preprocessing::OsuDifficultyHitObject* last_obj = &current;

        double angle_repeat_count = 0.0;

        // This is iterating backwards in time from the current object.
        int max_iter = std::min(current.index, 10);
        for (int i = 0; i < max_iter; i++) {
            const preprocessing::OsuDifficultyHitObject& current_obj =
                *static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(i));

            cumulative_strain_time += last_obj->adjusted_delta_time;

            if (!current_obj.base_is_spinner) {
                double jump_distance =
                    length(current.base_stacked_position - current_obj.base_stacked_end_position);

                // We want to nerf objects that can be easily seen within the Flashlight circle radius.
                if (i == 0) {
                    small_dist_nerf = std::min(1.0, jump_distance / 75.0);
                }

                // We also want to nerf stacks so that only the first object of the stack is accounted for.
                double stack_nerf = std::min(1.0, (current_obj.lazy_jump_distance / scaling_factor) / 25.0);

                // Bonus based on how visible the object is.
                double opacity_bonus =
                    1.0 + max_opacity_bonus *
                              (1.0 - opacity_at(current, current_obj.base_object_start_time, has_hidden));

                flashlight_difficulty +=
                    stack_nerf * opacity_bonus * scaling_factor * jump_distance / cumulative_strain_time;

                if (current_obj.angle.has_value() && current.angle.has_value()) {
                    // Objects further back in time should count less for the nerf.
                    if (std::fabs(current_obj.angle.value() - current.angle.value()) < 0.02) {
                        angle_repeat_count += std::max(1.0 - 0.1 * i, 0.0);
                    }
                }
            }

            last_obj = &current_obj;
        }

        flashlight_difficulty = utils::pow(small_dist_nerf * flashlight_difficulty, 2);

        // Additional bonus for Hidden due to there being no approach circles.
        if (hidden_bonus_applies) {
            flashlight_difficulty *= 1.0 + hidden_bonus;
        }

        // Nerf patterns with repeated angles.
        flashlight_difficulty *=
            min_angle_multiplier + (1.0 - min_angle_multiplier) / (angle_repeat_count + 1.0);

        double slider_bonus = 0.0;

        if (current.base_is_slider) {
            // Invert the scaling factor to determine the true travel distance independent of circle size.
            double pixel_travel_distance = current.lazy_travel_distance / scaling_factor;

            // Reward sliders based on velocity.
            slider_bonus =
                utils::pow(std::max(0.0, pixel_travel_distance / current.travel_time - min_velocity), 0.5);

            // Longer sliders require more memorisation.
            slider_bonus *= pixel_travel_distance;

            // Nerf sliders with repeats, as less memorisation is required.
            if (current.repeat_count > 0) {
                slider_bonus /= (current.repeat_count + 1);
            }
        }

        flashlight_difficulty += slider_bonus * slider_multiplier;

        return flashlight_difficulty;
    }
}}}} // namespace pppp::osu::difficulty::evaluators
