// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_EVALUATORS_FLASHLIGHT_EVALUATOR_H
#define PPPP_OSU_DIFFICULTY_EVALUATORS_FLASHLIGHT_EVALUATOR_H

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators {
    struct FlashlightEvaluator {
        /// Evaluates the difficulty of memorising and hitting an object, based on:
        /// - distance between a number of previous objects and the current object,
        /// - the visual opacity of the current object,
        /// - the angle made by the current object,
        /// - length and speed of the current object (for sliders),
        /// - and whether the hidden mod is enabled.
        static double evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                             bool has_hidden, double cs, bool hidden_bonus_applies);
    };
}}}} // namespace pppp::osu::difficulty::evaluators

#endif
