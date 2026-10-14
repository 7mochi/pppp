// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mania_beatmap_converter.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/beatmaps/legacy_hit_object_type.h"
#include "pppp/mania/mods/mania_mod_hold_off.h"
#include "pppp/mania/mods/mania_mod_invert.h"
#include "pppp/mania/mods/mania_mod_mirror.h"
#include "pppp/mania/mods/mania_mod_random.h"
#include "pppp/mania/patterns/hit_circle_pattern_generator.h"
#include "pppp/mania/patterns/pass_through_pattern_generator.h"
#include "pppp/mania/patterns/slider_pattern_generator.h"
#include "pppp/mania/patterns/spinner_pattern_generator.h"
#include "pppp/utils/linq/csharp.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>

namespace pppp { namespace mania {
    namespace control_points = ::pppp::beatmaps::control_points;
    namespace linq = ::pppp::utils::linq;
    namespace math = ::pppp::utils::math;

    namespace {
        bool has_duration(const beatmaps::HitObject& ho) {
            return (ho.type & (beatmaps::slider | beatmaps::spinner | beatmaps::hold)) != 0;
        }

        int end_time_object_count(const beatmaps::Beatmap& beatmap) {
            return linq::from(beatmap.hit_objects).count(linq::bind(&has_duration, linq::_1));
        }

        int get_column_count(const beatmaps::Beatmap& beatmap) {
            const double rounded_circle_size = math::round_half_even(beatmap.difficulty.circle_size);

            if (beatmap.mode == 3) {
                return static_cast<int>(std::max(1.0, rounded_circle_size));
            }

            const double rounded_overall_difficulty =
                math::round_half_even(beatmap.difficulty.overall_difficulty);
            const int total_object_count = static_cast<int>(beatmap.hit_objects.size());
            const int count_slider_or_spinner = end_time_object_count(beatmap);

            if (total_object_count > 0 && count_slider_or_spinner >= 0) {
                // In osu!stable, this division appears as if it happens on floats, but due to release-mode
                // optimisations, it actually ends up happening on doubles.
                const double percent_special_objects =
                    static_cast<double>(count_slider_or_spinner) / total_object_count;

                if (percent_special_objects < 0.2) {
                    return 7;
                }
                if (percent_special_objects < 0.3 || rounded_circle_size >= 5) {
                    return rounded_overall_difficulty > 5 ? 7 : 6;
                }
                if (percent_special_objects > 0.6) {
                    return rounded_overall_difficulty > 4 ? 5 : 4;
                }
            }

            return std::max(4, std::min(static_cast<int>(rounded_overall_difficulty) + 1, 7));
        }

        int random_seed_of(const beatmaps::Beatmap& beatmap) {
            const float drain_and_circle = static_cast<float>(
                utils::f32(beatmap.difficulty.drain_rate + beatmap.difficulty.circle_size));
            const int rounded_drain_and_circle = static_cast<int>(math::round_half_even(drain_and_circle));
            const int scaled_overall = static_cast<int>(beatmap.difficulty.overall_difficulty * 41.2);
            const int rounded_approach =
                static_cast<int>(math::round_half_even(utils::f32(beatmap.difficulty.approach_rate)));

            return rounded_drain_and_circle * 20 + scaled_overall + rounded_approach;
        }

        double convert_slider_duration(const beatmaps::Beatmap& beatmap,
                                       const control_points::ControlPointInfo& info,
                                       const beatmaps::HitObject& ho, const beatmaps::Slider& slider) {
            const control_points::TimingControlPoint* timing_point = info.timing_point_at(ho.start_time);
            const control_points::DifficultyControlPoint* difficulty_point =
                info.difficulty_point_at(ho.start_time);

            double scoring_distance = BASE_SCORING_DISTANCE * beatmap.difficulty.slider_multiplier *
                                      (difficulty_point != 0 ? difficulty_point->slider_velocity() : 1);

            double velocity = scoring_distance / (timing_point != 0 ? timing_point->beat_length()
                                                                    : control_points::DEFAULT_BEAT_LENGTH);

            const std::vector<double>& cumulative_lengths = slider.undecimated_cumulative_lengths.empty()
                                                                ? slider.cumulative_lengths
                                                                : slider.undecimated_cumulative_lengths;
            double distance = cumulative_lengths.empty() ? 0 : cumulative_lengths.back();
            int span_count = distance <= 1e-7 ? 1 : (slider.slides > 0 ? slider.slides : 1);

            return span_count * distance / velocity;
        }

