// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_EVALUATORS_READING_EVALUATOR_H
#define PPPP_OSU_DIFFICULTY_EVALUATORS_READING_EVALUATOR_H

#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators {
    /// The window, in milliseconds, within which objects are considered visible.
    const double READING_WINDOW_SIZE = 3000;
    /// The distance at which a past object's difficulty stops influencing the current one.
    const double DISTANCE_INFLUENCE_THRESHOLD = preprocessing::NORMALISED_DIAMETER * 1.5;

    struct ReadingEvaluator {
        static double evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current,
                                             bool hidden);
    };
}}}} // namespace pppp::osu::difficulty::evaluators

#endif
