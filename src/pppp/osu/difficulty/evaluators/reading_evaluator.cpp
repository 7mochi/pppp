// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/reading_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators {
    namespace {
        double high_bpm_bonus(double ms) { return 1.0 / (1.0 - utils::pow(0.8, ms / 1000.0)); }

        // Returns a nerfing factor for when objects are very distant in time, affecting reading less.
        double get_time_nerf_factor(double delta_time) {
            return utils::math::clamp(2.0 - delta_time / (READING_WINDOW_SIZE / 2.0), 0.0, 1.0);
        }

        // Returns the density of objects visible at the point in time the current object needs to be clicked
        // capped by the reading window.
        double retrieve_current_visible_object_density(const preprocessing::OsuDifficultyHitObject& current) {
            double visible_object_count = 0.0;

            const preprocessing::OsuDifficultyHitObject* hit_object =
                static_cast<const preprocessing::OsuDifficultyHitObject*>(current.next(0));
            while (hit_object) {
                // Object not visible at the time current object needs to be clicked.
                if (hit_object->start_time - current.start_time > READING_WINDOW_SIZE ||
                    current.start_time < hit_object->start_time - hit_object->preempt) {
                    break;
                }

                double time_between_curr_and_loop_obj = hit_object->start_time - current.start_time;
                double time_nerf_factor = get_time_nerf_factor(time_between_curr_and_loop_obj);

                visible_object_count +=
                    opacity_at(*hit_object, current.base_object_start_time, false) * time_nerf_factor;

                hit_object = static_cast<const preprocessing::OsuDifficultyHitObject*>(hit_object->next(0));
            }

            return visible_object_count;
        }

        double get_past_object_difficulty_influence(const preprocessing::OsuDifficultyHitObject& curr_obj) {
            double past_object_difficulty_influence = 0.0;

            for (int i = 0; i < curr_obj.index; i++) {
                const preprocessing::OsuDifficultyHitObject* loop_obj =
                    static_cast<const preprocessing::OsuDifficultyHitObject*>(curr_obj.previous(i));
                // Current object not visible at the time object needs to be clicked
                if (!loop_obj || curr_obj.start_time - loop_obj->start_time > READING_WINDOW_SIZE ||
                    loop_obj->start_time < curr_obj.start_time - curr_obj.preempt) {
                    break;
                }

                // When aiming an object small distances mean previous objects may be cheesed, so it doesn't
                // matter whether they were arranged confusingly.
                double loop_difficulty = opacity_at(curr_obj, loop_obj->base_object_start_time, false);

                loop_difficulty *=
                    utils::smootherstep(loop_obj->lazy_jump_distance, 15, DISTANCE_INFLUENCE_THRESHOLD);

                // Account less for objects close to the max reading window
                double time_between_curr_and_loop_obj = curr_obj.start_time - loop_obj->start_time;
                double time_nerf_factor = get_time_nerf_factor(time_between_curr_and_loop_obj);

                loop_difficulty *= time_nerf_factor;
                past_object_difficulty_influence += loop_difficulty;
            }

            return past_object_difficulty_influence;
        }

        // Returns a factor of how often the current object's angle has been repeated in a certain time
        // frame. It does this by checking the difference in angle between current and past objects and sums
        // them based on a range of similarity. https://www.desmos.com/calculator/eb057a4822
        double get_constant_angle_nerf_factor(const preprocessing::OsuDifficultyHitObject& current) {
            const double minimum_angle_relevancy_time = 2000; // 2 seconds
            const double maximum_angle_relevancy_time = 200;

            double constant_angle_count = 0.0;
            int index = 0;
            double current_time_gap = 0.0;

            const preprocessing::OsuDifficultyHitObject* loop_obj_prev0 = &current;
            const preprocessing::OsuDifficultyHitObject* loop_obj_prev1 = 0;
            const preprocessing::OsuDifficultyHitObject* loop_obj_prev2 = 0;

            while (current_time_gap < minimum_angle_relevancy_time) {
                const preprocessing::OsuDifficultyHitObject* loop_obj =
                    static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(index));
                if (!loop_obj) {
                    break;
                }

                // Account less for objects that are close to the time limit.
                double long_interval_factor =
                    1.0 - utils::reverse_lerp(loop_obj->adjusted_delta_time, maximum_angle_relevancy_time,
                                              minimum_angle_relevancy_time);

                if (loop_obj->angle.has_value() && current.angle.has_value()) {
                    double angle_difference = std::fabs(current.angle.value() - loop_obj->angle.value());
                    double angle_difference_alternating = utils::PI_D;

                    if (loop_obj_prev1 && loop_obj_prev2) {
                        if (loop_obj_prev0->angle.has_value() && loop_obj_prev1->angle.has_value() &&
                            loop_obj_prev2->angle.has_value()) {
                            angle_difference_alternating =
                                std::fabs(loop_obj_prev1->angle.value() - loop_obj->angle.value());
                            angle_difference_alternating +=
                                std::fabs(loop_obj_prev2->angle.value() - loop_obj_prev0->angle.value());

                            double weight = 1.0;

                            // Be sure that one of the angles is very sharp, when other is wide
                            weight *= utils::reverse_lerp(
                                std::min(loop_obj->angle.value(), loop_obj_prev0->angle.value()) * 180.0 /
                                    utils::PI_D,
                                20, 5);
                            weight *= utils::reverse_lerp(
                                std::max(loop_obj->angle.value(), loop_obj_prev0->angle.value()) * 180.0 /
                                    utils::PI_D,
                                60, 120);

                            // Lerp between max angle difference and rescaled alternating difference, with
                            // more harsh scaling compared to normal difference
                            angle_difference_alternating = utils::math::lerp_dotnet(
                                utils::PI_D, 0.1 * angle_difference_alternating, weight);
                        }
                    }

                    double stack_factor = utils::smootherstep(loop_obj->lazy_jump_distance, 0,
                                                              preprocessing::NORMALISED_RADIUS);

                    constant_angle_count +=
                        std::cos(3.0 * std::min(utils::math::degrees_to_radians(30),
                                                std::min(angle_difference, angle_difference_alternating) *
                                                    stack_factor)) *
                        long_interval_factor;
                }

                current_time_gap = current.start_time - loop_obj->start_time;
                index++;

                loop_obj_prev2 = loop_obj_prev1;
                loop_obj_prev1 = loop_obj_prev0;
                loop_obj_prev0 = loop_obj;
            }

            return utils::math::clamp(2.0 / constant_angle_count, 0.2, 1.0);
        }

        /// Calculates the density difficulty of the current object and how hard it is to aim it because of it
        /// based on:
        /// - cursor velocity to the current object
        /// - how many times the current object's angle was repeated
        /// - density of objects visible when the current object appears
        /// - density of objects visible when the current object needs to be clicked
        double calculate_density_difficulty(const preprocessing::OsuDifficultyHitObject* next_obj,
                                            double velocity, double constant_angle_nerf_factor,
                                            double past_object_difficulty_influence,
                                            double current_visible_object_density) {
            const double density_multiplier = 2.4;
            const double density_difficulty_base = 2.5;

            // Consider future densities too because it can make the path the cursor takes less clear
            double future_object_difficulty_influence = std::sqrt(current_visible_object_density);

            if (next_obj) {
                // Reduce difficulty if movement to next object is small
                future_object_difficulty_influence *=
                    utils::smootherstep(next_obj->lazy_jump_distance, 15, DISTANCE_INFLUENCE_THRESHOLD);
            }

            // Value higher note densities exponentially
            double note_density_difficulty =
                utils::pow(past_object_difficulty_influence + future_object_difficulty_influence, 1.7) * 0.4 *
                constant_angle_nerf_factor * velocity;

            // Award only denser than average maps.
            note_density_difficulty = std::max(0.0, note_density_difficulty - density_difficulty_base);

            // Apply a soft cap to general density reading to account for partial memorization
            note_density_difficulty = utils::pow(note_density_difficulty, 0.45) * density_multiplier;

            return note_density_difficulty;
        }

        /// Calculates the difficulty of aiming the current object when the approach rate is very high based
        /// on:
        /// - cursor velocity to the current object
        /// - how many times the current object's angle was repeated
        /// - how many milliseconds elapse between the approach circle appearing and touching the inner circle
        double calculate_preempt_difficulty(double velocity, double constant_angle_nerf_factor,
                                            double preempt) {
            const double preempt_balancing_factor = 140000;
            const double preempt_starting_point = 500; // AR 9.66 in milliseconds

            // Arbitrary curve for the base value preempt difficulty should have as approach rate increases.
            // https://www.desmos.com/calculator/c175335a71
            double preempt_difficulty =
                utils::pow((preempt_starting_point - preempt + std::fabs(preempt - preempt_starting_point)) /
                               2.0,
                           2.5) /
                preempt_balancing_factor;

            preempt_difficulty *= constant_angle_nerf_factor * velocity;

            return preempt_difficulty;
        }

        /// Calculates the difficulty of aiming the current object when the hidden mod is active based on:
        /// - cursor velocity to the current object
        /// - time the current object spends invisible
        /// - density of objects visible when the current object appears
        /// - density of objects visible when the current object needs to be clicked
        /// - how many times the current object's angle was repeated
        /// - if the current object is perfectly stacked to the previous one
        double calculate_hidden_difficulty(const preprocessing::OsuDifficultyHitObject& curr_obj,
                                           double past_object_difficulty_influence,
                                           double current_visible_object_density, double velocity,
                                           double constant_angle_nerf_factor) {
            const double hidden_multiplier = 0.28;

            // Higher preempt means that time spent invisible is higher too, we want to reward that
            double preempt_factor = utils::pow(curr_obj.preempt, 2.2) * 0.01;

            // Account for both past and current densities
            double density_factor =
                utils::pow(current_visible_object_density + past_object_difficulty_influence, 3.3) * 3;

            // Apply a soft cap to general HD reading to account for partial memorization
            double hidden_difficulty =
                (preempt_factor + density_factor) * constant_angle_nerf_factor * velocity * 0.01;

            hidden_difficulty = utils::pow(hidden_difficulty, 0.4) * hidden_multiplier;

            const preprocessing::OsuDifficultyHitObject* previous_obj =
                static_cast<const preprocessing::OsuDifficultyHitObject*>(curr_obj.previous(0));

            // Buff perfect stacks only if current note is completely invisible at the time you click the
            // previous note.
            if (curr_obj.lazy_jump_distance == 0 &&
                opacity_at(curr_obj, previous_obj->base_object_start_time, true) == 0 &&
                previous_obj->start_time > curr_obj.start_time - curr_obj.preempt) {
                hidden_difficulty += hidden_multiplier * 2500.0 /
                                     utils::pow(curr_obj.adjusted_delta_time,
                                                1.5); // Perfect stacks are harder the less time between notes
            }

            return hidden_difficulty;
        }
    } // namespace

    double ReadingEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                                    bool hidden) {
        if (current.base_is_spinner || current.index == 0) {
            return 0.0;
        }

        const preprocessing::OsuDifficultyHitObject* next_obj =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.next(0));

        double velocity = std::max(1.0, current.lazy_jump_distance /
                                            current.adjusted_delta_time); // Only allow velocity to buff

        double current_visible_object_density = retrieve_current_visible_object_density(current);
        double past_object_difficulty_influence = get_past_object_difficulty_influence(current);

        double constant_angle_nerf_factor = get_constant_angle_nerf_factor(current);

        double note_density_difficulty =
            calculate_density_difficulty(next_obj, velocity, constant_angle_nerf_factor,
                                         past_object_difficulty_influence, current_visible_object_density);

        double hidden_difficulty =
            hidden ? calculate_hidden_difficulty(current, past_object_difficulty_influence,
                                                 current_visible_object_density, velocity,
                                                 constant_angle_nerf_factor)
                   : 0.0;

        double preempt_difficulty =
            calculate_preempt_difficulty(velocity, constant_angle_nerf_factor, current.preempt);

        double reading_difficulty =
            utils::norm(1.5, preempt_difficulty, hidden_difficulty, note_density_difficulty);

        // Having less time to process information is harder
        reading_difficulty *= high_bpm_bonus(current.adjusted_delta_time);

        return reading_difficulty;
    }
}}}} // namespace pppp::osu::difficulty::evaluators
