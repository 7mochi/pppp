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
#include "pppp/utils/linq/csharp.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace pppp { namespace osu { namespace difficulty {
    namespace linq = ::pppp::utils::linq;
    namespace math = ::pppp::utils::math;

    namespace {
        double calculate_aim_difficulty_rating(double difficulty_value) {
            return utils::pow(difficulty_value, 0.63) * 0.02275;
        }

        double calculate_difficulty_rating(double difficulty_value) {
            return std::sqrt(difficulty_value) * 0.0675;
        }

        double calculate_star_rating(double base_performance) {
            return math::cbrt(base_performance * PERFORMANCE_BASE_MULTIPLIER);
        }

        bool is_slider(const object::OsuHitObject& hit_object) { return hit_object.slider >= 0; }

        bool is_spinner(const object::OsuHitObject& hit_object) {
            return hit_object.slider < 0 && (hit_object.type & 8) != 0;
        }

        bool is_hit_circle(const object::OsuHitObject& hit_object) {
            return hit_object.slider < 0 && (hit_object.type & 8) == 0;
        }

        int count_large_ticks(const OsuBeatmap& pb) {
            int large_ticks = 0;
            for (size_t i = 0; i < pb.objects.size(); i++) {
                const object::OsuHitObject& ho = pb.objects[i];
                if (ho.slider >= 0) {
                    const std::vector<beatmaps::SliderEventDescriptor>& events = pb.sliders[ho.slider].events;
                    for (size_t j = 0; j < events.size(); j++) {
                        if (events[j].type == beatmaps::SLIDER_EVENT_TICK ||
                            events[j].type == beatmaps::SLIDER_EVENT_REPEAT) {
                            large_ticks++;
                        }
                    }
                }
            }
            return large_ticks;
        }

        void create_difficulty_attributes(OsuDifficultyAttributes& out, const beatmaps::Beatmap& beatmap,
                                          const OsuBeatmap& pb, skills::Aim& aim_with_sliders,
                                          skills::Aim& aim_no_sliders, skills::Speed& speed,
                                          skills::Reading& reading, skills::Flashlight* flashlight) {
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
                std::max(1.0,
                         aim_no_sliders_difficult_strain_count - aim_no_sliders_top_weighted_slider_count);
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
            if (flashlight) {
                flashlight_rating = calculate_difficulty_rating(flashlight->difficulty_value());
            }

            double base_aim_perf = difficulty_to_performance(aim_rating);
            double base_speed_perf = skills::HarmonicSkill::difficulty_to_performance(speed_rating);
            double base_reading_perf = skills::HarmonicSkill::difficulty_to_performance(reading_rating);
            double base_flashlight_perf = skills::Flashlight::difficulty_to_performance(flashlight_rating);
            double base_cognition_perf = sum_cognition_difficulty(base_reading_perf, base_flashlight_perf);

            double base_performance = utils::norm(1.1, base_aim_perf, base_speed_perf, base_cognition_perf);
            double star_rating = calculate_star_rating(base_performance);

            int hit_circle_count = linq::from(pb.objects).count(linq::bind(&is_hit_circle, linq::_1));
            int slider_count = linq::from(pb.objects).count(linq::bind(&is_slider, linq::_1));
            int spinner_count = linq::from(pb.objects).count(linq::bind(&is_spinner, linq::_1));
            int large_tick_count = count_large_ticks(pb);

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
        }

        const double STRAIN_SECTION_LENGTH = 400;

        void build_strain_series(std::vector<double>& out, const std::vector<double>& values,
                                 const std::vector<preprocessing::OsuDifficultyHitObject>& objects,
                                 double start_time) {
            out.clear();
            if (objects.empty() || values.empty()) {
                return;
            }

            int length = std::max(1, static_cast<int>(std::floor((objects.back().start_time - start_time) /
                                                                 STRAIN_SECTION_LENGTH)) +
                                         1);
            out.assign(static_cast<size_t>(length), 0.0);

            for (size_t i = 0; i < objects.size(); i++) {
                int section = static_cast<int>(
                    std::floor((objects[i].start_time - start_time) / STRAIN_SECTION_LENGTH));

                if (section >= 0 && section < length) {
                    out[static_cast<size_t>(section)] =
                        std::max(out[static_cast<size_t>(section)], values[i]);
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
                utils::Vector2 mod_off = {off, off};
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

    double sum_cognition_difficulty(double reading, double flashlight) {
        if (reading <= 0) {
            return flashlight;
        }

        if (flashlight <= 0) {
            return reading;
        }

        // Nerf flashlight value in cognition sum when reading is greater than flashlight
        return utils::norm(PERFORMANCE_NORM_EXPONENT, reading,
                           flashlight * math::clamp(flashlight / reading, 0.25, 1.0));
    }

    Status calculate_difficulty(OsuDifficultyAttributes& out, const beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        OsuBeatmap pb;
        Status rc = build(pb, beatmap, mods, mod_count);
        if (!rc.ok()) {
            return rc;
        }

        out = OsuDifficultyAttributes();

        if (pb.objects.size() < 2) {
            if (pb.objects.empty()) {
                out.slider_factor = 0.0;
                return StatusCode::OK;
            }
            out.hit_circle_count = linq::from(pb.objects).count(linq::bind(&is_hit_circle, linq::_1));
            out.slider_count = linq::from(pb.objects).count(linq::bind(&is_slider, linq::_1));
            out.spinner_count = linq::from(pb.objects).count(linq::bind(&is_spinner, linq::_1));
            out.large_tick_count = count_large_ticks(pb);
            out.max_combo = max_combo(pb);
            out.nested_score_per_object =
                calculate_nested_score_per_object(pb, static_cast<int>(pb.objects.size()));
            out.legacy_score_base_multiplier = peppy_stars(beatmap);
            out.maximum_legacy_combo_score = simulate(beatmap, pb).combo_score;
            return StatusCode::OK;
        }

        std::vector<preprocessing::OsuDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        if (dho_list.empty()) {
            return StatusCode::OK;
        }

        bool has_flashlight =
            linq::from(mods, mod_count).any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_FL);

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

        create_difficulty_attributes(out, beatmap, pb, aim_with_sliders, aim_no_sliders, speed, reading,
                                     has_flashlight ? &flashlight : 0);

        return StatusCode::OK;
    }

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<OsuDifficultyAttributes>& attributes,
                                      const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count) {
        times.clear();
        attributes.clear();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        OsuBeatmap pb;
        Status rc = build(pb, beatmap, mods, mod_count);
        if (!rc.ok()) {
            return rc;
        }

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        bool has_flashlight =
            linq::from(mods, mod_count).any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_FL);

        skills::Aim aim_with_sliders(mods, mod_count, true);
        skills::Aim aim_no_sliders(mods, mod_count, false);
        skills::Speed speed(mods, mod_count);
        skills::Reading reading(mods, mod_count);
        skills::Flashlight flashlight(mods, mod_count, static_cast<int>(pb.objects.size()));

        OsuBeatmap progressive = pb;
        progressive.objects.clear();

        std::vector<preprocessing::OsuDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        size_t current = 0;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            progressive.objects.push_back(pb.objects[i]);

            while (current < dho_list.size() && pb.objects[current + 1].end_time <= pb.objects[i].end_time) {
                aim_with_sliders.process(dho_list[current]);
                aim_no_sliders.process(dho_list[current]);
                speed.process(dho_list[current]);
                reading.process(dho_list[current]);
                if (has_flashlight) {
                    flashlight.process(dho_list[current]);
                }

                current++;
            }

            OsuDifficultyAttributes step;
            create_difficulty_attributes(step, beatmap, progressive, aim_with_sliders, aim_no_sliders, speed,
                                         reading, has_flashlight ? &flashlight : 0);
            times.push_back(pb.objects[i].end_time);
            attributes.push_back(step);
        }

        return StatusCode::OK;
    }

    Status calculate_strains(OsuStrains& out, const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                             size_t mod_count) {
        out = OsuStrains();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        OsuBeatmap pb;
        Status rc = build(pb, beatmap, mods, mod_count);
        if (!rc.ok()) {
            return rc;
        }

        out.section_length = STRAIN_SECTION_LENGTH * pb.clock_rate;

        if (pb.objects.size() < 2) {
            return StatusCode::OK;
        }

        std::vector<preprocessing::OsuDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        if (dho_list.empty()) {
            return StatusCode::OK;
        }

        bool has_flashlight =
            linq::from(mods, mod_count).any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_FL);

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

        double start_time =
            std::ceil(dho_list[0].start_time / STRAIN_SECTION_LENGTH) * STRAIN_SECTION_LENGTH -
            STRAIN_SECTION_LENGTH;
        out.start_time = start_time * pb.clock_rate;

        build_strain_series(out.aim, aim_with_sliders.get_object_difficulties(), dho_list, start_time);
        build_strain_series(out.aim_no_sliders, aim_no_sliders.get_object_difficulties(), dho_list,
                            start_time);
        build_strain_series(out.speed, speed.get_object_difficulties(), dho_list, start_time);
        build_strain_series(out.reading, reading.get_object_difficulties(), dho_list, start_time);
        if (has_flashlight) {
            build_strain_series(out.flashlight, flashlight.get_object_difficulties(), dho_list, start_time);
        }

        return StatusCode::OK;
    }
}}} // namespace pppp::osu::difficulty
