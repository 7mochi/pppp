// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_EVALUATORS_AIM_FLOW_AIM_EVALUATOR_H
#define PPPP_OSU_DIFFICULTY_EVALUATORS_AIM_FLOW_AIM_EVALUATOR_H

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    struct FlowAimEvaluator {
        /// Evaluates difficulty of "flow aim" - aiming pattern where player doesn't stop their cursor on
        /// every object and instead "flows" through them.
        static double evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                             bool with_slider_travel_distance);
    };
}}}}} // namespace pppp::osu::difficulty::evaluators::aim

#endif