        patterns::SourceObject source_of(const beatmaps::Beatmap& beatmap,
                                         const control_points::ControlPointInfo& info,
                                         const beatmaps::HitObject& ho) {
            patterns::SourceObject src;
            src.start_time = ho.start_time;
            src.duration = has_duration(ho) ? ho.duration : 0;
            if (ho.slider >= 0 && static_cast<size_t>(ho.slider) < beatmap.sliders.size()) {
                src.duration = convert_slider_duration(beatmap, info, ho, beatmap.sliders[ho.slider]);
            }
            src.position = ho.position;
            src.hitsound = ho.hitsound;
            src.has_duration = has_duration(ho);
            src.has_path = ho.slider >= 0;

            if (ho.slider >= 0 && static_cast<size_t>(ho.slider) < beatmap.sliders.size()) {
                const beatmaps::Slider& slider = beatmap.sliders[ho.slider];
                src.slides = slider.slides > 0 ? slider.slides : 1;
                src.expected_distance = slider.expected_length;
                src.node_sounds = &slider.node_sounds;
            }

            return src;
        }
    } // namespace

    ManiaBeatmapConverter::ManiaBeatmapConverter(const beatmaps::Beatmap& beatmap_in,
                                                 const pppp::mods::Mod* mods_in, size_t mod_count_in)
        : beatmap(beatmap_in),
          target_columns(get_column_count(beatmap_in)),
          dual(false),
          is_for_current_ruleset(beatmap_in.mode == 3),
          random(random_seed_of(beatmap_in)),
          object_converted(),
          mods(mods_in),
          mod_count(mod_count_in),
          density(2147483647.0),
          last_time(0.0),
          last_stair(patterns::PATTERN_STAIR) {
        if (is_for_current_ruleset && target_columns > MAX_STAGE_KEYS) {
            // If support for odd key counts above 10 (11 / 13 / 15 / 17K) is ever desired, this code needs to
            // go.
            // For now, it's probably fine, as stable doesn't support them outside of manual .osu edits
            // either.
            target_columns /= 2;
            dual = true;
        }

        // Although this can work, for now let's not allow keymods for mania-specific beatmaps
        for (size_t i = 0; !is_for_current_ruleset && i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_1K: target_columns = 1; break;
            case pppp::mods::MOD_2K: target_columns = 2; break;
            case pppp::mods::MOD_3K: target_columns = 3; break;
            case pppp::mods::MOD_4K: target_columns = 4; break;
            case pppp::mods::MOD_5K: target_columns = 5; break;
            case pppp::mods::MOD_6K: target_columns = 6; break;
            case pppp::mods::MOD_7K: target_columns = 7; break;
            case pppp::mods::MOD_8K: target_columns = 8; break;
            case pppp::mods::MOD_9K: target_columns = 9; break;
            case pppp::mods::MOD_10K: target_columns = 10; break;
            case pppp::mods::MOD_DS: dual = true; break;
            default: break;
            }
        }

        last_position.x = 0.0;
        last_position.y = 0.0;
    }

    void ManiaBeatmapConverter::compute_density(double new_note_time) {
        previous_note_times.push_back(new_note_time);

        if (previous_note_times.size() > MAX_NOTES_FOR_DENSITY) {
            previous_note_times.erase(previous_note_times.begin());
        }
        if (previous_note_times.size() >= 2) {
            density = (previous_note_times.back() - previous_note_times.front()) /
                      static_cast<double>(previous_note_times.size());
        }
    }

    void ManiaBeatmapConverter::record_note(double time, utils::Vector2 position) {
        last_time = time;
        last_position = position;
    }

