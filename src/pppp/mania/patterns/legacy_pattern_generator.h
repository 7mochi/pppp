// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_LEGACY_PATTERN_GENERATOR_H
#define PPPP_MANIA_PATTERNS_LEGACY_PATTERN_GENERATOR_H

#include "pppp/config.h" // IWYU pragma: export
#include "pppp/mania/patterns/pattern_generator.h"
#include "pppp/utils/random/osu.h"

namespace pppp { namespace mania { namespace patterns {
    enum HitSampleBits { HIT_NORMAL = 1 << 0, HIT_WHISTLE = 1 << 1, HIT_FINISH = 1 << 2, HIT_CLAP = 1 << 3 };

    bool has_flag(int flags, int flag);

    bool has_sample(unsigned hitsound, unsigned bit);

    ManiaHitObject make_note(int column, double start_time);

    ManiaHitObject make_hold(int column, double start_time, double end_time);

    /// A pattern generator for legacy hit objects.
    class LegacyPatternGenerator : public PatternGenerator {
    public:
        /// The random number generator to use.
        pppp::utils::LegacyRandom& random;

        /// The column index at which to start generating random notes.
        int random_start;

        /// A difficulty factor used for various conversion methods from osu!stable.
        double conversion_difficulty;

        LegacyPatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info,
                               const pppp::beatmaps::Beatmap& beatmap, const SourceObject& hit_object,
                               const Pattern& previous_pattern, int total_columns,
                               pppp::utils::LegacyRandom& random);

        /// Converts an x-position into a column.
        /// @param position The x-position.
        /// @param allow_special Whether to treat as 7K + 1.
        /// @returns The column.
        int get_column(float position, bool allow_special) const;

        /// Returns a random column index in the range [lower_bound, upper_bound).
        /// @param lower_bound The minimum column index. If empty, random_start is used.
        /// @param upper_bound The maximum column index. If empty, total_columns is used.
        int get_random_column(nonstd::optional<int> lower_bound = nonstd::optional<int>(),
                              nonstd::optional<int> upper_bound = nonstd::optional<int>());

        /// Generates a count of notes to be generated from probabilities.
        /// @param p2 Probability for 2 notes to be generated.
        /// @param p3 Probability for 3 notes to be generated.
        /// @param p4 Probability for 4 notes to be generated.
        /// @param p5 Probability for 5 notes to be generated.
        /// @param p6 Probability for 6 notes to be generated.
        /// @returns The amount of notes to be generated.
        int get_random_note_count(double p2, double p3, double p4, double p5, double p6);

        /// Finds a new column in which a hit object can be placed. This uses get_random_column to pick
        /// the next candidate column.
        /// @param initial_column The initial column to test. This may be returned if it is already a
        /// valid column.
        /// @param lower_bound The minimum column index. If empty, random_start is used.
        /// @param upper_bound The maximum column index. If empty, total_columns is used.
        /// @param gathered_step Whether to step through the columns in order rather than picking them
        /// at random.
        /// @param forbidden_column A column that is never a valid candidate, or -1 for none.
        /// @param first A pattern the validity of a column should be checked against. A column is not
        /// a valid candidate if a hit object occupies the same column in it.
        /// @param second A second such pattern, or null.
        /// @returns The column, or empty when there are no valid candidate columns.
        nonstd::optional<int>
        find_available_column(int initial_column, nonstd::optional<int> lower_bound = nonstd::optional<int>(),
                              nonstd::optional<int> upper_bound = nonstd::optional<int>(),
                              bool gathered_step = false, int forbidden_column = -1, const Pattern* first = 0,
                              const Pattern* second = 0);

    private:
        bool is_valid(int column, int forbidden_column, const Pattern* first, const Pattern* second) const;
    };
}}} // namespace pppp::mania::patterns

#endif
