// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_DATA_ALTERNATING_MONO_PATTERN_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_DATA_ALTERNATING_MONO_PATTERN_H

#include "pppp/taiko/difficulty/preprocessing/colour/data/mono_streak.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace colour {
    namespace data {
        class RepeatingHitPatterns;

        /// Encodes a list of MonoStreaks. MonoStreaks with the same RunLength are grouped together.
        struct AlternatingMonoPattern {
            /// MonoStreaks that are grouped together within this AlternatingMonoPattern.
            std::vector<MonoStreak*> mono_streaks;

            /// The parent RepeatingHitPatterns that contains this AlternatingMonoPattern.
            RepeatingHitPatterns* parent;

            /// Index of this AlternatingMonoPattern within its parent RepeatingHitPatterns.
            int index;

            AlternatingMonoPattern()
                : parent(0),
                  index(0) {}

            /// The first hit object in this AlternatingMonoPattern.
            TaikoDifficultyHitObject* first_hit_object() const { return mono_streaks[0]->first_hit_object(); }

            /// Determine if this AlternatingMonoPattern is a repetition of another one. This is a
            /// strict comparison and is true if and only if the colour sequence is exactly the same.
            bool is_repetition_of(const AlternatingMonoPattern& other) const;

            /// Determine if this AlternatingMonoPattern has the same mono length of another one.
            bool has_identical_mono_length(const AlternatingMonoPattern& other) const;
        };
}}}}}} // namespace pppp::taiko::difficulty::preprocessing::colour::data

#endif
