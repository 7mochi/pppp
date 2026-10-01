// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/aim/agility_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace aim {
    namespace {
        double high_bpm_bonus(double ms) { return 1.0 / (1.0 - utils::pow(0.2, ms / 1000.0)); }
    } // namespace

    double AgilityEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current) {
        if (current.base_is_spinner) {
            return 0.0;
        }

        const double distance_cap =
            preprocessing::NORMALISED_DIAMETER * 1.2; // 1.2 circles distance between centers

        const preprocessing::OsuDifficultyHitObject* prev =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(0));

        double travel_distance = prev ? prev->lazy_travel_distance : 0.0;
        double distance = travel_distance + current.lazy_jump_distance;

        double distance_scaled = std::min(distance, distance_cap) / distance_cap;

        double agility_difficulty = distance_scaled * 1000.0 / current.adjusted_delta_time;

        agility_difficulty *= utils::pow(current.small_circle_bonus, 1.5);

        agility_difficulty *= high_bpm_bonus(current.adjusted_delta_time);

        return agility_difficulty;
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::aim
