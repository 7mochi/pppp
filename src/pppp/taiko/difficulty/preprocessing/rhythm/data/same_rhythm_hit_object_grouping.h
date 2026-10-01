// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_DATA_SAME_RHYTHM_HIT_OBJECT_GROUPING_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_DATA_SAME_RHYTHM_HIT_OBJECT_GROUPING_H

#include "pppp/config.h" // IWYU pragma: export
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing {
    struct TaikoDifficultyHitObject;

    namespace rhythm { namespace data {
        /// Represents a group of TaikoDifficultyHitObjects with no rhythm variation.
        struct SameRhythmHitObjectGrouping {
            std::vector<TaikoDifficultyHitObject*> hit_objects;

            SameRhythmHitObjectGrouping* previous;

            /// The normalised interval in ms of each hit object in this SameRhythmHitObjectGrouping. This is
            /// only defined if there are more than two hit objects in this SameRhythmHitObjectGrouping.
            nonstd::optional<double> hit_object_interval;

            /// The normalised ratio of hit_object_interval between this and the previous
            /// SameRhythmHitObjectGrouping. In the case where one or both of the hit_object_interval is
            /// undefined, this will have a value of 1.
            double hit_object_interval_ratio;

            /// The interval between the first hit object of this grouping and the previous one's.
            double interval;

            SameRhythmHitObjectGrouping(SameRhythmHitObjectGrouping* previous_in,
                                        const std::vector<TaikoDifficultyHitObject*>& hit_objects_in);

            TaikoDifficultyHitObject* first_hit_object() const { return hit_objects[0]; }

            /// The start time of the first hit object.
            double start_time() const;

            /// The interval between the first and final hit object within this group.
            double duration() const;
        };
    }} // namespace rhythm::data
}}}} // namespace pppp::taiko::difficulty::preprocessing

#endif
