// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/taiko_beatmap.h"
#include "pppp/mods/mod_setting_value.h"
#include "pppp/taiko/mods/taiko_mod_random.h"
#include "pppp/taiko/mods/taiko_mod_swap.h"
#include "pppp/taiko/taiko_beatmap_converter.h"
#include "pppp/utils/linq/csharp.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include "pppp/utils/sort/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace taiko {
    namespace control_points = ::pppp::beatmaps::control_points;
    namespace linq = ::pppp::utils::linq;
    namespace math = ::pppp::utils::math;
    namespace sort = ::pppp::utils::sort;

    namespace {
        double round_away_from_zero(double value) {
            // manually fold BitDecrement(0.5)
            double sum = value + (value < 0.0 || (value == 0.0 && 1.0 / value < 0.0) ? -0.49999999999999994
                                                                                     : 0.49999999999999994);
            return sum < 0.0 ? std::ceil(sum) : std::floor(sum);
        }

        double closest_positive_snapped_time(double point_time, double point_beat_length, double time,
                                             int beat_divisor) {
            double beat_length = point_beat_length / beat_divisor;
            double beats = (std::max(time, 0.0) - point_time) / beat_length;

            int rounded_beats = static_cast<int>(round_away_from_zero(beats));
            double snapped_time = point_time + rounded_beats * beat_length;

            if (snapped_time >= 0) {
                return snapped_time;
            }
            return snapped_time + beat_length;
        }

        int closest_beat_divisor(const TaikoBeatmap& pb, double time) {
            const control_points::TimingControlPoint* point = pb.info.timing_point_at(time);
            double point_time = point ? point->time : 0.0;
            double point_beat_length = point ? point->beat_length() : 1000.0;

            // getClosestSnappedTime() only returns positive time values.
            // due to that, if time is allowed to be negative, the loop lower below could return bogus results
            // as the "snapped time" will not necessarily be "closest" at that point.
            // compensate for this by moving time by enough beat lengths to go back to the positives.
            if (time < 0) {
                int offset_beats = static_cast<int>(std::ceil(-time / point_beat_length));
                time += offset_beats * point_beat_length;
            }

            int closest_divisor = 0;
            double closest_time = 1.7976931348623157e308;

            for (int i = 0; i < 11; i++) {
                double distance =
                    std::fabs(time - closest_positive_snapped_time(point_time, point_beat_length, time,
                                                                   PREDEFINED_DIVISORS[i]));

                if (utils::definitely_bigger(closest_time, distance)) {
                    closest_divisor = PREDEFINED_DIVISORS[i];
                    closest_time = distance;
                }
            }

            return closest_divisor;
        }

        int snap_between_notes(const TaikoBeatmap& pb, double current_time, double next_time) {
            const control_points::TimingControlPoint* point = pb.info.timing_point_at(current_time);
            double point_time = point ? point->time : 0.0;
            return closest_beat_divisor(pb, point_time + (next_time - current_time));
        }

        void process_pattern(TaikoBeatmap& pb, const std::vector<int>& hits, std::vector<bool>& removed,
                             int base_rhythm, int adjusted_rhythm, int pattern_start_index,
                             int pattern_end_index) {
            // Iterate through the pattern
            for (int j = pattern_start_index; j < pattern_end_index; j++) {
                int index_in_pattern = j - pattern_start_index;

                switch (base_rhythm) {
                // 1/8: Remove every second note
                case 8: {
                    if (index_in_pattern % 2 == 1) {
                        removed[hits[j]] = true;
                    }

                    break;
                }

                // 1/6 and 1/3: Remove every second note and adjust time of every third
                case 6:
                case 3: {
                    if (index_in_pattern % 3 == 1) {
                        removed[hits[j]] = true;
                    } else if (index_in_pattern % 3 == 2) {
                        const control_points::TimingControlPoint* point =
                            pb.info.timing_point_at(pb.objects[hits[j]].time);
                        pb.objects[hits[j]].time =
                            pb.objects[hits[j - 2]].time +
                            (point ? point->beat_length() : control_points::DEFAULT_BEAT_LENGTH) /
                                adjusted_rhythm;
                    }

                    break;
                }

                default: break;
                }
            }
        }

        int compare_start_time(const TaikoHitObject& a, const TaikoHitObject& b) {
            return math::compare_to(a.time, b.time);
        }

        void apply_simplified_rhythm(TaikoBeatmap& pb, bool one_third, bool one_sixth, bool one_eighth) {
            std::vector<int> hits;
            for (size_t i = 0; i < pb.objects.size(); i++) {
                if (pb.objects[i].kind == object::OBJECT_HIT) {
                    hits.push_back(static_cast<int>(i));
                }
            }

            if (hits.empty()) {
                return;
            }

            std::vector<std::pair<int, int> > conversions;

            if (one_eighth) {
                conversions.push_back(std::make_pair(8, 4));
            }
            if (one_sixth) {
                conversions.push_back(std::make_pair(6, 4));
            }
            if (one_third) {
                conversions.push_back(std::make_pair(3, 2));
            }

            std::vector<bool> removed(pb.objects.size(), false);
            bool in_pattern = false;

            for (size_t c = 0; c < conversions.size(); c++) {
                int base_rhythm = conversions[c].first;
                int adjusted_rhythm = conversions[c].second;
                int pattern_start_index = 0;

                for (int i = 1; i < static_cast<int>(hits.size()); i++) {
                    double snap_value =
                        snap_between_notes(pb, pb.objects[hits[i - 1]].time, pb.objects[hits[i]].time);

                    if (in_pattern) {
                        // pattern continues
                        if (snap_value == base_rhythm) {
                            continue;
                        }

                        in_pattern = false;
                        process_pattern(pb, hits, removed, base_rhythm, adjusted_rhythm, pattern_start_index,
                                        i);
                    } else {
                        if (snap_value == base_rhythm) {
                            pattern_start_index = i - 1;
                            in_pattern = true;
                        }
                    }
                }

                // Process the last pattern if we reached the end of the beatmap and are still in a pattern.
                if (in_pattern) {
                    process_pattern(pb, hits, removed, base_rhythm, adjusted_rhythm, pattern_start_index,
                                    static_cast<int>(hits.size()));
                }
            }

            std::vector<TaikoHitObject> kept;
            kept.reserve(pb.objects.size());
            for (size_t i = 0; i < pb.objects.size(); i++) {
                if (!removed[i]) {
                    kept.push_back(pb.objects[i]);
                }
            }
            pb.objects.swap(kept);

            sort::introspective_sort(pb.objects, compare_start_time);
        }

    } // namespace

    ModdedDifficulty modded_difficulty(const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                       size_t mod_count) {
        ModdedDifficulty out;
        out.overall_difficulty = beatmap.difficulty.overall_difficulty;
        out.drain_rate = beatmap.difficulty.drain_rate;
        out.slider_multiplier = beatmap.difficulty.slider_multiplier;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_EZ:
                out.drain_rate *= pppp::mods::EASY_ADJUST_RATIO;

                out.overall_difficulty *= pppp::mods::EASY_ADJUST_RATIO;
                out.slider_multiplier *= EASY_SLIDER_MULTIPLIER;
                break;
            case pppp::mods::MOD_HR:
                out.drain_rate = std::min(out.drain_rate * pppp::mods::HARD_ROCK_ADJUST_RATIO, 10.0f);

                out.overall_difficulty =
                    std::min(out.overall_difficulty * pppp::mods::HARD_ROCK_ADJUST_RATIO, 10.0f);
                out.slider_multiplier *= HARD_ROCK_SLIDER_MULTIPLIER;
                break;
            case pppp::mods::MOD_DA:
                if (mods[i].difficulty_adjust.drain_rate.has_value()) {
                    out.drain_rate =
                        pppp::mods::clamp_to_extended_limits(mods[i].difficulty_adjust.drain_rate.value(), 0,
                                                             10, nonstd::optional<float>(), 11.0f);
                }
                if (mods[i].difficulty_adjust.overall_difficulty.has_value()) {
                    out.overall_difficulty = pppp::mods::clamp_to_extended_limits(
                        mods[i].difficulty_adjust.overall_difficulty.value(), 0, 10,
                        nonstd::optional<float>(), 11.0f);
                }
                if (mods[i].difficulty_adjust.scroll_speed.has_value()) {
                    out.slider_multiplier *= pppp::mods::clamp_to_extended_limits(
                        mods[i].difficulty_adjust.scroll_speed.value(), 0.25f, 4, nonstd::optional<float>(),
                        nonstd::optional<float>());
                }
                break;
            default: break;
            }
        }

        return out;
    }

    TaikoBeatmap::TaikoBeatmap()
        : clock_rate(1.0),
          overall_difficulty(5.0),
          drain_rate(5.0),
          slider_multiplier(1.4),
          is_convert(false) {}

    double beat_length_at(const TaikoBeatmap& pb, double time) {
        const control_points::TimingControlPoint* point = pb.info.timing_point_at(time);
        return point != 0 ? point->beat_length() : control_points::DEFAULT_BEAT_LENGTH;
    }

    double scroll_speed_at(const TaikoBeatmap& pb, double time) {
        const control_points::EffectControlPoint* point = pb.info.effect_point_at(time);
        return point != 0 ? point->scroll_speed() : 1.0;
    }

    Status build(TaikoBeatmap& pb, const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                 size_t mod_count, const beatmaps::ObjectConverted<TaikoHitObject>& object_converted) {
        convert(pb, beatmap, object_converted);

        ModdedDifficulty modded = modded_difficulty(beatmap, mods, mod_count);
        pb.overall_difficulty = modded.overall_difficulty;
        pb.drain_rate = modded.drain_rate;
        pb.slider_multiplier = modded.slider_multiplier;

        std::vector<pppp::mods::Mod> playable_mods(mods, mods + mod_count);
        for (size_t i = 0; i < playable_mods.size(); i++) {
            switch (playable_mods[i].id) {
            case pppp::mods::MOD_RD: mods::apply_random(pb, playable_mods[i].random.seed.value()); break;
            case pppp::mods::MOD_SW: mods::apply_swap(pb); break;
            case pppp::mods::MOD_SR:
                apply_simplified_rhythm(pb, playable_mods[i].simplified_rhythm.one_third_conversion,
                                        playable_mods[i].simplified_rhythm.one_sixth_conversion,
                                        playable_mods[i].simplified_rhythm.one_eighth_conversion);
                break;
            case pppp::mods::MOD_WU:
            case pppp::mods::MOD_WD: {
                std::vector<double> end_times;
                for (size_t o = 0; o < pb.objects.size(); o++) {
                    end_times.push_back(pb.objects[o].time + pb.objects[o].duration);
                }
                pppp::mods::mod_time_ramp_apply_to_beatmap(
                    &playable_mods[i], !pb.objects.empty(), pb.objects.empty() ? 0.0 : pb.objects[0].time,
                    end_times.empty() ? 0.0 : linq::from(end_times).max());
                break;
            }
            default: break;
            }
        }

        pb.clock_rate = pppp::mods::mod_calculate_rate(playable_mods.empty() ? 0 : &playable_mods[0],
                                                       playable_mods.size());
        return StatusCode::OK;
    }

    int max_combo(const TaikoBeatmap& pb) {
        int combo = 0;
        for (size_t i = 0; i < pb.objects.size(); i++) {
            if (pb.objects[i].kind == object::OBJECT_HIT) {
                combo++;
            }
        }
        return combo;
    }
}} // namespace pppp::taiko
