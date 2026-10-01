// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_TAIKO_COLOUR_DATA_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_TAIKO_COLOUR_DATA_H

#include "pppp/taiko/difficulty/preprocessing/colour/data/alternating_mono_pattern.h"
#include "pppp/taiko/difficulty/preprocessing/colour/data/mono_streak.h"
#include "pppp/taiko/difficulty/preprocessing/colour/data/repeating_hit_patterns.h"

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing {
    struct TaikoDifficultyHitObject;

    namespace colour {
        /// Stores colour compression information for a TaikoDifficultyHitObject.
        struct TaikoColourData {
            /// The MonoStreak that encodes this note.
            data::MonoStreak* mono_streak;

            /// The AlternatingMonoPattern that encodes this note.
            data::AlternatingMonoPattern* alternating_mono_pattern;

            /// The RepeatingHitPatterns that encodes this note.
            data::RepeatingHitPatterns* repeating_hit_pattern;

            TaikoColourData()
                : mono_streak(0),
                  alternating_mono_pattern(0),
                  repeating_hit_pattern(0) {}

            /// The closest past TaikoDifficultyHitObject that's not the same colour.
            TaikoDifficultyHitObject* previous_colour_change() const;

            /// The closest future TaikoDifficultyHitObject that's not the same colour.
            TaikoDifficultyHitObject* next_colour_change() const;
        };
    } // namespace colour
}}}} // namespace pppp::taiko::difficulty::preprocessing

#endif
