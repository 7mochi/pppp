// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_TAIKO_DIFFICULTY_ATTRIBUTES_H
#define PPPP_TAIKO_DIFFICULTY_TAIKO_DIFFICULTY_ATTRIBUTES_H

#include "pppp/common/difficulty_attributes.h"

namespace pppp { namespace taiko { namespace difficulty {
    struct TaikoDifficultyAttributes : pppp::common::DifficultyAttributes {
        /// The difficulty corresponding to the mechanical skills in osu!taiko.
        /// This includes colour and stamina combined.
        double mechanical_difficulty;

        /// The difficulty corresponding to the rhythm skill.
        double rhythm_difficulty;

        /// The difficulty corresponding to the reading skill.
        double reading_difficulty;

        /// The difficulty corresponding to the colour skill.
        double colour_difficulty;

        /// The difficulty corresponding to the stamina skill.
        double stamina_difficulty;

        /// The ratio of stamina difficulty from mono-color (single colour) streams to total stamina
        /// difficulty.
        double mono_stamina_factor;

        /// The factor corresponding to the consistency of a map.
        double consistency_factor;

        double stamina_top_strains;

        TaikoDifficultyAttributes();
    };
}}} // namespace pppp::taiko::difficulty

#endif
