// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_EVALUATORS_READING_EVALUATOR_H
#define PPPP_TAIKO_DIFFICULTY_EVALUATORS_READING_EVALUATOR_H

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"

namespace pppp { namespace taiko { namespace difficulty { namespace evaluators {
    const double MID_VELOCITY_MIN = 360.0;
    const double MID_VELOCITY_MAX = 480.0;
    const double HIGH_VELOCITY_MIN = 480.0;
    const double HIGH_VELOCITY_MAX = 640.0;

    struct ReadingEvaluator {
        /// Calculates the influence of higher slider velocities on hitobject difficulty. The bonus is
        /// determined based on the effective BPM, shifting within a defined range between the upper
        /// and lower boundaries to reflect how increased slider velocity impacts difficulty.
        /// @param object The hit object to evaluate.
        /// @returns The reading difficulty value for the given hit object.
        static double evaluate_difficulty_of(const preprocessing::TaikoDifficultyHitObject& object);
    };
}}}} // namespace pppp::taiko::difficulty::evaluators

#endif
