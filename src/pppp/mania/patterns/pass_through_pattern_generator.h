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
        PassThroughPatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info,
                                    const pppp::beatmaps::Beatmap& beatmap, const SourceObject& hit_object,
                                    const Pattern& previous_pattern, int total_columns,
                                    pppp::utils::LegacyRandom& random)
            : LegacyPatternGenerator(info, beatmap, hit_object, previous_pattern, total_columns, random) {}

        void generate(std::vector<Pattern>& out);
    };
}}} // namespace pppp::mania::patterns

#endif
