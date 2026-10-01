// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_OSU_DIFFICULTY_ATTRIBUTES_H
#define PPPP_OSU_DIFFICULTY_OSU_DIFFICULTY_ATTRIBUTES_H

#include "pppp/common/difficulty_attributes.h"

namespace pppp { namespace osu { namespace difficulty {
    struct OsuDifficultyAttributes : pppp::common::DifficultyAttributes {
        /// The difficulty corresponding to the aim skill.
        double aim_difficulty;

        /// The difficulty corresponding to the speed skill.
        double speed_difficulty;

        /// The difficulty corresponding to the reading skill.
        double reading_difficulty;

        /// The difficulty corresponding to the flashlight skill.
        double flashlight_difficulty;

        /// Describes how much of aim_difficulty is contributed to by hitcircles or sliders.
        /// A value closer to 1.0 indicates most of aim_difficulty is contributed by hitcircles.
        /// A value closer to 0.0 indicates most of aim_difficulty is contributed by sliders.
        /// @see aim_difficulty
        double slider_factor;

        double aim_difficult_strain_count;

        double speed_difficult_strain_count;

        double reading_difficult_note_count;

        /// The number of sliders weighted by difficulty.
        double aim_difficult_slider_count;

        /// Describes how much of aim_difficult_strain_count is contributed to by hitcircles or
        /// sliders.
        /// A value closer to 0.0 indicates most of aim_difficult_strain_count is contributed by
        /// hitcircles. A value closer to Infinity indicates most of aim_difficult_strain_count is contributed
        /// by sliders.
        /// @see aim_difficult_strain_count
        double aim_top_weighted_slider_factor;

        /// Describes how much of speed_difficult_strain_count is contributed to by hitcircles or
        /// sliders.
        /// A value closer to 0.0 indicates most of speed_difficult_strain_count is contributed by
        /// hitcircles. A value closer to Infinity indicates most of speed_difficult_strain_count is
        /// contributed by sliders.
        /// @see speed_difficult_strain_count
        double speed_top_weighted_slider_factor;

        /// The number of clickable objects weighted by difficulty.
        /// @see speed_difficulty
        double speed_note_count;

        /// The number of hitcircles in the beatmap.
        int hit_circle_count;

        /// The number of sliders in the beatmap.
        int slider_count;

        int large_tick_count;

        /// The number of spinners in the beatmap.
        int spinner_count;

        double nested_score_per_object;
        double legacy_score_base_multiplier;
        double maximum_legacy_combo_score;

        OsuDifficultyAttributes();
    };
}}} // namespace pppp::osu::difficulty

#endif
