// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/flow_aim_evaluator.h"
#include "pppp/osu/difficulty/evaluators/aim/snap_aim_evaluator.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/vector2.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    namespace {
        double calculate_overlap_factor(const preprocessing::OsuDifficultyHitObject& first,
                                        const preprocessing::OsuDifficultyHitObject& second) {
            double object_radius = object::OsuHitObject::calculate_radius(first.circle_size);

            double distance = length(first.base_stacked_position - second.base_stacked_position);
            return utils::math::clamp(
                1.0 - utils::pow(std::max(distance - object_radius, 0.0) / object_radius, 2), 0.0, 1.0);
        }
    } // namespace

    double FlowAimEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                                    bool with_slider_travel_distance) {
        const preprocessing::OsuDifficultyHitObject* prev =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));

        if (current.base_is_spinner || current.index <= 1 || prev->base_is_spinner) {
            return 0.0;
        }

        const preprocessing::OsuDifficultyHitObject* prev2 =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(1));

        const double velocity_change_multiplier = 0.52;

        double curr_distance =
            with_slider_travel_distance ? current.lazy_jump_distance : current.jump_distance;
        double prev_distance = with_slider_travel_distance ? prev->lazy_jump_distance : prev->jump_distance;

        double curr_velocity = curr_distance / current.adjusted_delta_time;

        if (prev->base_is_slider && with_slider_travel_distance) {
            // If the last object is a slider, then we extend the travel velocity through the slider into
            // the current object.
            double slider_distance = prev->lazy_travel_distance + current.lazy_jump_distance;
            curr_velocity = std::max(curr_velocity, slider_distance / current.adjusted_delta_time);
        }

        double prev_velocity = prev_distance / prev->adjusted_delta_time;

        double flow_difficulty = curr_velocity;

        // Apply high circle size bonus to the base velocity.
        // We use reduced CS bonus here because the bonus was made for an evaluator with a different d/t
        // scaling
        flow_difficulty *= std::sqrt(current.small_circle_bonus);

        // Rhythm changes are harder to flow
        flow_difficulty *=
            1.0 +
            std::min(0.25, utils::pow((std::max(current.adjusted_delta_time, prev->adjusted_delta_time) -
                                       std::min(current.adjusted_delta_time, prev->adjusted_delta_time)) /
                                          50.0,
                                      4));

        if (current.angle.has_value() && prev->angle.has_value()) {
            double angle_difference = std::fabs(current.angle.value() - prev->angle.value());
            double angle_difference_adjusted = std::sin(angle_difference / 2.0) * 180.0;
            double angular_velocity = angle_difference_adjusted / (current.adjusted_delta_time * 0.1);

            // Low angular velocity flow (angles are consistent) is easier to follow than erratic flow
            flow_difficulty *= 0.8 + std::sqrt(angular_velocity / 270.0);
        }

        // If all three notes are overlapping - don't reward bonuses as you don't have to do additional
        // movement
        double overlapped_notes_weight = 1.0;

        if (current.index > 2) {
            double o1 = calculate_overlap_factor(current, *prev);
            double o2 = calculate_overlap_factor(current, *prev2);
            double o3 = calculate_overlap_factor(*prev, *prev2);

            overlapped_notes_weight = 1.0 - o1 * o2 * o3;
        }

        if (current.angle.has_value()) {
            // Acute angles are also hard to flow
            flow_difficulty += curr_velocity * SnapAimEvaluator::calc_angle_acuteness(current.angle.value()) *
                               overlapped_notes_weight;
        }

        if (std::max(prev_velocity, curr_velocity) != 0) {
            if (with_slider_travel_distance) {
                curr_velocity = curr_distance / current.adjusted_delta_time;
            }

            // Scale with ratio of difference compared to 0.5 * max dist.
            double dist_ratio = utils::smoothstep(
                std::fabs(prev_velocity - curr_velocity) / std::max(prev_velocity, curr_velocity), 0, 1);

            // Reward for % distance up to 125 / strainTime for overlaps where velocity is still changing.
            double overlap_velocity_buff =
                std::min(preprocessing::NORMALISED_DIAMETER * 1.25 /
                             std::min(current.adjusted_delta_time, prev->adjusted_delta_time),
                         std::fabs(prev_velocity - curr_velocity));

            flow_difficulty +=
                overlap_velocity_buff * dist_ratio * overlapped_notes_weight * velocity_change_multiplier;
        }

        if (current.base_is_slider && with_slider_travel_distance) {
            // Include slider velocity to make velocity more consistent with snap
            flow_difficulty += current.travel_distance / current.travel_time;
        }

        // Final velocity is being raised to a power because flow difficulty scales harder with both high
        // distance and time, and we want to account for that
        flow_difficulty = utils::pow(flow_difficulty, 1.45);

        // Reduce difficulty for low spacing since spacing below radius is always to be flowed
        return flow_difficulty * utils::smootherstep(curr_distance, 0, preprocessing::NORMALISED_RADIUS);
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
