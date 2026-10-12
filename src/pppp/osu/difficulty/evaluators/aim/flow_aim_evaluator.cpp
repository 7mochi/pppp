// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/flow_aim_evaluator.h"
#include "pppp/osu/difficulty/utils/angle_utils.h"
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
            return pppp::utils::math::clamp(
                1.0 - pppp::utils::pow(std::max(distance - object_radius, 0.0) / object_radius, 2), 0.0, 1.0);
        }

        double calculate_overlap_weight(const preprocessing::OsuDifficultyHitObject& current,
                                        const preprocessing::OsuDifficultyHitObject& previous,
                                        const preprocessing::OsuDifficultyHitObject& last_last) {
            double o1 = calculate_overlap_factor(current, previous);
            double o2 = calculate_overlap_factor(current, last_last);
            double o3 = calculate_overlap_factor(previous, last_last);

            return 1.0 - o1 * o2 * o3;
        }

        double calculate_rhythm_change_bonus(const preprocessing::OsuDifficultyHitObject& current,
                                             const preprocessing::OsuDifficultyHitObject& last) {
            const double maximum_rhythm_change_bonus = 0.1;

            double bonus =
                pppp::utils::pow((std::max(current.adjusted_delta_time, last.adjusted_delta_time) -
                                  std::min(current.adjusted_delta_time, last.adjusted_delta_time)) /
                                     50.0,
                                 4);

            return 1.0 + std::min(maximum_rhythm_change_bonus, bonus);
        }

        /// Scales flow difficulty by angular velocity.
        /// This nerfs consistent angles whilst buffing "erratic" flow.
        double calculate_angular_velocity_bonus(const preprocessing::OsuDifficultyHitObject& current,
                                                const preprocessing::OsuDifficultyHitObject& last) {
            if (!current.angle.has_value() || !last.angle.has_value()) {
                return 1.0;
            }

            double angle_difference = std::fabs(current.angle.value() - last.angle.value());
            double angle_difference_adjusted = std::sin(angle_difference / 2.0) * 180.0;
            double angular_velocity = angle_difference_adjusted / (current.adjusted_delta_time * 0.1);

            return 0.8 + std::sqrt(angular_velocity / 270.0);
        }

        double calculate_acute_angle_bonus(const preprocessing::OsuDifficultyHitObject& current,
                                           const preprocessing::OsuDifficultyHitObject& last,
                                           const preprocessing::OsuDifficultyHitObject& last_last,
                                           const preprocessing::OsuDifficultyHitObject* next,
                                           double curr_velocity) {
            const double acute_angle_multiplier = 0.9;

            if (!current.angle.has_value() || !next || !next->angle.has_value()) {
                return 0.0;
            }

            double curr_acuteness = utils::AngleUtils::calculate_acuteness(current.angle.value());
            double next_acuteness = utils::AngleUtils::calculate_acuteness(next->angle.value());

            double acuteness;
            double overlap_weight;

            // We want to evaluate flow turns at the center point of the actual turn, but curr.Angle is a
            // prev2-prev-curr angle. The issue with changing that to prev-curr-next is that we might evaluate
            // the second note of a flow pattern as snap if prev is acute. With min(curr,next) the evaluation
            // (assuming acute affects snap/flow probability enough) behaves roughly like this:
            //
            //    flow (prev-curr-next and prev2-prev-curr evaluates as wide)
            //     🡓🡓
            //     ooo 🡐 flow (prev2-prev-curr evaluates as wide)
            //      /
            //   ooo 🡐 snap (prev2-prev-curr evaluates as acute)
            //   🡑🡑
            //  flow (prev-curr-next evaluates as wide)
            //
            //
            //  flow (prev-curr-next and prev2-prev-curr evaluates as wide)
            //   🡓🡓
            //   ooo 🡐 flow (prev-curr-next and prev2-prev-curr evaluates as wide)
            //      ╲
            //     ooo 🡐 snap (prev-curr-next evaluates as acute as the center point of the turn)
            //     🡑🡑
            //    flow (prev-curr-next evaluates as wide)
            //
            // In both examples the first object in a flow pattern is evaluated as acute (likely snap) and the
            // rest are wide (likely flow).
            if (curr_acuteness < next_acuteness) {
                acuteness = curr_acuteness;
                overlap_weight = calculate_overlap_weight(current, last, last_last);
            } else {
                acuteness = next_acuteness;
                overlap_weight = calculate_overlap_weight(*next, current, last);
            }

            return curr_velocity * acuteness * overlap_weight * acute_angle_multiplier;
        }

        double calculate_velocity_change_bonus(const preprocessing::OsuDifficultyHitObject& current,
                                               const preprocessing::OsuDifficultyHitObject& previous,
                                               double curr_velocity, double prev_velocity,
                                               double curr_distance, double overlapped_notes_weight,
                                               bool with_slider_travel_distance) {
            const double velocity_change_multiplier = 1.9;

            if (std::max(prev_velocity, curr_velocity) == 0) {
                return 0.0;
            }

            if (with_slider_travel_distance) {
                curr_velocity = curr_distance / current.adjusted_delta_time;
            }

            const double velocity_distance_cap = preprocessing::NORMALISED_DIAMETER * 1.1;

            curr_velocity = std::min(curr_velocity, velocity_distance_cap / current.adjusted_delta_time);
            prev_velocity = std::min(prev_velocity, velocity_distance_cap / previous.adjusted_delta_time);

            // Scale with ratio of difference compared to 0.5 * max dist.
            double dist_ratio = pppp::utils::smoothstep(
                std::fabs(prev_velocity - curr_velocity) / std::max(prev_velocity, curr_velocity), 0, 1);

            double velocity_change_bonus = std::fabs(prev_velocity - curr_velocity) * dist_ratio;

            velocity_change_bonus *=
                pppp::utils::pow(std::min(current.adjusted_delta_time, previous.adjusted_delta_time) /
                                     std::max(current.adjusted_delta_time, previous.adjusted_delta_time),
                                 3);

            return velocity_change_bonus * overlapped_notes_weight * velocity_change_multiplier;
        }

        double calculate_slider_bonus(const preprocessing::OsuDifficultyHitObject& current) {
            return current.travel_distance / current.travel_time;
        }

        double calculate_current_velocity(const preprocessing::OsuDifficultyHitObject& current,
                                          const preprocessing::OsuDifficultyHitObject& previous,
                                          double curr_distance, bool with_slider_travel_distance) {
            double curr_velocity = curr_distance / current.adjusted_delta_time;

            // If the last object is a slider, then we extend the travel velocity through the slider into
            // the current object.
            if (previous.base_is_slider && with_slider_travel_distance) {
                double slider_distance = previous.lazy_travel_distance + current.lazy_jump_distance;
                curr_velocity = std::max(curr_velocity, slider_distance / current.adjusted_delta_time);
            }

            return curr_velocity;
        }
    } // namespace

    double FlowAimEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                                    bool with_slider_travel_distance) {
        if (current.index <= 1 || current.base_is_spinner) {
            return 0.0;
        }

        const preprocessing::OsuDifficultyHitObject* osu_next =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.next(0));
        const preprocessing::OsuDifficultyHitObject* osu_last =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));
        const preprocessing::OsuDifficultyHitObject* osu_last_last =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(1));

        double curr_distance =
            with_slider_travel_distance ? current.lazy_jump_distance : current.jump_distance;
        double prev_distance =
            with_slider_travel_distance ? osu_last->lazy_jump_distance : osu_last->jump_distance;

        double curr_velocity =
            calculate_current_velocity(current, *osu_last, curr_distance, with_slider_travel_distance);

        double prev_velocity = prev_distance / osu_last->adjusted_delta_time;

        double flow_difficulty = curr_velocity;

        // Apply high circle size bonus to the base velocity.
        // We use reduced CS bonus here because the bonus was made for an evaluator with a different d/t
        // scaling
        flow_difficulty *= pppp::utils::pow(current.small_circle_bonus, 0.55);

        flow_difficulty *= calculate_rhythm_change_bonus(current, *osu_last);
        flow_difficulty *= calculate_angular_velocity_bonus(current, *osu_last);

        flow_difficulty +=
            calculate_acute_angle_bonus(current, *osu_last, *osu_last_last, osu_next, curr_velocity);

        flow_difficulty += calculate_velocity_change_bonus(
            current, *osu_last, curr_velocity, prev_velocity, curr_distance,
            calculate_overlap_weight(current, *osu_last, *osu_last_last), with_slider_travel_distance);

        if (current.base_is_slider && with_slider_travel_distance) {
            flow_difficulty += calculate_slider_bonus(current);
        }

        // Final velocity is being raised to a power because flow difficulty scales harder with both high
        // distance and time, and we want to account for that
        flow_difficulty = pppp::utils::pow(flow_difficulty, 1.45);

        // Reduce difficulty for low spacing since spacing below radius is always to be flowed
        return flow_difficulty *
               pppp::utils::smootherstep(curr_distance, 0, preprocessing::NORMALISED_RADIUS);
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
