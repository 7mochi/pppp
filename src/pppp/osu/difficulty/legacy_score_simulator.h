// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_LEGACY_SCORE_SIMULATOR_H
#define PPPP_OSU_DIFFICULTY_LEGACY_SCORE_SIMULATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/osu_beatmap.h"

namespace pppp { namespace osu { namespace difficulty {
    const double MAXIMUM_ROTATIONS_PER_SECOND = 477.0 / 60;

    // Normally, this value depends on the final overall difficulty. For simplicity, we'll only consider
    // the worst case that maximises bonus score.
    // As we're primarily concerned with computing the maximum theoretical final score,
    // this will have the final effect of slightly underestimating bonus score achieved on stable when
    // converting from score V1.
    const double MINIMUM_ROTATIONS_PER_SECOND = 3;

    struct LegacyScoreAttributes {
        /// The accuracy portion of the legacy (ScoreV1) total score.
        double accuracy_score;

        /// The combo-multiplied portion of the legacy (ScoreV1) total score.
        double combo_score;

        /// A ratio of standardised score to legacy score for the bonus part of total score.
        double bonus_score_ratio;

        /// The bonus portion of the legacy (ScoreV1) total score.
        int bonus_score;

        /// The max combo of the legacy (ScoreV1) total score.
        int max_combo;

        LegacyScoreAttributes();
    };

    /// Calculates the average amount of score per object that is caused by nested judgements such as
    /// slider-ticks and spinners.
    double calculate_nested_score_per_object(const OsuBeatmap& pb, int object_count);

    /// The beatmap's legacy score base multiplier (the stable "peppy" stars): a wrapper around
    /// calculate_difficulty_peppy_stars that reads the beatmap's own difficulty settings.
    int peppy_stars(const pppp::beatmaps::Beatmap& beatmap);

    LegacyScoreAttributes simulate(const pppp::beatmaps::Beatmap& beatmap, const OsuBeatmap& pb);

    double get_legacy_score_multiplier(const pppp::mods::Mod* mods, size_t mod_count);
}}} // namespace pppp::osu::difficulty

#endif
