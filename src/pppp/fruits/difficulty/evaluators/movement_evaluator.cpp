// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/difficulty/evaluators/movement_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace fruits { namespace difficulty { namespace evaluators {
    double MovementEvaluator::evaluate_difficulty_of(const preprocessing::CatchDifficultyHitObject& current) {
        const preprocessing::CatchDifficultyHitObject* last =
            static_cast<const preprocessing::CatchDifficultyHitObject*>(current.previous(0));
        const preprocessing::CatchDifficultyHitObject* last_last =
            static_cast<const preprocessing::CatchDifficultyHitObject*>(current.previous(1));

        // In catch, clockrate adjustments do not only affect the timings of hitobjects,
        // but also the speed of the player's catcher, which has an impact on difficulty
        double catcher_speed_multiplier = current.clock_rate;

        double weighted_strain_time = current.strain_time + 13 + (3 / catcher_speed_multiplier);

        double distance_addition = utils::pow(std::fabs(current.distance_moved), 1.3) / 510;
        double sqrt_strain = std::sqrt(weighted_strain_time);

        double edge_dash_bonus = 0;

        // Direction change bonus.
        if (std::fabs(current.distance_moved) > 0.1) {
            if (current.index >= 1 && std::fabs(last->distance_moved) > 0.1 &&
                utils::math::sign(current.distance_moved) != utils::math::sign(last->distance_moved)) {
                double bonus_factor = std::min(50.0f, std::fabs(current.distance_moved)) / 50.0f;
                double antiflow_factor = std::max(
                    static_cast<double>(std::min(70.0f, std::fabs(last->distance_moved)) / 70.0f), 0.38);

                distance_addition += DIRECTION_CHANGE_BONUS / std::sqrt(last->strain_time + 16) *
                                     bonus_factor * antiflow_factor *
                                     std::max(1 - utils::pow(weighted_strain_time / 1000, 3), 0.0);
            }

            // Base bonus for every movement, giving some weight to streams.
            distance_addition +=
                12.5 *
                std::min(std::fabs(current.distance_moved),
                         static_cast<float>(preprocessing::NORMALIZED_HALF_CATCHER_WIDTH) * 2.0f) /
                (preprocessing::NORMALIZED_HALF_CATCHER_WIDTH * 6) / sqrt_strain;
        }

        // Linear spacing nerf.
        int linear_spacing_count = 0;

        for (int i = 0; i < std::min(current.index, 10); i++) {
            const preprocessing::CatchDifficultyHitObject* catch_prev_obj =
                static_cast<const preprocessing::CatchDifficultyHitObject*>(current.previous(i));

            // Only same direction movements matter as they do not take any additional inputs.
            if (utils::math::sign(current.distance_moved) !=
                    utils::math::sign(catch_prev_obj->distance_moved) ||
                current.distance_moved == 0 || catch_prev_obj->distance_moved == 0) {
                break;
            }

            double current_spacing = std::fabs(current.distance_moved / current.strain_time);
            double previous_spacing = std::fabs(catch_prev_obj->distance_moved / catch_prev_obj->strain_time);

            double relative_difference = std::fabs(current_spacing / previous_spacing - 1);

            if (relative_difference > 0.05) {
                break;
            }

            linear_spacing_count++;
        }

        distance_addition *= utils::pow(0.7, linear_spacing_count);

        // Bonus for edge dashes.
        if (current.last_distance_to_hyper_dash <= 20.0f) {
            if (!current.last_hyper_dash) {
                edge_dash_bonus += 5.7;
            }

            distance_addition *=
                1.0 + edge_dash_bonus * ((20.0f - current.last_distance_to_hyper_dash) / 20.0f) *
                          utils::pow(std::min(current.strain_time * catcher_speed_multiplier, 265.0) / 265,
                                     1.5); // Edge Dashes are easier at lower ms values
        }

        // There is an edge case where horizontal back and forth sliders create "buzz" patterns which are
        // repeated "movements" with a distance lower than the platter's width but high enough to be
        // considered a movement due to the absolute_player_positioning_error and
        // preprocessing::NORMALIZED_HALF_CATCHER_WIDTH offsets We are detecting this exact scenario. The
        // first back and forth is counted but all subsequent ones are nullified. To achieve that, we need to
        // store the exact distances (distance ignoring absolute_player_positioning_error and
        // preprocessing::NORMALIZED_HALF_CATCHER_WIDTH)
        if (current.index >= 2 &&
            std::fabs(current.exact_distance_moved) <= preprocessing::NORMALIZED_HALF_CATCHER_WIDTH * 2 &&
            current.exact_distance_moved == -last->exact_distance_moved &&
            last->exact_distance_moved == -last_last->exact_distance_moved &&
            current.strain_time == last->strain_time && last->strain_time == last_last->strain_time) {
            distance_addition = 0;
        }

        return distance_addition / weighted_strain_time;
    }
}}}} // namespace pppp::fruits::difficulty::evaluators
