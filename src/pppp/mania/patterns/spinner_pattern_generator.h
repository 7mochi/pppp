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
        SpinnerPatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info_in,
                                const pppp::beatmaps::Beatmap& beatmap_in, const SourceObject& hit_object_in,
                                const Pattern& previous_pattern_in, int total_columns_in,
                                pppp::utils::LegacyRandom& random_in)
            : LegacyPatternGenerator(info_in, beatmap_in, hit_object_in, previous_pattern_in, total_columns_in,
                                     random_in) {}

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
