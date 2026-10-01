// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_EVALUATORS_AIM_SNAP_AIM_EVALUATOR_H
#define PPPP_OSU_DIFFICULTY_EVALUATORS_AIM_SNAP_AIM_EVALUATOR_H

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    struct SnapAimEvaluator {
        /// Evaluates the difficulty of aiming the current object, based on:
        /// - cursor velocity to the current object
        /// - angle difficulty
        /// - sharp velocity increases
        /// - and slider difficulty
        static double evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                             bool with_slider_travel_distance);

        static double calc_angle_acuteness(double angle);
    };
}}}}} // namespace pppp::osu::difficulty::evaluators::aim

#endif
