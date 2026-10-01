// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_OSU_LEGACY_SCORE_MISS_CALCULATOR_H
#define PPPP_OSU_DIFFICULTY_OSU_LEGACY_SCORE_MISS_CALCULATOR_H

#include "pppp/common/score_info.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"

namespace pppp { namespace osu { namespace difficulty {
    class OsuLegacyScoreMissCalculator {
    public:
        const pppp::common::ScoreInfo& score;
        const OsuDifficultyAttributes& attributes;

        OsuLegacyScoreMissCalculator(const pppp::common::ScoreInfo& score_in,
                                     const OsuDifficultyAttributes& attributes_in)
            : score(score_in),
              attributes(attributes_in) {}

        double calculate() const;

    private:
        int count_of(int result) const;

        /// Calculates the amount of score that would be achieved at a given combo.
        double calculate_score_at_combo(double combo, double relevant_combo_per_object,
                                        double score_v1_multiplier) const;

        /// Calculates the relevant combo per object for legacy score. This assumes a uniform
        /// distribution for circles and sliders. This handles cases where objects, such as buzz
        /// sliders, do not fit a normal arithmetic progression model.
        double calculate_relevant_score_combo_per_object() const;

        /// A harsher version of the current combo-based miss count, used to provide a reasonable
        /// value for cases where the score-based miss count cannot.
        double calculate_maximum_combo_based_miss_count() const;
    };
}}} // namespace pppp::osu::difficulty

#endif
