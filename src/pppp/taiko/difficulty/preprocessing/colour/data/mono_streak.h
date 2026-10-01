// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_DATA_MONO_STREAK_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_COLOUR_DATA_MONO_STREAK_H

#include "pppp/config.h" // IWYU pragma: export
#include "pppp/taiko/object/taiko_hit_object.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing {
    struct TaikoDifficultyHitObject;

    namespace colour { namespace data {
        struct AlternatingMonoPattern;

        /// Encode colour information for a sequence of TaikoDifficultyHitObjects. Consecutive
        /// TaikoDifficultyHitObjects of the same HitType are encoded within the same MonoStreak.
        struct MonoStreak {
            /// List of hit objects that are encoded within this MonoStreak.
            std::vector<TaikoDifficultyHitObject*> hit_objects;

            /// The parent AlternatingMonoPattern that contains this MonoStreak.
            AlternatingMonoPattern* parent;

            /// Index of this MonoStreak within its parent AlternatingMonoPattern.
            int index;

            MonoStreak()
                : parent(0),
                  index(0) {}

            /// The first hit object in this MonoStreak.
            TaikoDifficultyHitObject* first_hit_object() const { return hit_objects[0]; }

            /// The last hit object in this MonoStreak.
            TaikoDifficultyHitObject* last_hit_object() const { return hit_objects[hit_objects.size() - 1]; }

            /// The hit type of all objects encoded within this MonoStreak.
            nonstd::optional<object::HitType> hit_type() const;

            /// The position of the given hit object within this MonoStreak, or -1 if it is not in it.
            int index_of(const TaikoDifficultyHitObject* hit_object) const {
                for (size_t i = 0; i < hit_objects.size(); i++) {
                    if (hit_objects[i] == hit_object) {
                        return static_cast<int>(i);
                    }
                }
                return -1;
            }

            /// How long the mono pattern encoded within is.
            int run_length() const { return static_cast<int>(hit_objects.size()); }
        };
    }} // namespace colour::data
}}}} // namespace pppp::taiko::difficulty::preprocessing

#endif
