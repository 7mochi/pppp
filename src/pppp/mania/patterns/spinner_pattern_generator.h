// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_SPINNER_PATTERN_GENERATOR_H
#define PPPP_MANIA_PATTERNS_SPINNER_PATTERN_GENERATOR_H

#include "pppp/mania/patterns/legacy_pattern_generator.h"
#include <vector>

namespace pppp { namespace mania { namespace patterns {
    /// Converter for legacy "Spinner" hit objects.
    class SpinnerPatternGenerator : public LegacyPatternGenerator {
    public:
        SpinnerPatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info,
                                const pppp::beatmaps::Beatmap& beatmap, const SourceObject& hit_object,
                                const Pattern& previous_pattern, int total_columns,
                                pppp::utils::LegacyRandom& random)
            : LegacyPatternGenerator(info, beatmap, hit_object, previous_pattern, total_columns, random) {}

        void generate(std::vector<Pattern>& out);

    private:
        /// Constructs and adds a note to a pattern.
        /// @param pattern The pattern to add to.
        /// @param column The column to add the note to.
        /// @param hold_note Whether to add a hold note.
        void add_to_pattern(Pattern& pattern, int column, bool hold_note) const;
    };
}}} // namespace pppp::mania::patterns

#endif
