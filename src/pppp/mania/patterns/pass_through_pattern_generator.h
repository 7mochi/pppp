// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_PASS_THROUGH_PATTERN_GENERATOR_H
#define PPPP_MANIA_PATTERNS_PASS_THROUGH_PATTERN_GENERATOR_H

#include "pppp/mania/patterns/legacy_pattern_generator.h"
#include <vector>

namespace pppp { namespace mania { namespace patterns {
    /// A simple generator which, for any object, if the hitobject has an end time
    /// it becomes a HoldNote or otherwise a Note
    class PassThroughPatternGenerator : public LegacyPatternGenerator {
    public:
        PassThroughPatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info_in,
                                    const pppp::beatmaps::Beatmap& beatmap_in,
                                    const SourceObject& hit_object_in, const Pattern& previous_pattern_in,
                                    int total_columns_in, pppp::utils::LegacyRandom& random_in)
            : LegacyPatternGenerator(info_in, beatmap_in, hit_object_in, previous_pattern_in,
                                     total_columns_in, random_in) {}

        void generate(std::vector<Pattern>& out);
    };
}}} // namespace pppp::mania::patterns

#endif
