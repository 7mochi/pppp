// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/patterns/hit_circle_pattern_generator.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace mania { namespace patterns {
    HitCirclePatternGenerator::HitCirclePatternGenerator(
        const pppp::beatmaps::control_points::ControlPointInfo& info_in,
        const pppp::beatmaps::Beatmap& beatmap_in, const SourceObject& hit_object_in,
        const Pattern& previous_pattern_in, int total_columns_in, pppp::utils::LegacyRandom& random_in,
        double previous_time, pppp::utils::Vector2 previous_position, double density, int last_stair)
        : LegacyPatternGenerator(info_in, beatmap_in, hit_object_in, previous_pattern_in, total_columns_in,
                                 random_in),
          stair_type(last_stair),
          convert_type(PATTERN_NONE) {
        const pppp::beatmaps::control_points::TimingControlPoint* tp =
            info.timing_point_at(hit_object.start_time);
        const double beat_length =
            tp != 0 ? tp->beat_length : pppp::beatmaps::control_points::DEFAULT_BEAT_LENGTH;
        const pppp::beatmaps::control_points::EffectControlPoint* ep =
            info.effect_point_at(hit_object.start_time);
        const bool kiai = ep != 0 && ep->kiai_mode;

        const float dx = static_cast<float>(utils::f32(hit_object.position.x - previous_position.x));
        const float dy = static_cast<float>(utils::f32(hit_object.position.y - previous_position.y));
        const float position_separation =
            static_cast<float>(utils::f32(std::sqrt(utils::f32(utils::f32(dx * dx) + utils::f32(dy * dy)))));
        const double time_separation = hit_object.start_time - previous_time;

        if (time_separation <= 80) {
            // More than 187 BPM
            convert_type |= PATTERN_FORCE_NOT_STACK | PATTERN_KEEP_SINGLE;
        } else if (time_separation <= 95) {
            // More than 157 BPM
            convert_type |= PATTERN_FORCE_NOT_STACK | PATTERN_KEEP_SINGLE | last_stair;
        } else if (time_separation <= 105) {
            // More than 140 BPM
            convert_type |= PATTERN_FORCE_NOT_STACK | PATTERN_LOW_PROBABILITY;
        } else if (time_separation <= 125) {
            // More than 120 BPM
            convert_type |= PATTERN_FORCE_NOT_STACK;
        } else if (time_separation <= 135 && position_separation < 20) {
            // More than 111 BPM stream
            convert_type |= PATTERN_CYCLE | PATTERN_KEEP_SINGLE;
        } else if (time_separation <= 150 && position_separation < 20) {
            // More than 100 BPM stream
            convert_type |= PATTERN_FORCE_STACK | PATTERN_LOW_PROBABILITY;
        } else if (position_separation < 20 && density >= beat_length / 2.5) {
            // Low density stream
            convert_type |= PATTERN_REVERSE | PATTERN_LOW_PROBABILITY;
        } else if (density < beat_length / 2.5 || kiai) {
            // High density
        } else {
            convert_type |= PATTERN_LOW_PROBABILITY;
        }

        if (!has_flag(convert_type, PATTERN_KEEP_SINGLE)) {
            if (has_sample(hit_object.hitsound, HIT_FINISH) && total_columns != 8) {
                convert_type |= PATTERN_MIRROR;
            } else if (has_sample(hit_object.hitsound, HIT_CLAP)) {
                convert_type |= PATTERN_GATHERED;
            }
        }
    }

    void HitCirclePatternGenerator::add_to_pattern(Pattern& pattern, int column) const {
        pattern.add(make_note(column, hit_object.start_time));
    }

    Pattern HitCirclePatternGenerator::generate_random_notes(int note_count) {
        Pattern pattern;

        const bool allow_stacking = !has_flag(convert_type, PATTERN_FORCE_NOT_STACK);

        if (!allow_stacking) {
            note_count =
                std::min(note_count, total_columns - random_start - previous_pattern.column_with_objects);
        }

        const bool gathered = has_flag(convert_type, PATTERN_GATHERED);
        int next_column = get_column(static_cast<float>(hit_object.position.x), true);

        for (int i = 0; i < note_count; i++) {
            const nonstd::optional<int> found =
                allow_stacking
                    ? find_available_column(next_column, nonstd::optional<int>(), nonstd::optional<int>(),
                                            gathered, -1, &pattern, 0)
                    : find_available_column(next_column, nonstd::optional<int>(), nonstd::optional<int>(),
                                            gathered, -1, &pattern, &previous_pattern);
            if (!found.has_value()) {
                break;
            }
            next_column = found.value();
            add_to_pattern(pattern, next_column);
        }

        return pattern;
    }

    bool HitCirclePatternGenerator::has_special_column() const {
        return has_sample(hit_object.hitsound, HIT_CLAP) && has_sample(hit_object.hitsound, HIT_FINISH);
    }

    int HitCirclePatternGenerator::get_random_note_count(double p2, double p3, double p4, double p5) {
        switch (total_columns) {
        case 2:
            p2 = 0;
            p3 = 0;
            p4 = 0;
            p5 = 0;
            break;
        case 3:
            p2 = std::min(p2, 0.1);
            p3 = 0;
            p4 = 0;
            p5 = 0;
            break;
        case 4:
            p2 = std::min(p2, 0.23);
            p3 = std::min(p3, 0.04);
            p4 = 0;
            p5 = 0;
            break;
        case 5:
            p3 = std::min(p3, 0.15);
            p4 = std::min(p4, 0.03);
            p5 = 0;
            break;
        default: break;
        }

        if (has_sample(hit_object.hitsound, HIT_CLAP)) {
            p2 = 1;
        }

        return LegacyPatternGenerator::get_random_note_count(p2, p3, p4, p5, 0);
    }

    int HitCirclePatternGenerator::get_random_note_count_mirrored(double centre_probability, double p2,
                                                                  double p3, bool* add_to_centre) {
        switch (total_columns) {
        case 2:
            centre_probability = 0;
            p2 = 0;
            p3 = 0;
            break;
        case 3:
            centre_probability = std::min(centre_probability, 0.03);
            p2 = 0;
            p3 = 0;
            break;
        case 4:
            centre_probability = 0;

            // Stable requires rngValue > x, which is an inverse-probability. Lazer uses true
            // probability (1 - x). But multiplying this value by 2 (stable) is not the same
            // operation as dividing it by 2 (lazer), so it needs to be converted to from a
            // probability and then back after the multiplication.
            p2 = 1 - std::max((1 - p2) * 2, 0.8);
            p3 = 0;
            break;
        case 5:
            centre_probability = std::min(centre_probability, 0.03);
            p3 = 0;
            break;
        case 6:
            centre_probability = 0;

            // Stable requires rngValue > x, which is an inverse-probability. Lazer uses true
            // probability (1 - x). But multiplying this value by 2 (stable) is not the same
            // operation as dividing it by 2 (lazer), so it needs to be converted to from a
            // probability and then back after the multiplication.
            p2 = 1 - std::max((1 - p2) * 2, 0.5);
            p3 = 1 - std::max((1 - p3) * 2, 0.85);
            break;
        default: break;
        }

        // The stable values were allowed to exceed 1, which indicate <0% probability. These values
        // needs to be clamped otherwise the note count would be drawn from an invalid range.
        p2 = utils::math::clamp(p2, 0, 1);
        p3 = utils::math::clamp(p3, 0, 1);

        const double centre_val = random.next_double();
        const int note_count = LegacyPatternGenerator::get_random_note_count(p2, p3, 0, 0, 0);

        *add_to_centre = total_columns % 2 != 0 && note_count != 3 && centre_val > 1 - centre_probability;
        return note_count;
    }

    Pattern HitCirclePatternGenerator::generate_random_pattern(double p2, double p3, double p4, double p5) {
        Pattern pattern;
        pattern.add(generate_random_notes(get_random_note_count(p2, p3, p4, p5)));

        if (random_start > 0 && has_special_column()) {
            add_to_pattern(pattern, 0);
        }
        return pattern;
    }

    Pattern HitCirclePatternGenerator::generate_random_pattern_with_mirrored(double centre_probability,
                                                                             double p2, double p3) {
        if (has_flag(convert_type, PATTERN_FORCE_NOT_STACK)) {
            return generate_random_pattern(0.5 + p2 / 2, p2, (p2 + p3) / 2, p3);
        }

        Pattern pattern;
        bool add_to_centre = false;
        const int note_count = get_random_note_count_mirrored(centre_probability, p2, p3, &add_to_centre);

        const int column_limit = (total_columns % 2 == 0 ? total_columns : total_columns - 1) / 2;
        int next_column = get_random_column(nonstd::optional<int>(), column_limit);

        for (int i = 0; i < note_count; i++) {
            const nonstd::optional<int> found = find_available_column(next_column, nonstd::optional<int>(),
                                                                      column_limit, false, -1, &pattern, 0);
            if (!found.has_value()) {
                break;
            }
            next_column = found.value();

            // Add normal note
            add_to_pattern(pattern, next_column);
            // Add mirrored note
            add_to_pattern(pattern, random_start + total_columns - next_column - 1);
        }

        if (add_to_centre) {
            add_to_pattern(pattern, total_columns / 2);
        }
        if (random_start > 0 && has_special_column()) {
            add_to_pattern(pattern, 0);
        }

        return pattern;
    }

    Pattern HitCirclePatternGenerator::generate_core() {
        Pattern pattern;

        if (total_columns == 1) {
            add_to_pattern(pattern, 0);
            return pattern;
        }

        const int last_column =
            previous_pattern.hit_objects.empty() ? 0 : previous_pattern.hit_objects.front().column;

        if (has_flag(convert_type, PATTERN_REVERSE) && !previous_pattern.hit_objects.empty()) {
            // Generate a new pattern by copying the last hit objects in reverse-column order
            for (int i = random_start; i < total_columns; i++) {
                if (previous_pattern.column_has_object(i)) {
                    add_to_pattern(pattern, random_start + total_columns - i - 1);
                }
            }
            return pattern;
        }

        if (has_flag(convert_type, PATTERN_CYCLE) &&
            previous_pattern.hit_objects.size() == 1
            // If we convert to 7K + 1, let's not overload the special key
            && (total_columns != 8 || last_column != 0)
            // Make sure the last column was not the centre column
            && (total_columns % 2 == 0 || last_column != total_columns / 2)) {
            // Generate a new pattern by cycling backwards (similar to Reverse but for only one hit
            // object)
            add_to_pattern(pattern, random_start + total_columns - last_column - 1);
            return pattern;
        }

        if (has_flag(convert_type, PATTERN_FORCE_STACK) && !previous_pattern.hit_objects.empty()) {
            // Generate a new pattern by placing on the already filled columns
            for (int i = random_start; i < total_columns; i++) {
                if (previous_pattern.column_has_object(i)) {
                    add_to_pattern(pattern, i);
                }
            }
            return pattern;
        }

        if (previous_pattern.hit_objects.size() == 1) {
            if (has_flag(convert_type, PATTERN_STAIR)) {
                // Generate a new pattern by placing on the next column, cycling back to the start
                // if there is no "next"
                int target_column = last_column + 1;
                if (target_column == total_columns) {
                    target_column = random_start;
                }
                add_to_pattern(pattern, target_column);
                return pattern;
            }

            if (has_flag(convert_type, PATTERN_REVERSE_STAIR)) {
                // Generate a new pattern by placing on the previous column, cycling back to the end
                // if there is no "previous"
                int target_column = last_column - 1;
                if (target_column == random_start - 1) {
                    target_column = total_columns - 1;
                }
                add_to_pattern(pattern, target_column);
                return pattern;
            }
        }

        if (has_flag(convert_type, PATTERN_KEEP_SINGLE)) {
            return generate_random_notes(1);
        }

        if (has_flag(convert_type, PATTERN_MIRROR)) {
            if (conversion_difficulty > 6.5) {
                return generate_random_pattern_with_mirrored(0.12, 0.38, 0.12);
            }
            if (conversion_difficulty > 4) {
                return generate_random_pattern_with_mirrored(0.12, 0.17, 0);
            }
            return generate_random_pattern_with_mirrored(0.12, 0, 0);
        }

        if (conversion_difficulty > 6.5) {
            return has_flag(convert_type, PATTERN_LOW_PROBABILITY) ? generate_random_pattern(0.78, 0.42, 0, 0)
                                                                   : generate_random_pattern(1, 0.62, 0, 0);
        }
        if (conversion_difficulty > 4) {
            return has_flag(convert_type, PATTERN_LOW_PROBABILITY)
                       ? generate_random_pattern(0.35, 0.08, 0, 0)
                       : generate_random_pattern(0.52, 0.15, 0, 0);
        }
        if (conversion_difficulty > 2) {
            return has_flag(convert_type, PATTERN_LOW_PROBABILITY) ? generate_random_pattern(0.18, 0, 0, 0)
                                                                   : generate_random_pattern(0.45, 0, 0, 0);
        }

        return generate_random_pattern(0, 0, 0, 0);
    }

    void HitCirclePatternGenerator::generate(std::vector<Pattern>& out) {
        const Pattern pattern = generate_core();

        for (size_t i = 0; i < pattern.hit_objects.size(); i++) {
            const int column = pattern.hit_objects[i].column;

            if (has_flag(convert_type, PATTERN_STAIR) && column == total_columns - 1) {
                stair_type = PATTERN_REVERSE_STAIR;
            }
            if (has_flag(convert_type, PATTERN_REVERSE_STAIR) && column == random_start) {
                stair_type = PATTERN_STAIR;
            }
        }

        out.push_back(pattern);
    }
}}} // namespace pppp::mania::patterns
