// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/agility_evaluator.h"
#include "pppp/osu/difficulty/utils/angle_utils.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    namespace {
        double calculate_angle_switching_bonus(const preprocessing::OsuDifficultyHitObject& osu_curr_obj,
                                               const preprocessing::OsuDifficultyHitObject* osu_prev_obj) {
            const double angle_switching_bonus = 0.8;

            if (!osu_curr_obj.angle.has_value() || !osu_prev_obj || !osu_prev_obj->angle.has_value()) {
                return 0.0;
            }

            double acuteness_difference =
                1.0 - std::min(utils::AngleUtils::calculate_acuteness(osu_curr_obj.angle.value()),
                               pppp::utils::pow(
                                   utils::AngleUtils::calculate_acuteness(osu_prev_obj->angle.value()), 3));

            double rhythm_difference_penalty = pppp::utils::pow(
                std::min(osu_curr_obj.adjusted_delta_time, osu_prev_obj->adjusted_delta_time) /
                    std::max(osu_curr_obj.adjusted_delta_time, osu_prev_obj->adjusted_delta_time),
                3);

            return acuteness_difference * rhythm_difference_penalty *
                   pppp::utils::reverse_lerp(osu_prev_obj->lazy_jump_distance,
                                             preprocessing::NORMALISED_RADIUS,
                                             preprocessing::NORMALISED_DIAMETER) *
                   angle_switching_bonus;
        }
    } // namespace

    double AgilityEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current) {
        const double previous_delta_influence = 0.5;

        if (current.base_is_spinner) {
            return 0.0;
        }

        const preprocessing::OsuDifficultyHitObject* osu_prev_obj =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));

        // Assume that the player always uses at least the full great hitwindow for the movement
        double current_delta = std::max(current.adjusted_delta_time, current.hit_window_great);

        // For objects that are stacked we want to reduce the agility difficulty slightly by combining delta
        // times of both objects together
        // Because we can assume that they likely would be done in one movement.
        double previous_delta = 0.0;

        if (osu_prev_obj != 0) {
            previous_delta = osu_prev_obj->adjusted_delta_time *
                             pppp::utils::reverse_lerp(osu_prev_obj->lazy_jump_distance,
                                                       preprocessing::NORMALISED_RADIUS, 0.0);
        }

        double combined_delta = current_delta + previous_delta * previous_delta_influence;

        double angle_switching_bonus = calculate_angle_switching_bonus(current, osu_prev_obj);

        double agility_difficulty =
            (1.0 + angle_switching_bonus) * 10000000.0 / pppp::utils::pow(combined_delta, 3.1);

        agility_difficulty *= current.small_circle_bonus;

        return agility_difficulty;
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
