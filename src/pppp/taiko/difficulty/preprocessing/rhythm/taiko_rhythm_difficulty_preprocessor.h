// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_TAIKO_RHYTHM_DIFFICULTY_PREPROCESSOR_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_RHYTHM_TAIKO_RHYTHM_DIFFICULTY_PREPROCESSOR_H

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"
#include "pppp/taiko/difficulty/utils/interval_grouping_utils.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace rhythm {
    const double SNAP_TOLERANCE = utils::MARGIN_OF_ERROR;

    void process_and_assign(const std::vector<TaikoDifficultyHitObject*>& note_objects,
                            std::vector<data::SameRhythmHitObjectGrouping>& rhythm_groupings,
                            std::vector<data::SamePatternsGroupedHitObjects>& pattern_groupings);
}}}}} // namespace pppp::taiko::difficulty::preprocessing::rhythm

#endif
