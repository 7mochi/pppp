// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/snap_aim_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/vector2.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    namespace {
        double high_bpm_bonus(double ms) {
            return 1.0 / (1.0 - utils::pow(0.03, utils::pow(ms / 1000.0, 0.65)));
        }

        double calc_angle_wideness(double angle) {
            return utils::smoothstep(angle, utils::math::degrees_to_radians(40),
                                     utils::math::degrees_to_radians(140));
        }

        double vector_angle_repetition(const preprocessing::OsuDifficultyHitObject& current) {
            if (!current.angle.has_value() || current.index < 1) {
                return 1.0;
            }

            const preprocessing::OsuDifficultyHitObject* previous =
                static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));

            if (!previous->angle.has_value()) {
                return 1.0;
            }

            const double note_limit = 6;
            const double maximum_repetition_nerf = 0.15;
            const double maximum_vector_influence = 0.5;

            double constant_angle_count = 0.0;

            for (int idx = 0; idx < static_cast<int>(note_limit); idx++) {
                const preprocessing::OsuDifficultyHitObject* prev_obj =
                    static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(idx));
                if (!prev_obj) {
                    break;
                }

                // Only consider vectors in the same jump section, stopping to change rhythm ruins
                // momentum
                if (std::max(current.adjusted_delta_time, prev_obj->adjusted_delta_time) >
                    1.1 * std::min(current.adjusted_delta_time, prev_obj->adjusted_delta_time)) {
                    break;
                }

                if (prev_obj->normalised_vector_angle.has_value() &&
                    current.normalised_vector_angle.has_value()) {
                    double angle_difference = std::fabs(current.normalised_vector_angle.value() -
                                                        prev_obj->normalised_vector_angle.value());
                    // Refer to this desmos for tuning, constants need to be precise so that values stay
                    // within the range of 0 and 1. https://www.desmos.com/calculator/a8jesv5sv2
                    constant_angle_count +=
                        std::cos(8 * std::min(utils::math::degrees_to_radians(11.25), angle_difference));
                }
            }

            double vector_repetition = utils::pow(std::min(0.5 / constant_angle_count, 1.0), 2);

            double stack_factor =
                utils::smootherstep(current.lazy_jump_distance, 0, preprocessing::NORMALISED_DIAMETER);

            double curr_angle = current.angle.value();
            double last_angle = previous->angle.value();

            double angle_difference_adjusted =
                std::cos(2 * std::min(utils::math::degrees_to_radians(45),
                                      std::fabs(curr_angle - last_angle) * stack_factor));

            double base_nerf = 1.0 - maximum_repetition_nerf *
                                         SnapAimEvaluator::calc_angle_acuteness(last_angle) *
                                         angle_difference_adjusted;

            return utils::pow(base_nerf + (1.0 - base_nerf) * vector_repetition * maximum_vector_influence *
                                              stack_factor,
                              2);
        }
    } // namespace

    double SnapAimEvaluator::calc_angle_acuteness(double angle) {
        return utils::smoothstep(angle, utils::math::degrees_to_radians(140),
                                 utils::math::degrees_to_radians(40));
    }

    double SnapAimEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                                    bool with_slider_travel_distance) {
        const preprocessing::OsuDifficultyHitObject* prev =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));
        const preprocessing::OsuDifficultyHitObject* prev2 =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(2));

        if (current.base_is_spinner || current.index <= 1 || prev->base_is_spinner) {
            return 0.0;
        }

        const double wide_angle_multiplier = 9.67;
        const double acute_angle_multiplier = 2.41;
        const double slider_multiplier = 1.5;
        const double velocity_change_multiplier = 0.9;

        // WARNING: Increasing this multiplier beyond 1.02 reduces difficulty as distance increases. Refer
        // to the desmos link above the wiggle bonus calculation
        const double wiggle_multiplier = 1.02;

        const int radius = static_cast<int>(preprocessing::NORMALISED_RADIUS);
        const int diameter = static_cast<int>(preprocessing::NORMALISED_DIAMETER);

        // Calculate the velocity to the current hitobject, which starts with a base distance / time
        // assuming the last object is a hitcircle.
        double curr_distance =
            with_slider_travel_distance ? current.lazy_jump_distance : current.jump_distance;
        double curr_velocity = curr_distance / current.adjusted_delta_time;

        // But if the last object is a slider, then we extend the travel velocity through the slider into
        // the current object.
        if (prev->base_is_slider && with_slider_travel_distance) {
            double slider_distance = prev->lazy_travel_distance + current.lazy_jump_distance;
            curr_velocity = std::max(curr_velocity, slider_distance / current.adjusted_delta_time);
        }

        double prev_distance = with_slider_travel_distance ? prev->lazy_jump_distance : prev->jump_distance;
        double prev_velocity = prev_distance / prev->adjusted_delta_time;

        double snap_difficulty = curr_velocity; // Start difficulty with regular velocity.

        // Penalize angle repetition.
        snap_difficulty *= vector_angle_repetition(current);

        if (current.angle.has_value() && prev->angle.has_value()) {
            double curr_angle = current.angle.value();
            double last_angle = prev->angle.value();

            // Rewarding angles, take the smaller velocity as base.
            double velocity_influence = std::min(curr_velocity, prev_velocity);

            double acute_angle_bonus = 0.0;

            // If rhythms are the same.
            if (std::max(current.adjusted_delta_time, prev->adjusted_delta_time) <
                1.25 * std::min(current.adjusted_delta_time, prev->adjusted_delta_time)) {
                acute_angle_bonus = calc_angle_acuteness(curr_angle);

                // Penalize angle repetition. It is important to do it _before_ multiplying by anything
                // because we compare raw acuteness here
                acute_angle_bonus *=
                    0.08 + 0.92 * (1.0 - std::min(acute_angle_bonus,
                                                  utils::pow(calc_angle_acuteness(last_angle), 3)));

                // Apply acute angle bonus for BPM above 300 1/2 and distance more than one diameter
                acute_angle_bonus *=
                    velocity_influence *
                    utils::smootherstep(utils::milliseconds_to_bpm(current.adjusted_delta_time, 2), 300,
                                        400) *
                    utils::smootherstep(curr_distance, 0, diameter * 2);
            }

            double wide_angle_bonus = calc_angle_wideness(curr_angle);

            // Penalize angle repetition. It is important to do it _before_ multiplying by velocity
            // because we compare raw wideness here
            wide_angle_bonus *=
                0.25 +
                0.75 * (1.0 - std::min(wide_angle_bonus, utils::pow(calc_angle_wideness(last_angle), 3)));

            // Rescaling velocity for the wide angle bonus
            const double wide_angle_time_scale = 1.45;
            double wide_angle_curr_velocity =
                curr_distance / utils::pow(current.adjusted_delta_time, wide_angle_time_scale);
            double wide_angle_prev_velocity =
                prev_distance / utils::pow(prev->adjusted_delta_time, wide_angle_time_scale);

            if (prev->base_is_slider && with_slider_travel_distance) {
                double slider_distance = prev->lazy_travel_distance + current.lazy_jump_distance;
                wide_angle_curr_velocity = std::max(
                    wide_angle_curr_velocity,
                    slider_distance / utils::pow(current.adjusted_delta_time, wide_angle_time_scale));
            }

            wide_angle_bonus *= std::min(wide_angle_curr_velocity, wide_angle_prev_velocity);

            if (prev2) {
                // If objects just go back and forth through a middle point - don't give as much wide
                // bonus
                // Use previous(2) and previous(0) because angles calculation is done prevprev-prev-curr,
                // so any object's angle's center point is always the previous object
                float dist = length(prev->base_stacked_position - prev2->base_stacked_position);

                if (dist < 1) {
                    wide_angle_bonus *= 1.0 - 0.55 * (1.0 - dist);
                }
            }

            // Add in acute angle bonus or wide angle bonus, whichever is larger.
            snap_difficulty += std::max(acute_angle_bonus * acute_angle_multiplier,
                                        wide_angle_bonus * wide_angle_multiplier);

            // Apply wiggle bonus for jumps that are [radius, 3*diameter] in distance, with < 110 angle
            // https://www.desmos.com/calculator/dp0v0nvowc
            double wiggle_bonus =
                velocity_influence * utils::smootherstep(curr_distance, radius, diameter) *
                utils::pow(utils::reverse_lerp(curr_distance, diameter * 3, diameter), 1.8) *
                utils::smootherstep(curr_angle, utils::math::degrees_to_radians(110),
                                    utils::math::degrees_to_radians(60)) *
                utils::smootherstep(prev_distance, radius, diameter) *
                utils::pow(utils::reverse_lerp(prev_distance, diameter * 3, diameter), 1.8) *
                utils::smootherstep(last_angle, utils::math::degrees_to_radians(110),
                                    utils::math::degrees_to_radians(60));

            snap_difficulty += wiggle_bonus * wiggle_multiplier;
        }

        if (std::max(prev_velocity, curr_velocity) != 0) {
            if (with_slider_travel_distance) {
                // We want to use just the object jump without slider velocity when awarding differences
                curr_velocity = curr_distance / current.adjusted_delta_time;
            }

            // Scale with ratio of difference compared to 0.5 * max dist.
            double dist_ratio = utils::smoothstep(
                std::fabs(prev_velocity - curr_velocity) / std::max(prev_velocity, curr_velocity), 0, 1);

            // Reward for % distance up to 125 / strainTime for overlaps where velocity is still changing.
            double overlap_velocity_buff =
                std::min(diameter * 1.25 / std::min(current.adjusted_delta_time, prev->adjusted_delta_time),
                         std::fabs(prev_velocity - curr_velocity));

            double velocity_change_bonus = overlap_velocity_buff * dist_ratio;

            // Penalize for rhythm changes.
            velocity_change_bonus *=
                utils::pow(std::min(current.adjusted_delta_time, prev->adjusted_delta_time) /
                               std::max(current.adjusted_delta_time, prev->adjusted_delta_time),
                           2);

            snap_difficulty += velocity_change_bonus * velocity_change_multiplier;
        }

        // Reward sliders based on velocity.
        if (current.base_is_slider && with_slider_travel_distance) {
            double slider_bonus = current.travel_distance / current.travel_time;
            snap_difficulty +=
                (slider_bonus < 1 ? slider_bonus : utils::pow(slider_bonus, 0.75)) * slider_multiplier;
        }

        // Apply high circle size bonus
        snap_difficulty *= current.small_circle_bonus;

        snap_difficulty *= high_bpm_bonus(current.adjusted_delta_time);

        return snap_difficulty;
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
