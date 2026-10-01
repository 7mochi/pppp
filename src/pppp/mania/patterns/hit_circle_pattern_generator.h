// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_HIT_CIRCLE_PATTERN_GENERATOR_H
#define PPPP_MANIA_PATTERNS_HIT_CIRCLE_PATTERN_GENERATOR_H

#include "pppp/mania/patterns/legacy_pattern_generator.h"
#include "pppp/utils/vector2.h"
#include <vector>

namespace pppp { namespace mania { namespace patterns {
    /// Converter for legacy "HitCircle" hit objects.
    class HitCirclePatternGenerator : public LegacyPatternGenerator {
    public:
        HitCirclePatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info_in,
                                  const pppp::beatmaps::Beatmap& beatmap_in,
                                  const SourceObject& hit_object_in, const Pattern& previous_pattern_in,
                                  int total_columns_in, pppp::utils::LegacyRandom& random_in,
                                  double previous_time, pppp::utils::Vector2 previous_position,
                                  double density, int last_stair);

        int stair_type;

        void generate(std::vector<Pattern>& out);

    private:
        int convert_type;

        void add_to_pattern(Pattern& pattern, int column) const;

        /// Generates random notes.
        /// This will generate as many as it can up to note_count, accounting for
        /// any stacks if convert_type is forcing no stacks.
        /// @param note_count The amount of notes to generate.
        /// @returns The pattern containing the hit objects.
        Pattern generate_random_notes(int note_count);

        /// Whether this hit object can generate a note in the special column.
        bool has_special_column() const;

        int get_random_note_count(double p2, double p3, double p4, double p5);

        /// Generates a count of notes to be generated from a list of probabilities.
        /// @param centre_probability The probability for a note to be added to the centre column.
        /// @param p2 Probability for 2 notes to be generated.
        /// @param p3 Probability for 3 notes to be generated.
        /// @param add_to_centre Whether to add a note to the centre column.
        /// @returns The amount of notes to be generated. The note to be added to the centre column
        /// will NOT be part of this count.
        int get_random_note_count_mirrored(double centre_probability, double p2, double p3,
                                           bool* add_to_centre);

        /// Generates a random pattern.
        /// @param p2 Probability for 2 notes to be generated.
        /// @param p3 Probability for 3 notes to be generated.
        /// @param p4 Probability for 4 notes to be generated.
        /// @param p5 Probability for 5 notes to be generated.
        /// @returns The pattern containing the hit objects.
        Pattern generate_random_pattern(double p2, double p3, double p4, double p5);

        /// Generates a random pattern which has both normal and mirrored notes.
        /// @param centre_probability The probability for a note to be added to the centre column.
        /// @param p2 Probability for 2 notes to be generated.
        /// @param p3 Probability for 3 notes to be generated.
        /// @returns The pattern containing the hit objects.
        Pattern generate_random_pattern_with_mirrored(double centre_probability, double p2, double p3);

        Pattern generate_core();
    };
}}} // namespace pppp::mania::patterns

#endif
