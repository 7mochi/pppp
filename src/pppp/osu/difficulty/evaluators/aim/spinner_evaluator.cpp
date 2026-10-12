// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/spinner_evaluator.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/utils/difficulty_range.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    double SpinnerEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current) {
        if (!current.base_is_spinner || current.spins_required <= 0) {
            return 0.0;
        }

        // The average RPS required over the length of the spinner to clear the spinner.
        double rps =
            pppp::utils::difficulty_range(current.overall_difficulty, object::CLEAR_RPM_RANGE) / 60.0;

        double duration = current.spinner_duration / current.clock_rate / 1000.0;

        return rps / duration; // The longer the spinner the more lenient spinning requirements are.
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
