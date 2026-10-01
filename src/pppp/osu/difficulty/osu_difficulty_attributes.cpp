// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/osu_difficulty_attributes.h"

namespace pppp { namespace osu { namespace difficulty {
    OsuDifficultyAttributes::OsuDifficultyAttributes()
        : aim_difficulty(0.0),
          speed_difficulty(0.0),
          reading_difficulty(0.0),
          flashlight_difficulty(0.0),
          slider_factor(1.0),
          aim_difficult_strain_count(0.0),
          speed_difficult_strain_count(0.0),
          reading_difficult_note_count(0.0),
          aim_difficult_slider_count(0.0),
          aim_top_weighted_slider_factor(0.0),
          speed_top_weighted_slider_factor(0.0),
          speed_note_count(0.0),
          hit_circle_count(0),
          slider_count(0),
          large_tick_count(0),
          spinner_count(0),
          nested_score_per_object(0.0),
          legacy_score_base_multiplier(0.0),
          maximum_legacy_combo_score(0.0) {}
}}} // namespace pppp::osu::difficulty
