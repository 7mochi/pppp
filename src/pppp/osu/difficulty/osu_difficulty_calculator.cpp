// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/osu_difficulty_calculator.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/base_performance.h"
#include "pppp/osu/difficulty/legacy_score_simulator.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.h"
#include "pppp/osu/difficulty/skills/aim.h"
#include "pppp/osu/difficulty/skills/flashlight.h"
#include "pppp/osu/difficulty/skills/reading.h"
#include "pppp/osu/difficulty/skills/speed.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace pppp { namespace osu { namespace difficulty {
    namespace {
        double calculate_aim_difficulty_rating(double difficulty_value) {
            return utils::pow(difficulty_value, 0.63) * 0.02275;
        }

        double calculate_difficulty_rating(double difficulty_value) {
            return std::sqrt(difficulty_value) * 0.0675;
        }

        double calculate_star_rating(double base_performance) {
            return utils::math::cbrt(base_performance * PERFORMANCE_BASE_MULTIPLIER);
        }

        /// Counts the objects the difficulty attributes report, by type.
        void count_objects(const OsuBeatmap& pb, int* hit_circles, int* sliders, int* large_ticks,
                           int* spinners) {
            *hit_circles = *sliders = *large_ticks = *spinners = 0;
            for (size_t i = 0; i < pb.objects.size(); i++) {
                const object::OsuHitObject& ho = pb.objects[i];
                if (ho.slider >= 0) {
                    (*sliders)++;
                    const std::vector<pppp::beatmaps::SliderEventDescriptor>& events =
                        pb.sliders[ho.slider].events;
                    for (size_t j = 0; j < events.size(); j++) {
                        if (events[j].type == pppp::beatmaps::SLIDER_EVENT_TICK ||
                            events[j].type == pppp::beatmaps::SLIDER_EVENT_REPEAT) {
                            (*large_ticks)++;
                        }
                    }
                } else if (ho.type & 8) {
                    (*spinners)++;
                } else {
                    (*hit_circles)++;
                }
            }
        }

    } // namespace

    void preprocessing::create_hit_objects(std::vector<preprocessing::OsuDifficultyHitObject>& out,
                                           std::vector<preprocessing::DifficultyHitObject*>& object_ptrs,
                                           const OsuBeatmap& pb) {
        out.clear();
        object_ptrs.clear();

        // The first jump is formed by the first two hitobjects of the map.
        // If the map has less than two OsuHitObjects, nothing will be returned.
        if (pb.objects.size() < 2) {
            return;
        }

        double cs = pb.circle_size;

        preprocessing::PositionCtx pos;
        {
            float scale = static_cast<float>(object::OsuHitObject::calculate_scale_from_cs(cs));
            size_t n = pb.objects.size();
            pos.raw.resize(n);
            pos.offset.resize(n);
            pos.stacked.resize(n);
            for (size_t i = 0; i < n; i++) {
                const object::OsuHitObject& ho = pb.objects[i];
                float off = (ho.type & 8) ? 0.0f : static_cast<float>(ho.stack_height) * scale * -6.4f;
                pppp::utils::Vector2 mod_off = {off, off};
                pos.raw[i] = ho.position;
                pos.offset[i] = mod_off;
                pos.stacked[i] = ho.position + mod_off;
            }
        }

        out.reserve(pb.objects.size() - 1);
        object_ptrs.reserve(pb.objects.size() - 1);

        for (size_t i = 1; i < pb.objects.size(); i++) {
            out.push_back(preprocessing::OsuDifficultyHitObject(pb.objects[i], pb.objects[i - 1],
                                                                pb.clock_rate, object_ptrs,
                                                                static_cast<int>(out.size()), pb, pos));
            object_ptrs.push_back(&out.back());
        }
    }

    Result::Value calculate_difficulty(OsuDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                       const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return Result::INVALID_ARGUMENT;
        }

        OsuBeatmap pb;
        Result::Value rc = build(pb, beatmap, mods, mod_count);
        if (rc != Result::OK) {
            return rc;
        }

        out = OsuDifficultyAttributes();

        if (pb.objects.size() < 2) {
            if (pb.objects.empty()) {
                out.slider_factor = 0.0;
                return Result::OK;
            }
            count_objects(pb, &out.hit_circle_count, &out.slider_count, &out.large_tick_count,
                          &out.spinner_count);
            out.max_combo = max_combo(pb);
            out.nested_score_per_object =
                calculate_nested_score_per_object(pb, static_cast<int>(pb.objects.size()));
            out.legacy_score_base_multiplier = peppy_stars(beatmap);
            out.maximum_legacy_combo_score = simulate(beatmap, pb).combo_score;
            return Result::OK;
        }

        std::vector<preprocessing::OsuDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        if (dho_list.empty()) {
            return Result::OK;
        }

        bool has_flashlight = pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_FL);

        skills::Aim aim_with_sliders(mods, mod_count, true);
        skills::Aim aim_no_sliders(mods, mod_count, false);
        skills::Speed speed(mods, mod_count);
        skills::Reading reading(mods, mod_count);
        skills::Flashlight flashlight(mods, mod_count, static_cast<int>(pb.objects.size()));

        for (size_t i = 0; i < dho_list.size(); i++) {
            aim_with_sliders.process(dho_list[i]);
            aim_no_sliders.process(dho_list[i]);
            speed.process(dho_list[i]);
            reading.process(dho_list[i]);
            if (has_flashlight) {
                flashlight.process(dho_list[i]);
            }
        }

        double aim_dv = aim_with_sliders.difficulty_value();
        double aim_no_sliders_dv = aim_no_sliders.difficulty_value();
        double speed_dv = speed.difficulty_value();
        double reading_dv = reading.difficulty_value();

        double aim_difficult_strain_count = aim_with_sliders.count_top_weighted_strains(aim_dv);
        double speed_difficult_strain_count = speed.count_top_weighted_object_difficulties(speed_dv);
        double reading_difficult_note_count = reading.count_top_weighted_object_difficulties(reading_dv);

        double speed_note_count = speed.relevant_object_count();

        double aim_no_sliders_top_weighted_slider_count =
            aim_no_sliders.count_top_weighted_sliders(aim_no_sliders_dv);
        double aim_no_sliders_difficult_strain_count =
            aim_no_sliders.count_top_weighted_strains(aim_no_sliders_dv);
        double aim_top_weighted_slider_factor =
            aim_no_sliders_top_weighted_slider_count /
            std::max(1.0, aim_no_sliders_difficult_strain_count - aim_no_sliders_top_weighted_slider_count);
        double speed_top_weighted_slider_count = speed.count_top_weighted_sliders(speed_dv);
        double speed_top_weighted_slider_factor =
            speed_top_weighted_slider_count /
            std::max(1.0, speed_difficult_strain_count - speed_top_weighted_slider_count);
        double difficult_sliders = aim_with_sliders.get_difficult_sliders();

        double aim_rating = calculate_aim_difficulty_rating(aim_dv);
        double aim_no_sliders_rating = calculate_aim_difficulty_rating(aim_no_sliders_dv);

        double slider_factor = aim_dv > 0 ? aim_no_sliders_rating / aim_rating : 1.0;

        double speed_rating = calculate_difficulty_rating(speed_dv);
        double reading_rating = calculate_difficulty_rating(reading_dv);

        double flashlight_rating = 0.0;
        double flashlight_dv = 0.0;
        if (has_flashlight) {
            flashlight_dv = flashlight.difficulty_value();
            flashlight_rating = calculate_difficulty_rating(flashlight_dv);
        }

        double base_aim_perf = difficulty_to_performance(aim_rating);
        double base_speed_perf = difficulty_to_performance(speed_rating);
        double base_reading_perf = difficulty_to_performance(reading_rating);
        double base_flashlight_perf = flashlight_difficulty_to_performance(flashlight_rating);
        double base_cognition_perf = sum_cognition_difficulty(base_reading_perf, base_flashlight_perf);

        double base_performance = utils::norm(1.1, base_aim_perf, base_speed_perf, base_cognition_perf);
        double star_rating = calculate_star_rating(base_performance);

        int hit_circle_count = 0, slider_count = 0, large_tick_count = 0, spinner_count = 0;
        count_objects(pb, &hit_circle_count, &slider_count, &large_tick_count, &spinner_count);

        out.star_rating = star_rating;
        out.aim_difficulty = aim_rating;
        out.speed_difficulty = speed_rating;
        out.reading_difficulty = reading_rating;
        out.flashlight_difficulty = flashlight_rating;
        out.slider_factor = slider_factor;
        out.aim_difficult_strain_count = aim_difficult_strain_count;
        out.speed_difficult_strain_count = speed_difficult_strain_count;
        out.reading_difficult_note_count = reading_difficult_note_count;
        out.aim_difficult_slider_count = difficult_sliders;
        out.aim_top_weighted_slider_factor = aim_top_weighted_slider_factor;
        out.speed_top_weighted_slider_factor = speed_top_weighted_slider_factor;
        out.speed_note_count = speed_note_count;
        out.hit_circle_count = hit_circle_count;
        out.slider_count = slider_count;
        out.large_tick_count = large_tick_count;
        out.spinner_count = spinner_count;

        out.max_combo = max_combo(pb);

        out.nested_score_per_object =
            calculate_nested_score_per_object(pb, static_cast<int>(pb.objects.size()));
        out.legacy_score_base_multiplier = peppy_stars(beatmap);
        out.maximum_legacy_combo_score = simulate(beatmap, pb).combo_score;

        return Result::OK;
    }
}}} // namespace pppp::osu::difficulty
