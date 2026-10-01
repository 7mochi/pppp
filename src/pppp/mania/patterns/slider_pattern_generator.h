// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_SLIDER_PATTERN_GENERATOR_H
#define PPPP_MANIA_PATTERNS_SLIDER_PATTERN_GENERATOR_H

#include "pppp/mania/patterns/legacy_pattern_generator.h"
#include <vector>

namespace pppp { namespace mania { namespace patterns {
    /// Converter for legacy "Slider" hit objects.
    class SliderPatternGenerator : public LegacyPatternGenerator {
    public:
        SliderPatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info,
                               const pppp::beatmaps::Beatmap& beatmap, const SourceObject& hit_object,
                               const Pattern& previous_pattern, int total_columns,
                               pppp::utils::LegacyRandom& random);

        int span_count;
        int segment_duration;
        int start_time;
        int end_time;

        void generate(std::vector<Pattern>& out);

    private:
        int convert_type;

        void add_to_pattern(Pattern& pattern, int column, int time, int end_time) const;

        /// Retrieves the sample at a point in time.
        /// @param time The time to retrieve the sample from.
        unsigned sample_info_list_at(int time) const;

        /// Generates random hold notes that start at and span the same amount of rows.
        /// @param time Start time of each hold note.
        /// @param note_count Number of hold notes.
        /// @returns The pattern containing the hit objects.
        Pattern generate_random_hold_notes(int time, int note_count);

        /// Generates random notes, with one note per row and no stacking.
        /// @param time The start time.
        /// @param note_count The number of notes.
        /// @returns The pattern containing the hit objects.
        Pattern generate_random_notes(int time, int note_count);

        /// Generates a stair of notes, with one note per row.
        /// @param time The start time.
        /// @returns The pattern containing the hit objects.
        Pattern generate_stair(int time);

        /// Generates random notes with 1-2 notes per row and no stacking.
        /// @param time The start time.
        /// @returns The pattern containing the hit objects.
        Pattern generate_random_multiple_notes(int time);

        /// Generates random hold notes. The amount of hold notes generated is determined by
        /// probabilities.
        /// @param time The hold note start time.
        /// @param p2 The probability required for 2 hold notes to be generated.
        /// @param p3 The probability required for 3 hold notes to be generated.
        /// @param p4 The probability required for 4 hold notes to be generated.
        /// @returns The pattern containing the hit objects.
        Pattern generate_n_random_notes(int time, double p2, double p3, double p4);

        /// Generates tiled hold notes. You can think of this as a stair of hold notes.
        /// @param time The first hold note start time.
        /// @returns The pattern containing the hit objects.
        Pattern generate_tiled_hold_notes(int time);

        /// Generates a hold note alongside normal notes.
        /// @param time The start time of notes.
        /// @returns The pattern containing the hit objects.
        Pattern generate_hold_and_normal_notes(int time);

        Pattern generate_core();
    };
}}} // namespace pppp::mania::patterns

#endif
