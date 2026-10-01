// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/patterns/legacy_pattern_generator.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace mania { namespace patterns {
    bool has_flag(int flags, int flag) { return (flags & flag) != 0; }

    bool has_sample(unsigned hitsound, unsigned bit) { return (hitsound & bit) != 0; }

    ManiaHitObject make_note(int column, double start_time) {
        ManiaHitObject note;
        note.column = column;
        note.start_time = start_time;
        note.end_time = start_time;
        note.hold = false;
        return note;
    }

    ManiaHitObject make_hold(int column, double start_time, double end_time) {
        ManiaHitObject hold;
        hold.column = column;
        hold.start_time = start_time;
        hold.end_time = end_time;
        hold.hold = true;
        return hold;
    }

    namespace {
        double conversion_difficulty_of(const pppp::beatmaps::Beatmap& beatmap) {
            double total_break_time = 0.0;

            for (size_t i = 0; i < beatmap.breaks.size(); i++) {
                total_break_time += beatmap.breaks[i].end_time - beatmap.breaks[i].start_time;
            }

            const double last_time =
                beatmap.hit_objects.empty() ? 0.0 : beatmap.hit_objects.back().start_time;
            const double first_time =
                beatmap.hit_objects.empty() ? 0.0 : beatmap.hit_objects.front().start_time;

            // Drain time in seconds
            int drain_time = static_cast<int>((last_time - first_time - total_break_time) / 1000);

            if (drain_time == 0) {
                drain_time = 10000;
            }

            const double difficulty = ((beatmap.difficulty.drain_rate +
                                        utils::math::clamp(beatmap.difficulty.approach_rate, 4, 7)) /
                                           1.5 +
                                       static_cast<double>(beatmap.hit_objects.size()) / drain_time * 9.0) /
                                      38.0 * 5.0 / 1.15;

            return std::min(difficulty, 12.0);
        }
    } // namespace

    LegacyPatternGenerator::LegacyPatternGenerator(
        const pppp::beatmaps::control_points::ControlPointInfo& info_in,
        const pppp::beatmaps::Beatmap& beatmap_in, const SourceObject& hit_object_in,
        const Pattern& previous_pattern_in, int total_columns_in, pppp::utils::LegacyRandom& random_in)
        : PatternGenerator(info_in, beatmap_in, hit_object_in, previous_pattern_in, total_columns_in),
          random(random_in),
          random_start(total_columns_in == 8 ? 1 : 0),
          conversion_difficulty(conversion_difficulty_of(beatmap_in)) {}

    int LegacyPatternGenerator::get_column(float position, bool allow_special) const {
        if (allow_special && total_columns == 8) {
            const float local_x_divisor = 512.0f / 7;
            const int column = static_cast<int>(std::floor(utils::f32(position / local_x_divisor)));
            return utils::math::clamp_int(column, 0, 6) + 1;
        }

        const float local_x_divisor = 512.0f / static_cast<float>(total_columns);
        const int column = static_cast<int>(std::floor(utils::f32(position / local_x_divisor)));
        return utils::math::clamp_int(column, 0, total_columns - 1);
    }

    int LegacyPatternGenerator::get_random_column(nonstd::optional<int> lower_bound,
                                                  nonstd::optional<int> upper_bound) {
        const int low = lower_bound.has_value() ? lower_bound.value() : random_start;
        const int high = upper_bound.has_value() ? upper_bound.value() : total_columns;
        return random.next(low, high);
    }

    int LegacyPatternGenerator::get_random_note_count(double p2, double p3, double p4, double p5, double p6) {
        const double val = random.next_double();

        if (val >= 1 - p6) {
            return 6;
        }
        if (val >= 1 - p5) {
            return 5;
        }
        if (val >= 1 - p4) {
            return 4;
        }
        if (val >= 1 - p3) {
            return 3;
        }
        return val >= 1 - p2 ? 2 : 1;
    }

    bool LegacyPatternGenerator::is_valid(int column, int forbidden_column, const Pattern* first,
                                          const Pattern* second) const {
        if (forbidden_column >= 0 && column == forbidden_column) {
            return false;
        }
        if (first != 0 && first->column_has_object(column)) {
            return false;
        }
        if (second != 0 && second->column_has_object(column)) {
            return false;
        }
        return true;
    }

    nonstd::optional<int> LegacyPatternGenerator::find_available_column(
        int initial_column, nonstd::optional<int> lower_bound, nonstd::optional<int> upper_bound,
        bool gathered_step, int forbidden_column, const Pattern* first, const Pattern* second) {
        const int low = lower_bound.has_value() ? lower_bound.value() : random_start;
        const int high = upper_bound.has_value() ? upper_bound.value() : total_columns;

        // Check for the initial column
        if (is_valid(initial_column, forbidden_column, first, second)) {
            return nonstd::optional<int>(initial_column);
        }

        // At least one free column has to exist or the loop below never ends.
        bool has_valid_columns = false;

        for (int i = low; i < high; i++) {
            has_valid_columns = is_valid(i, forbidden_column, first, second);
            if (has_valid_columns) {
                break;
            }
        }

        if (!has_valid_columns) {
            return nonstd::optional<int>();
        }

        // Iterate until a valid column is found. This is a random iteration in the default case.
        int column = initial_column;
        do {
            if (gathered_step) {
                column++;
                if (column == total_columns) {
                    column = random_start;
                }
            } else {
                column = get_random_column(low, high);
            }
        } while (!is_valid(column, forbidden_column, first, second));

        return nonstd::optional<int>(column);
    }
}}} // namespace pppp::mania::patterns