    void ManiaBeatmapConverter::convert(ManiaBeatmap& pb) {
        pb = ManiaBeatmap();
        pb.is_for_current_ruleset = is_for_current_ruleset;
        pb.target_columns = target_columns;
        pb.dual = dual;

        control_points::ControlPointInfo original;
        original.build(beatmap);
        const control_points::ControlPointInfo info = original.deep_clone();

        const int columns = total_columns();

        std::vector<patterns::Pattern> generated;

        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            const beatmaps::HitObject& ho = beatmap.hit_objects[i];
            const patterns::SourceObject src = source_of(beatmap, info, ho);
            const size_t first = pb.objects.size();

            generated.clear();
            bool keeps_pattern = false;

            if ((ho.type & beatmaps::spinner) != 0) {
                // Note: Some older mania-specific beatmaps can have spinners that are converted rather than
                // passed through.
                //       Newer beatmaps will usually use the "hold" hitobject type below.
                patterns::SpinnerPatternGenerator conversion(info, beatmap, src, last_pattern, columns,
                                                             random);
                conversion.generate(generated);
                utils::Vector2 spinner_position;
                spinner_position.x = 256.0;
                spinner_position.y = 192.0;
                record_note(src.end_time(), spinner_position);
                compute_density(src.end_time());
            } else if ((ho.type & beatmaps::hold) != 0) {
                patterns::PassThroughPatternGenerator conversion(info, beatmap, src, last_pattern, columns,
                                                                 random);
                conversion.generate(generated);
                record_note(src.end_time(), ho.position);
                compute_density(src.end_time());
            } else if (src.has_path) {
                if (is_for_current_ruleset) {
                    patterns::PassThroughPatternGenerator conversion(info, beatmap, src, last_pattern,
                                                                     columns, random);
                    conversion.generate(generated);
                    record_note(src.start_time, ho.position);
                } else {
                    patterns::SliderPatternGenerator conversion(info, beatmap, src, last_pattern, columns,
                                                                random);
                    conversion.generate(generated);
                    keeps_pattern = true;

                    for (int span = 0; span <= conversion.span_count; span++) {
                        const double time = src.start_time + conversion.segment_duration * span;

                        record_note(time, ho.position);
                        compute_density(time);
                    }
                }
            } else {
                if (is_for_current_ruleset) {
                    patterns::PassThroughPatternGenerator conversion(info, beatmap, src, last_pattern,
                                                                     columns, random);
                    conversion.generate(generated);
                    record_note(src.start_time, ho.position);
                } else {
                    // Note: The density is used during the pattern generator constructor, and intentionally
                    // computed first.
                    compute_density(src.start_time);

                    patterns::HitCirclePatternGenerator conversion(info, beatmap, src, last_pattern, columns,
                                                                   random, last_time, last_position, density,
                                                                   last_stair);
                    conversion.generate(generated);
                    last_stair = conversion.stair_type;
                    keeps_pattern = true;

                    record_note(src.start_time, ho.position);
                }
            }

            for (size_t p = 0; p < generated.size(); p++) {
                if (keeps_pattern) {
                    last_pattern = generated[p];
                }
                for (size_t o = 0; o < generated[p].hit_objects.size(); o++) {
                    pb.objects.push_back(generated[p].hit_objects[o]);
                }
            }

            if (object_converted.invoke != 0) {
                object_converted.invoke(i, pb.objects.size() > first ? &pb.objects[first] : 0,
                                        pb.objects.size() - first, object_converted.context);
            }
        }

        std::vector<object::ManiaHitObject> sorted =
            linq::from(pb.objects)
                .order_by(linq::bind(&object::ManiaHitObject::start_time, linq::_1))
                .to_list();
        pb.objects.swap(sorted);

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_IN: mods::apply_invert(pb, info); break;
            case pppp::mods::MOD_HO: mods::apply_hold_off(pb); break;
            default: break;
            }
        }

        std::vector<pppp::mods::Mod> playable_mods(mods, mods + mod_count);
        for (size_t i = 0; i < playable_mods.size(); i++) {
            switch (playable_mods[i].id) {
            case pppp::mods::MOD_MR: mods::apply_mirror(pb); break;
            case pppp::mods::MOD_RD: mods::apply_random(pb, playable_mods[i].random.seed.value()); break;
            case pppp::mods::MOD_WU:
            case pppp::mods::MOD_WD:
                pppp::mods::mod_time_ramp_apply_to_beatmap(
                    &playable_mods[i], !pb.objects.empty(),
                    pb.objects.empty() ? 0.0 : pb.objects[0].start_time,
                    pb.objects.empty()
                        ? 0.0
                        : linq::from(pb.objects).max(linq::bind(&object::get_end_time, linq::_1)));
                break;
            default: break;
            }
        }

        pb.clock_rate = pppp::mods::mod_calculate_rate(playable_mods.empty() ? 0 : &playable_mods[0],
                                                       playable_mods.size());
    }

    Status build(ManiaBeatmap& pb, const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                 size_t mod_count) {
        ManiaBeatmapConverter converter(beatmap, mods, mod_count);
        converter.convert(pb);
        return StatusCode::OK;
    }
}} // namespace pppp::mania
