// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/taiko_beatmap.h"
#include "pppp/taiko/mods/taiko_mod_random.h"
#include "pppp/taiko/mods/taiko_mod_swap.h"
#include "pppp/taiko/taiko_beatmap_converter.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace taiko {
    namespace {
        double round_away_from_zero(double x) { return x >= 0.0 ? std::floor(x + 0.5) : std::ceil(x - 0.5); }

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
            const pppp::beatmaps::control_points::TimingControlPoint* point = pb.info.timing_point_at(time);
            double point_time = point ? point->time : 0.0;
            double point_beat_length = point ? point->beat_length : 1000.0;

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

                if (closest_time - 1e-7 > distance) {
                    closest_divisor = PREDEFINED_DIVISORS[i];
                    closest_time = distance;
                }
            }

            return closest_divisor;
        }

        int snap_between_notes(const TaikoBeatmap& pb, double current_time, double next_time) {
            const pppp::beatmaps::control_points::TimingControlPoint* point =
                pb.info.timing_point_at(current_time);
            double point_time = point ? point->time : 0.0;
            return closest_beat_divisor(pb, point_time + (next_time - current_time));
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

            int conversions[3][2];
            int conversion_count = 0;
            if (one_eighth) {
                conversions[conversion_count][0] = 8;
                conversions[conversion_count][1] = 4;
                conversion_count++;
            }
            if (one_sixth) {
                conversions[conversion_count][0] = 6;
                conversions[conversion_count][1] = 4;
                conversion_count++;
            }
            if (one_third) {
                conversions[conversion_count][0] = 3;
                conversions[conversion_count][1] = 2;
                conversion_count++;
            }

            std::vector<bool> removed(pb.objects.size(), false);
            bool in_pattern = false;

            for (int c = 0; c < conversion_count; c++) {
                int base_rhythm = conversions[c][0];
                int adjusted_rhythm = conversions[c][1];
                int pattern_start = 0;

                for (size_t i = 1; i <= hits.size(); i++) {
                    int pattern_end = -1;

                    if (i < hits.size()) {
                        int snap =
                            snap_between_notes(pb, pb.objects[hits[i - 1]].time, pb.objects[hits[i]].time);
                        if (in_pattern) {
                            // pattern continues
                            if (snap == base_rhythm) {
                                continue;
                            }
                            in_pattern = false;
                            pattern_end = static_cast<int>(i);
                        } else {
                            if (snap == base_rhythm) {
                                pattern_start = static_cast<int>(i) - 1;
                                in_pattern = true;
                            }
                            continue;
                        }
                    } else {
                        // Process the last pattern if we reached the end of the beatmap and are still in a
                        // pattern.
                        if (!in_pattern) {
                            break;
                        }
                        pattern_end = static_cast<int>(hits.size());
                    }

                    // Iterate through the pattern
                    for (int j = pattern_start; j < pattern_end; j++) {
                        int index_in_pattern = j - pattern_start;

                        // 1/8: Remove every second note
                        if (base_rhythm == 8) {
                            if (index_in_pattern % 2 == 1) {
                                removed[hits[j]] = true;
                            }
                        } else {
                            // 1/6 and 1/3: Remove every second note and adjust time of every third
                            if (index_in_pattern % 3 == 1) {
                                removed[hits[j]] = true;
                            } else if (index_in_pattern % 3 == 2) {
                                const pppp::beatmaps::control_points::TimingControlPoint* point =
                                    pb.info.timing_point_at(pb.objects[hits[j]].time);
                                double beat_length = point ? point->beat_length : 1000.0;
                                pb.objects[hits[j]].time =
                                    pb.objects[hits[j - 2]].time + beat_length / adjusted_rhythm;
                            }
                        }
                    }
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

            std::stable_sort(pb.objects.begin(), pb.objects.end(), hit_object_less);
        }

    } // namespace

    bool hit_object_less(const TaikoHitObject& a, const TaikoHitObject& b) { return a.time < b.time; }

    ModdedDifficulty modded_difficulty(const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                       size_t mod_count) {
        ModdedDifficulty out;
        out.overall_difficulty = beatmap.difficulty.overall_difficulty;
        out.drain_rate = beatmap.difficulty.drain_rate;
        out.slider_multiplier = beatmap.difficulty.slider_multiplier;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_EZ:
                out.drain_rate = utils::f32(out.drain_rate * pppp::mods::EASY_ADJUST_RATIO);
                out.overall_difficulty = utils::f32(out.overall_difficulty * pppp::mods::EASY_ADJUST_RATIO);
                out.slider_multiplier *= EASY_SLIDER_MULTIPLIER;
                break;
            case pppp::mods::MOD_HR: {
                double ratio = pppp::mods::HARD_ROCK_ADJUST_RATIO;
                out.drain_rate = std::min(utils::f32(out.drain_rate * ratio), 10.0);
                out.overall_difficulty = std::min(utils::f32(out.overall_difficulty * ratio), 10.0);
                out.slider_multiplier *= HARD_ROCK_SLIDER_MULTIPLIER;
                break;
            }
            case pppp::mods::MOD_DA:
                if (mods[i].difficulty_adjust.drain_rate.value() >= 0.0) {
                    out.drain_rate = mods[i].difficulty_adjust.drain_rate.value();
                }
                if (mods[i].difficulty_adjust.overall_difficulty.value() >= 0.0) {
                    out.overall_difficulty = mods[i].difficulty_adjust.overall_difficulty.value();
                }
                if (mods[i].difficulty_adjust.scroll_speed.value() >= 0.0) {
                    out.slider_multiplier *= mods[i].difficulty_adjust.scroll_speed.value();
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
        const pppp::beatmaps::control_points::TimingControlPoint* point = pb.info.timing_point_at(time);
        return point != 0 ? point->beat_length : pppp::beatmaps::control_points::DEFAULT_BEAT_LENGTH;
    }

    double scroll_speed_at(const TaikoBeatmap& pb, double time) {
        const pppp::beatmaps::control_points::EffectControlPoint* point = pb.info.effect_point_at(time);
        return point != 0 ? point->clamped_scroll_speed() : 1.0;
    }

    Result::Value build(TaikoBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                        size_t mod_count,
                        const pppp::beatmaps::ObjectConverted<TaikoHitObject>& object_converted) {
        convert(pb, beatmap, object_converted);
        pb.clock_rate = pppp::mods::mod_calculate_rate(mods, mod_count);

        ModdedDifficulty modded = modded_difficulty(beatmap, mods, mod_count);
        pb.overall_difficulty = modded.overall_difficulty;
        pb.drain_rate = modded.drain_rate;
        pb.slider_multiplier = modded.slider_multiplier;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_RD: mods::apply_random(pb, mods[i].random.seed.value()); break;
            case pppp::mods::MOD_SW: mods::apply_swap(pb); break;
            case pppp::mods::MOD_SR:
                apply_simplified_rhythm(pb, mods[i].simplified_rhythm.one_third_conversion,
                                        mods[i].simplified_rhythm.one_sixth_conversion,
                                        mods[i].simplified_rhythm.one_eighth_conversion);
                break;
            default: break;
            }
        }

        return Result::OK;
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
