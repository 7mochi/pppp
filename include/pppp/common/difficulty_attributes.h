// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_DIFFICULTY_ATTRIBUTES_H
#define PPPP_COMMON_DIFFICULTY_ATTRIBUTES_H

namespace pppp { namespace common {
    /// Describes the difficulty of a beatmap, as output by the difficulty calculator.
    struct DifficultyAttributes {
        /// The combined star rating of all skills.
        double star_rating;

        /// The maximum achievable combo.
        int max_combo;

        /// Creates new DifficultyAttributes.
        DifficultyAttributes();
    };
}} // namespace pppp::common

#endif
