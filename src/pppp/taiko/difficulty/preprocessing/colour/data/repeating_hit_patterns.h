// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_DATA_REPEATING_HIT_PATTERNS_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_DATA_REPEATING_HIT_PATTERNS_H

#include "pppp/taiko/difficulty/preprocessing/colour/data/alternating_mono_pattern.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace colour {
    namespace data {
        /// Encodes a list of AlternatingMonoPatterns, grouped together by back and forth repetition
        /// of the same AlternatingMonoPattern. Also stores the repetition interval between this and
        /// the previous RepeatingHitPatterns.
        class RepeatingHitPatterns {
        public:
            /// The AlternatingMonoPatterns that are grouped together within this
            /// RepeatingHitPatterns.
            std::vector<AlternatingMonoPattern*> alternating_mono_patterns;

            /// The previous RepeatingHitPatterns. This is used to determine the repetition interval.
            RepeatingHitPatterns* previous;

            /// How many RepeatingHitPatterns between the current and previous identical
            /// RepeatingHitPatterns. If no repetition is found this will have a value of
            /// the maximum repetition interval plus one.
            int repetition_interval;

            RepeatingHitPatterns()
                : previous(0),
                  repetition_interval(MAX_REPETITION_INTERVAL + 1) {}

            /// The first hit object in this RepeatingHitPatterns.
            TaikoDifficultyHitObject* first_hit_object() const {
                return alternating_mono_patterns[0]->first_hit_object();
            }

            /// Finds the closest previous RepeatingHitPatterns that has the identical
            /// AlternatingMonoPatterns. Interval is defined as the amount of RepeatingHitPatterns
            /// chunks between the current and repeated patterns.
            void find_repetition_interval();

        private:
            /// Maximum amount of RepeatingHitPatterns to look back to find a repetition.
            static const int MAX_REPETITION_INTERVAL = 16;

            /// Whether other is considered a repetition of this pattern. This is true if other's
            /// first two payloads have identical mono lengths.
            bool is_repetition_of(const RepeatingHitPatterns& other) const;
        };
}}}}}} // namespace pppp::taiko::difficulty::preprocessing::colour::data

#endif
