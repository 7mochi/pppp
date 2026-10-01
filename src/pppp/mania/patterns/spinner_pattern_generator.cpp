// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/patterns/spinner_pattern_generator.h"

namespace pppp { namespace mania { namespace patterns {
    void SpinnerPatternGenerator::generate(std::vector<Pattern>& out) {
        const int end_time = static_cast<int>(hit_object.end_time);
        const int convert_type =
            previous_pattern.column_with_objects == total_columns ? PATTERN_NONE : PATTERN_FORCE_NOT_STACK;

        const bool generate_hold = end_time - hit_object.start_time >= 100;

        Pattern pattern;
        int column;

        if (total_columns == 8 && has_sample(hit_object.hitsound, HIT_FINISH) &&
            end_time - hit_object.start_time < 1000) {
            column = 0;
        } else {
            const nonstd::optional<int> lower_bound =
                total_columns == 8 ? nonstd::optional<int>() : nonstd::optional<int>(0);
            const int initial = get_random_column(lower_bound);
            const nonstd::optional<int> found =
                has_flag(convert_type, PATTERN_FORCE_NOT_STACK)
                    ? find_available_column(initial, lower_bound, nonstd::optional<int>(), false, -1,
                                            &previous_pattern, 0)
                    : find_available_column(initial, lower_bound, nonstd::optional<int>(), false, -1, 0, 0);
            column = found.has_value() ? found.value() : 0;
        }

        add_to_pattern(pattern, column, generate_hold);

        out.push_back(pattern);
    }

    void SpinnerPatternGenerator::add_to_pattern(Pattern& pattern, int column, bool hold_note) const {
        const int end_time = static_cast<int>(hit_object.end_time);

        if (hold_note) {
            pattern.add(make_hold(column, hit_object.start_time, end_time));
        } else {
            pattern.add(make_note(column, hit_object.start_time));
        }
    }
}}} // namespace pppp::mania::patterns
