// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_TAIKO_RHYTHM_DATA_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_TAIKO_RHYTHM_DATA_H

#include "pppp/taiko/difficulty/preprocessing/rhythm/data/same_patterns_grouped_hit_objects.h"
#include "pppp/taiko/difficulty/preprocessing/rhythm/data/same_rhythm_hit_object_grouping.h"

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace rhythm {
    /// List of most common rhythm changes in taiko maps. Based on how each object's interval
    /// compares to the previous object.
    /// @remarks The general guidelines for the values are:
    /// - rhythm changes with ratio closer to 1 (that are not 1) are harder to play,
    /// - speeding up is generally harder than slowing down (with exceptions of rhythm changes
    ///   requiring a hand switch).
    const double COMMON_RATIOS[9] = {1.0 / 1.0, 2.0 / 1.0, 1.0 / 2.0, 3.0 / 1.0, 1.0 / 3.0,
                                     3.0 / 2.0, 2.0 / 3.0, 5.0 / 4.0, 4.0 / 5.0};

    /// Stores rhythm data for a TaikoDifficultyHitObject.
    struct TaikoRhythmData {
        /// The group of hit objects with consistent rhythm that this object belongs to.
        data::SameRhythmHitObjectGrouping* same_rhythm_grouping;
        /// The larger pattern of rhythm groups that this object is part of.
        data::SamePatternsGroupedHitObjects* same_patterns_grouping;
        /// The ratio of current delta time to previous delta time for the rhythm change. A ratio above
        /// 1 indicates a slow-down; a ratio below 1 indicates a speed-up.
        /// @remarks This is snapped to the closest matching common ratio.
        double ratio;

        TaikoRhythmData()
            : same_rhythm_grouping(0),
              same_patterns_grouping(0),
              ratio(1.0) {}
    };
}}}}} // namespace pppp::taiko::difficulty::preprocessing::rhythm

#endif
