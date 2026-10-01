// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_DATA_SAME_PATTERNS_GROUPED_HIT_OBJECTS_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_DATA_SAME_PATTERNS_GROUPED_HIT_OBJECTS_H

#include "pppp/taiko/difficulty/preprocessing/rhythm/data/same_rhythm_hit_object_grouping.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace rhythm {
    namespace data {
        /// Represents SameRhythmHitObjectGroupings grouped by their start time's interval.
        struct SamePatternsGroupedHitObjects {
            std::vector<SameRhythmHitObjectGrouping*> groups;

            SamePatternsGroupedHitObjects* previous;

            SamePatternsGroupedHitObjects()
                : previous(0) {}

            TaikoDifficultyHitObject* first_hit_object() const { return groups[0]->first_hit_object(); }

            /// The interval between groups. If there is only one group, this will have the value of
            /// the first group's interval.
            double group_interval() const;

            /// The ratio of the group interval between this and the previous
            /// SamePatternsGroupedHitObjects. In the case where there is no previous one, this will
            /// have a value of 1.
            double interval_ratio() const;
        };
}}}}}} // namespace pppp::taiko::difficulty::preprocessing::rhythm::data

#endif
