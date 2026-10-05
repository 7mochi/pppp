// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/fruits/catch_beatmap.h"
#include "pppp/fruits/difficulty/preprocessing/catch_difficulty_hit_object.h"
#include "pppp/fruits/difficulty/skills/movement.h"
#include <cmath>
#include <vector>

namespace pppp { namespace fruits { namespace difficulty {
    namespace {
        void flatten(std::vector<object::CatchHitObject*>& out, object::CatchHitObject& object) {
            out.push_back(&object);
            for (size_t i = 0; i < object.nested.size(); i++) {
                flatten(out, object.nested[i]);
            }
        }

        void create_difficulty_attributes(CatchDifficultyAttributes& out, const CatchBeatmap& pb,
                                          skills::Movement& movement) {
            out.star_rating = std::sqrt(movement.difficulty_value()) * DIFFICULTY_MULTIPLIER;
            out.max_combo = max_combo(pb);
        }
    } // namespace

    void preprocessing::create_hit_objects(std::vector<preprocessing::CatchDifficultyHitObject>& out,
                                           std::vector<preprocessing::DifficultyHitObject*>& object_ptrs,
                                           CatchBeatmap& pb) {
        out.clear();
        object_ptrs.clear();

        std::vector<object::CatchHitObject*> palpable;

        // In 2B beatmaps, it is possible that a normal Fruit is placed in the middle of a JuiceStream.
        palpable_objects(palpable, pb);
        if (palpable.size() < 2) {
            return;
        }

        float half_catcher_width =
            static_cast<float>(object::CatchHitObject::calculate_catch_width(pb.circle_size)) * 0.5f;

        // For circle sizes above 5.5, reduce the catcher width further to simulate imperfect gameplay.
        half_catcher_width *= 1 - (std::max(0.0f, static_cast<float>(pb.circle_size) - 5.5f) * 0.0625f);

        out.reserve(palpable.size() - 1);
        object_ptrs.reserve(palpable.size() - 1);

        for (size_t i = 1; i < palpable.size(); i++) {
            const object::CatchHitObject& object = *palpable[i];
            const object::CatchHitObject& last = *palpable[i - 1];

            out.push_back(preprocessing::CatchDifficultyHitObject(
                object, last, pb.clock_rate, half_catcher_width, object_ptrs, static_cast<int>(out.size())));
            object_ptrs.push_back(&out.back());
        }
    }

    Status calculate_difficulty(CatchDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        CatchBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        out = CatchDifficultyAttributes();

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        std::vector<preprocessing::CatchDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        skills::Movement movement(mods, mod_count);
        for (size_t i = 0; i < dho_list.size(); i++) {
            movement.process(dho_list[i]);
        }

        create_difficulty_attributes(out, pb, movement);

        return StatusCode::OK;
    }

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<CatchDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count) {
        times.clear();
        attributes.clear();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        CatchBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        skills::Movement movement(mods, mod_count);

        CatchBeatmap progressive = pb;
        progressive.objects.clear();
        progressive.all_objects.clear();

        std::vector<preprocessing::CatchDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        std::vector<object::CatchHitObject*> palpable;
        palpable_objects(palpable, pb);

        size_t current = 0;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            flatten(progressive.all_objects, pb.objects[i]);

            while (current < dho_list.size() && palpable[current + 1]->end_time <= pb.objects[i].end_time) {
                movement.process(dho_list[current]);

                current++;
            }

            CatchDifficultyAttributes step;
            create_difficulty_attributes(step, progressive, movement);
            times.push_back(pb.objects[i].end_time);
            attributes.push_back(step);
        }

        return StatusCode::OK;
    }

    Status calculate_strains(CatchStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count) {
        out = CatchStrains();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        CatchBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        const double section_length = 750;

        std::vector<preprocessing::CatchDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        skills::Movement movement(mods, mod_count);
        if (!pb.objects.empty()) {
            for (size_t i = 0; i < dho_list.size(); i++) {
                movement.process(dho_list[i]);
            }
        }

        if (!dho_list.empty()) {
            out.start_time =
                (std::ceil(dho_list[0].start_time / section_length) * section_length - section_length) *
                pb.clock_rate;
        }
        out.section_length = section_length * pb.clock_rate;

        movement.get_current_strain_peaks(out.movement);

        return StatusCode::OK;
    }
}}} // namespace pppp::fruits::difficulty
