// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/snap_aim_evaluator.h"
#include "pppp/osu/difficulty/utils/angle_utils.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/vector2.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    namespace {
        double high_bpm_bonus(double ms) {
            return 1.0 / (1.0 - pppp::utils::pow(0.03, pppp::utils::pow(ms / 1000.0, 0.65)));
        }

        double calculate_current_velocity(const preprocessing::OsuDifficultyHitObject& osu_curr_obj,
                                          const preprocessing::OsuDifficultyHitObject& osu_last_obj,
                                          double curr_distance, bool with_slider_travel_distance) {
            double curr_velocity = curr_distance / osu_curr_obj.adjusted_delta_time;

            // If the last object is a slider, then we extend the travel velocity through the slider into
            // the current object.
            if (osu_last_obj.base_is_slider && with_slider_travel_distance) {
                double slider_distance = osu_last_obj.lazy_travel_distance + osu_curr_obj.lazy_jump_distance;
                curr_velocity = std::max(curr_velocity, slider_distance / osu_curr_obj.adjusted_delta_time);
            }

            return curr_velocity;
        }

        double vector_angle_repetition(const preprocessing::OsuDifficultyHitObject& current,
                                       const preprocessing::OsuDifficultyHitObject& previous) {
            if (!current.angle.has_value() || !previous.angle.has_value() || previous.base_is_spinner ||
                static_cast<const preprocessing::OsuDifficultyHitObject*>(previous.previous(0))
                    ->base_is_spinner) {
                return 1.0;
            }

            const double note_limit = 6;
            const double maximum_repetition_nerf = 0.15;
            const double maximum_vector_influence = 0.5;

            double constant_angle_count = 0.0;

            for (int idx = 0; idx < static_cast<int>(note_limit); idx++) {
                const preprocessing::OsuDifficultyHitObject* prev_obj =
                    static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(idx));

                if (!prev_obj || prev_obj->base_is_spinner) {
                    break;
                }

                // Only consider vectors in the same jump section, stopping to change rhythm ruins momentum
                if (std::max(current.adjusted_delta_time, prev_obj->adjusted_delta_time) >
                    1.1 * std::min(current.adjusted_delta_time, prev_obj->adjusted_delta_time)) {
                    break;
                }

                if (!prev_obj->normalised_vector_angle.has_value() ||
                    !current.normalised_vector_angle.has_value()) {
                    continue;
                }

                double angle_difference = std::fabs(current.normalised_vector_angle.value() -
                                                    prev_obj->normalised_vector_angle.value());

                // Refer to this desmos for tuning, constants need to be precise so that values stay within
                // the range of 0 and 1. https://www.desmos.com/calculator/a8jesv5sv2
                constant_angle_count +=
                    std::cos(8 * std::min(pppp::utils::math::degrees_to_radians(11.25), angle_difference));
            }

            double vector_repetition = pppp::utils::pow(std::min(0.5 / constant_angle_count, 1.0), 2);

            double stack_factor =
                pppp::utils::smootherstep(current.lazy_jump_distance, 0, preprocessing::NORMALISED_DIAMETER);

            double curr_angle = current.angle.value();
            double last_angle = previous.angle.value();

            double angle_difference_adjusted =
                std::cos(2 * std::min(pppp::utils::math::degrees_to_radians(45),
                                      std::fabs(curr_angle - last_angle) * stack_factor));

            double base_nerf = 1.0 - maximum_repetition_nerf *
                                         utils::AngleUtils::calculate_acuteness(last_angle) *
                                         angle_difference_adjusted;

            return pppp::utils::pow(base_nerf + (1.0 - base_nerf) * vector_repetition *
                                                    maximum_vector_influence * stack_factor,
                                    2);
        }

        double calculate_wide_angle_bonus(const preprocessing::OsuDifficultyHitObject& osu_curr_obj,
                                          const preprocessing::OsuDifficultyHitObject& osu_last_obj,
                                          double curr_distance, double prev_distance,
                                          bool with_slider_travel_distance) {
            const double wide_angle_multiplier = 2.15;

            if (!osu_curr_obj.angle.has_value() || !osu_last_obj.angle.has_value()) {
                return 0.0;
            }

            double wide_angle_bonus = utils::AngleUtils::calculate_wideness(osu_curr_obj.angle.value());

            // Penalize angle repetition. It is important to do it _before_ multiplying by velocity because
            // we compare raw wideness here
            wide_angle_bonus *=
                0.25 + 0.75 * (1.0 - std::min(wide_angle_bonus,
                                              pppp::utils::pow(utils::AngleUtils::calculate_wideness(
                                                                   osu_last_obj.angle.value()),
                                                               3)));

            // Rescaling velocity for the wide angle bonus
            const double wide_angle_time_scale = 1.2;

            double curr_rescaled_velocity =
                curr_distance / pppp::utils::pow(osu_curr_obj.adjusted_delta_time, wide_angle_time_scale);
            double prev_rescaled_velocity =
                prev_distance / pppp::utils::pow(osu_last_obj.adjusted_delta_time, wide_angle_time_scale);

            if (osu_last_obj.base_is_slider && with_slider_travel_distance) {
                double slider_distance = osu_last_obj.lazy_travel_distance + osu_curr_obj.lazy_jump_distance;
                curr_rescaled_velocity =
                    std::max(curr_rescaled_velocity,
                             slider_distance /
                                 pppp::utils::pow(osu_curr_obj.adjusted_delta_time, wide_angle_time_scale));
            }

            wide_angle_bonus *= std::min(curr_rescaled_velocity, prev_rescaled_velocity);

            const preprocessing::OsuDifficultyHitObject* osu_last2_obj =
                static_cast<const preprocessing::OsuDifficultyHitObject*>(osu_curr_obj.previous(2));

            if (osu_last2_obj) {
                // If objects just go back and forth through a middle point - don't give as much wide
                // bonus
                // Use previous(2) and previous(0) because angles calculation is done prevprev-prev-curr,
                // so any object's angle's center point is always the previous object
                float distance =
                    length(osu_last_obj.base_stacked_position - osu_last2_obj->base_stacked_position);

                if (distance < 1) {
                    wide_angle_bonus *= 1.0 - 0.55 * (1.0 - distance);
                }
            }

            return wide_angle_bonus * wide_angle_multiplier;
        }

        double calculate_velocity_change_bonus(bool with_slider_travel_distance, double prev_velocity,
                                               double curr_velocity, double curr_distance,
                                               const preprocessing::OsuDifficultyHitObject& osu_curr_obj,
                                               const preprocessing::OsuDifficultyHitObject& osu_last_obj) {
            const double velocity_change_multiplier = 0.85;

            if (std::max(prev_velocity, curr_velocity) == 0) {
                return 0.0;
            }

            if (with_slider_travel_distance) {
                // We want to use just the object jump without slider velocity when awarding differences
                curr_velocity = curr_distance / osu_curr_obj.adjusted_delta_time;
            }

            // cap velocity to at least 1 radius distance to only award snap patterns
            curr_velocity = std::max(curr_velocity,
                                     preprocessing::NORMALISED_DIAMETER / osu_curr_obj.adjusted_delta_time);
            prev_velocity = std::max(prev_velocity,
                                     preprocessing::NORMALISED_DIAMETER / osu_last_obj.adjusted_delta_time);

            // Scale with ratio of difference compared to 0.5 * max dist.
            double dist_ratio = pppp::utils::smoothstep(
                std::fabs(prev_velocity - curr_velocity) / std::max(prev_velocity, curr_velocity), 0, 1);

            // Reward for % distance up to 125 / strainTime for overlaps where velocity is still changing.
            double overlap_velocity_buff =
                std::min(preprocessing::NORMALISED_DIAMETER * 1.25 /
                             std::min(osu_curr_obj.adjusted_delta_time, osu_last_obj.adjusted_delta_time),
                         std::fabs(prev_velocity - curr_velocity));

            double velocity_change_bonus = overlap_velocity_buff * dist_ratio;

            // Penalize for rhythm changes.
            velocity_change_bonus *= pppp::utils::pow(
                std::min(osu_curr_obj.adjusted_delta_time, osu_last_obj.adjusted_delta_time) /
                    std::max(osu_curr_obj.adjusted_delta_time, osu_last_obj.adjusted_delta_time),
                3);

            return velocity_change_bonus * velocity_change_multiplier;
        }

        double calculate_slider_bonus(const preprocessing::OsuDifficultyHitObject& osu_curr_obj) {
            const double slider_multiplier = 1.35;

            // Reward sliders based on velocity.
            double slider_bonus = osu_curr_obj.travel_distance / osu_curr_obj.travel_time;
            double rescaled_slider_bonus =
                slider_bonus < 1 ? slider_bonus : pppp::utils::pow(slider_bonus, 0.75);

            return rescaled_slider_bonus * slider_multiplier;
        }
    } // namespace

    double SnapAimEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                                    bool with_slider_travel_distance) {
        if (current.index <= 1 || current.base_is_spinner) {
            return 0.0;
        }

        const preprocessing::OsuDifficultyHitObject* osu_last_obj =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));

        double curr_distance =
            with_slider_travel_distance ? current.lazy_jump_distance : current.jump_distance;
        double curr_velocity =
            calculate_current_velocity(current, *osu_last_obj, curr_distance, with_slider_travel_distance);

        double prev_distance =
            with_slider_travel_distance ? osu_last_obj->lazy_jump_distance : osu_last_obj->jump_distance;
        double prev_velocity = prev_distance / osu_last_obj->adjusted_delta_time;

        double snap_difficulty = curr_velocity; // Start difficulty with regular velocity.

        // Penalize angle repetition.
        snap_difficulty *= vector_angle_repetition(current, *osu_last_obj);
        snap_difficulty += calculate_wide_angle_bonus(current, *osu_last_obj, curr_distance, prev_distance,
                                                      with_slider_travel_distance);
        snap_difficulty += calculate_velocity_change_bonus(
            with_slider_travel_distance, prev_velocity, curr_velocity, curr_distance, current, *osu_last_obj);

        if (current.base_is_slider && with_slider_travel_distance) {
            snap_difficulty += calculate_slider_bonus(current);
        }

        // Apply high circle size bonus
        snap_difficulty *= current.small_circle_bonus;

        snap_difficulty *= high_bpm_bonus(current.adjusted_delta_time);

        return snap_difficulty;
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
