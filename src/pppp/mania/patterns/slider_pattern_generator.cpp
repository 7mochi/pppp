// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/patterns/slider_pattern_generator.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace mania { namespace patterns {
    SliderPatternGenerator::SliderPatternGenerator(
        const pppp::beatmaps::control_points::ControlPointInfo& info_in,
        const pppp::beatmaps::Beatmap& beatmap_in, const SourceObject& hit_object_in,
        const Pattern& previous_pattern_in, int total_columns_in, pppp::utils::LegacyRandom& random_in)
        : LegacyPatternGenerator(info_in, beatmap_in, hit_object_in, previous_pattern_in, total_columns_in,
                                 random_in),
          convert_type(PATTERN_NONE) {
        const pppp::beatmaps::control_points::TimingControlPoint* timing_point =
            info.timing_point_at(hit_object.start_time);
        const double timing_beat_length = timing_point != 0
                                              ? timing_point->beat_length
                                              : pppp::beatmaps::control_points::DEFAULT_BEAT_LENGTH;
        const pppp::beatmaps::control_points::DifficultyControlPoint* difficulty_point =
            info.difficulty_point_at(hit_object.start_time);
        const double slider_velocity =
            difficulty_point != 0 ? difficulty_point->clamped_slider_velocity() : 1.0;

        double beat_length = timing_beat_length;
        const double slider_velocity_as_beat_length = -100.0 / slider_velocity;
        if (slider_velocity_as_beat_length < 0) {
            double clamped = utils::f32(-slider_velocity_as_beat_length);
            if (clamped < 10.0) {
                clamped = 10.0;
            }
            if (clamped > 10000.0) {
                clamped = 10000.0;
            }
            beat_length = timing_beat_length * (clamped / 100.0);
        }

        span_count = hit_object.slides > 0 ? hit_object.slides : 1;
        start_time = static_cast<int>(utils::math::round_half_even(hit_object.start_time));

        // This matches stable's calculation.
        end_time =
            static_cast<int>(std::floor(start_time + hit_object.expected_distance * beat_length * span_count *
                                                         0.01 / beatmap.difficulty.slider_multiplier));
        segment_duration = (end_time - start_time) / span_count;

        const pppp::beatmaps::control_points::EffectControlPoint* effect_point =
            info.effect_point_at(hit_object.start_time);
        convert_type =
            (effect_point != 0 && effect_point->kiai_mode) ? PATTERN_NONE : PATTERN_LOW_PROBABILITY;
    }

    unsigned SliderPatternGenerator::sample_info_list_at(int time) const {
        if (hit_object.node_sounds == 0 || hit_object.node_sounds->empty()) {
            return hit_object.hitsound;
        }

        const int index = segment_duration == 0 ? 0 : (time - start_time) / segment_duration;
        const size_t at = index < 0 ? 0 : static_cast<size_t>(index);

        return at < hit_object.node_sounds->size() ? (*hit_object.node_sounds)[at] : hit_object.hitsound;
    }

    void SliderPatternGenerator::add_to_pattern(Pattern& pattern, int column, int time, int note_end) const {
        if (time == note_end) {
            pattern.add(make_note(column, time));
        } else {
            pattern.add(make_hold(column, time, note_end));
        }
    }

    Pattern SliderPatternGenerator::generate_random_hold_notes(int time, int note_count) {
        // - - - -
        // ■ - ■ ■
        // □ - □ □
        // ■ - ■ ■

        Pattern pattern;

        const int usable_columns = total_columns - random_start - previous_pattern.column_with_objects;
        int next_column = get_random_column();

        for (int i = 0; i < std::min(usable_columns, note_count); i++) {
            // Find available column
            const nonstd::optional<int> found =
                find_available_column(next_column, nonstd::optional<int>(), nonstd::optional<int>(), false,
                                      -1, &pattern, &previous_pattern);
            if (!found.has_value()) {
                return pattern;
            }
            next_column = found.value();
            add_to_pattern(pattern, next_column, time, end_time);
        }

        // This is can't be combined with the above loop due to RNG
        for (int i = 0; i < note_count - usable_columns; i++) {
            const nonstd::optional<int> found = find_available_column(
                next_column, nonstd::optional<int>(), nonstd::optional<int>(), false, -1, &pattern, 0);
            if (!found.has_value()) {
                return pattern;
            }
            next_column = found.value();
            add_to_pattern(pattern, next_column, time, end_time);
        }

        return pattern;
    }

    Pattern SliderPatternGenerator::generate_random_notes(int time, int note_count) {
        // - - - -
        // x - - -
        // - - x -
        // - - - x
        // x - - -

        Pattern pattern;

        int next_column = get_column(static_cast<float>(hit_object.position.x), true);
        if (has_flag(convert_type, PATTERN_FORCE_NOT_STACK) &&
            previous_pattern.column_with_objects < total_columns) {
            const nonstd::optional<int> found =
                find_available_column(next_column, nonstd::optional<int>(), nonstd::optional<int>(), false,
                                      -1, &previous_pattern, 0);
            if (!found.has_value()) {
                return pattern;
            }
            next_column = found.value();
        }

        int last_column = next_column;

        for (int i = 0; i < note_count; i++) {
            add_to_pattern(pattern, next_column, time, time);
            const nonstd::optional<int> found = find_available_column(
                next_column, nonstd::optional<int>(), nonstd::optional<int>(), false, last_column, 0, 0);
            if (!found.has_value()) {
                return pattern;
            }
            next_column = found.value();
            last_column = next_column;
            time += segment_duration;
        }

        return pattern;
    }

    Pattern SliderPatternGenerator::generate_stair(int time) {
        // - - - -
        // x - - -
        // - x - -
        // - - x -
        // - - - x
        // - - x -
        // - x - -
        // x - - -

        Pattern pattern;

        int column = get_column(static_cast<float>(hit_object.position.x), true);
        bool increasing = random.next_double() > 0.5;

        for (int i = 0; i <= span_count; i++) {
            add_to_pattern(pattern, column, time, time);
            time += segment_duration;

            // Check if we're at the borders of the stage, and invert the pattern if so
            if (increasing) {
                if (column >= total_columns - 1) {
                    increasing = false;
                    column--;
                } else {
                    column++;
                }
            } else {
                if (column <= random_start) {
                    increasing = true;
                    column++;
                } else {
                    column--;
                }
            }
        }

        return pattern;
    }

    Pattern SliderPatternGenerator::generate_random_multiple_notes(int time) {
        // - - - -
        // x - - -
        // - x x -
        // - - - x
        // x - x -

        Pattern pattern;

        const bool legacy = total_columns >= 4 && total_columns <= 8;
        const int interval = random.next(1, total_columns - (legacy ? 1 : 0));

        int next_column = get_column(static_cast<float>(hit_object.position.x), true);

        for (int i = 0; i <= span_count; i++) {
            add_to_pattern(pattern, next_column, time, time);

            next_column += interval;
            if (next_column >= total_columns - random_start) {
                next_column = next_column - total_columns - random_start + (legacy ? 1 : 0);
            }
            next_column += random_start;

            // If we're in 2K, let's not add many consecutive doubles
            if (total_columns > 2) {
                add_to_pattern(pattern, next_column, time, time);
            }

            next_column = get_random_column();
            time += segment_duration;
        }

        return pattern;
    }

    Pattern SliderPatternGenerator::generate_n_random_notes(int time, double p2, double p3, double p4) {
        // - - - -
        // ■ - ■ ■
        // □ - □ □
        // ■ - ■ ■

        switch (total_columns) {
        case 2:
            p2 = 0;
            p3 = 0;
            p4 = 0;
            break;
        case 3:
            p2 = std::min(p2, 0.1);
            p3 = 0;
            p4 = 0;
            break;
        case 4:
            p2 = std::min(p2, 0.3);
            p3 = std::min(p3, 0.04);
            p4 = 0;
            break;
        case 5:
            p2 = std::min(p2, 0.34);
            p3 = std::min(p3, 0.1);
            p4 = std::min(p4, 0.03);
            break;
        default: break;
        }

        const unsigned double_sample = HIT_CLAP | HIT_FINISH;

        bool can_generate_two_notes = !has_flag(convert_type, PATTERN_LOW_PROBABILITY);
        can_generate_two_notes &= (hit_object.hitsound & double_sample) != 0 ||
                                  (sample_info_list_at(start_time) & double_sample) != 0;

        if (can_generate_two_notes) {
            p2 = 1;
        }

        return generate_random_hold_notes(time, get_random_note_count(p2, p3, p4, 0, 0));
    }

    Pattern SliderPatternGenerator::generate_tiled_hold_notes(int time) {
        // - - - -
        // ■ ■ ■ ■
        // □ □ □ □
        // □ □ □ □
        // □ □ □ ■
        // □ □ ■ -
        // □ ■ - -
        // ■ - - -

        Pattern pattern;

        const int column_repeat = std::min(span_count, total_columns);

        // Due to integer rounding, this is not guaranteed to be the same as the end time (the
        // class-level variable).
        const int end = time + segment_duration * span_count;

        int next_column = get_column(static_cast<float>(hit_object.position.x), true);
        if (has_flag(convert_type, PATTERN_FORCE_NOT_STACK) &&
            previous_pattern.column_with_objects < total_columns) {
            const nonstd::optional<int> found =
                find_available_column(next_column, nonstd::optional<int>(), nonstd::optional<int>(), false,
                                      -1, &previous_pattern, 0);
            if (!found.has_value()) {
                return pattern;
            }
            next_column = found.value();
        }

        for (int i = 0; i < column_repeat; i++) {
            const nonstd::optional<int> found = find_available_column(
                next_column, nonstd::optional<int>(), nonstd::optional<int>(), false, -1, &pattern, 0);
            if (!found.has_value()) {
                return pattern;
            }
            next_column = found.value();
            add_to_pattern(pattern, next_column, time, end);
            time += segment_duration;
        }

        return pattern;
    }

    Pattern SliderPatternGenerator::generate_hold_and_normal_notes(int time) {
        // - - - -
        // ■ x x -
        // ■ - x x
        // ■ x - x
        // ■ - x x

        Pattern pattern;

        int hold_column = get_column(static_cast<float>(hit_object.position.x), true);
        if (has_flag(convert_type, PATTERN_FORCE_NOT_STACK) &&
            previous_pattern.column_with_objects < total_columns) {
            const nonstd::optional<int> found =
                find_available_column(hold_column, nonstd::optional<int>(), nonstd::optional<int>(), false,
                                      -1, &previous_pattern, 0);
            if (!found.has_value()) {
                return pattern;
            }
            hold_column = found.value();
        }

        // Create the hold note
        add_to_pattern(pattern, hold_column, time, end_time);

        int next_column = get_random_column();
        int note_count;

        if (conversion_difficulty > 6.5) {
            note_count = get_random_note_count(0.63, 0, 0, 0, 0);
        } else if (conversion_difficulty > 4) {
            note_count = get_random_note_count(total_columns < 6 ? 0.12 : 0.45, 0, 0, 0, 0);
        } else if (conversion_difficulty > 2.5) {
            note_count = get_random_note_count(total_columns < 6 ? 0 : 0.24, 0, 0, 0, 0);
        } else {
            note_count = 0;
        }
        note_count = std::min(total_columns - 1, note_count);

        const unsigned audible = HIT_WHISTLE | HIT_FINISH | HIT_CLAP;
        const bool ignore_head = (sample_info_list_at(time) & audible) == 0;

        Pattern row_pattern;

        for (int i = 0; i <= span_count; i++) {
            if (!(ignore_head && time == start_time)) {
                for (int j = 0; j < note_count; j++) {
                    const nonstd::optional<int> found =
                        find_available_column(next_column, nonstd::optional<int>(), nonstd::optional<int>(),
                                              false, hold_column, &row_pattern, 0);
                    if (!found.has_value()) {
                        break;
                    }
                    next_column = found.value();
                    add_to_pattern(row_pattern, next_column, time, time);
                }
            }

            pattern.add(row_pattern);
            row_pattern.clear();

            time += segment_duration;
        }

        return pattern;
    }

    Pattern SliderPatternGenerator::generate_core() {
        if (total_columns == 1) {
            Pattern pattern;
            add_to_pattern(pattern, 0, start_time, end_time);
            return pattern;
        }

        if (span_count > 1) {
            if (segment_duration <= 90) {
                return generate_random_hold_notes(start_time, 1);
            }
            if (segment_duration <= 120) {
                convert_type |= PATTERN_FORCE_NOT_STACK;
                return generate_random_notes(start_time, span_count + 1);
            }
            if (segment_duration <= 160) {
                return generate_stair(start_time);
            }
            if (segment_duration <= 200 && conversion_difficulty > 3) {
                return generate_random_multiple_notes(start_time);
            }

            const double duration = end_time - start_time;
            if (duration >= 4000) {
                return generate_n_random_notes(start_time, 0.23, 0, 0);
            }
            if (segment_duration > 400 && span_count < total_columns - 1 - random_start) {
                return generate_tiled_hold_notes(start_time);
            }
            return generate_hold_and_normal_notes(start_time);
        }

        if (segment_duration <= 110) {
            if (previous_pattern.column_with_objects < total_columns) {
                convert_type |= PATTERN_FORCE_NOT_STACK;
            } else {
                convert_type &= ~PATTERN_FORCE_NOT_STACK;
            }
            return generate_random_notes(start_time, segment_duration < 80 ? 1 : 2);
        }

        const bool low = has_flag(convert_type, PATTERN_LOW_PROBABILITY);

        if (conversion_difficulty > 6.5) {
            return low ? generate_n_random_notes(start_time, 0.78, 0.3, 0)
                       : generate_n_random_notes(start_time, 0.85, 0.36, 0.03);
        }
        if (conversion_difficulty > 4) {
            return low ? generate_n_random_notes(start_time, 0.43, 0.08, 0)
                       : generate_n_random_notes(start_time, 0.56, 0.18, 0);
        }
        if (conversion_difficulty > 2.5) {
            return low ? generate_n_random_notes(start_time, 0.3, 0, 0)
                       : generate_n_random_notes(start_time, 0.37, 0.08, 0);
        }
        return low ? generate_n_random_notes(start_time, 0.17, 0, 0)
                   : generate_n_random_notes(start_time, 0.27, 0, 0);
    }

    void SliderPatternGenerator::generate(std::vector<Pattern>& out) {
        const Pattern original = generate_core();

        if (original.hit_objects.size() == 1) {
            out.push_back(original);
            return;
        }

        // We need to split the intermediate pattern into two new patterns:
        // 1. A pattern containing all objects that do not end at our end time.
        // 2. A pattern containing all objects that end at our end time. This will be used for
        // further pattern generation.
        Pattern intermediate;
        Pattern end_timepattern;

        for (size_t i = 0; i < original.hit_objects.size(); i++) {
            const ManiaHitObject& obj = original.hit_objects[i];

            if (end_time != static_cast<int>(utils::math::round_half_even(obj.end_time))) {
                intermediate.add(obj);
            } else {
                end_timepattern.add(obj);
            }
        }

        out.push_back(intermediate);
        out.push_back(end_timepattern);
    }
}}} // namespace pppp::mania::patterns
