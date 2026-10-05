// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/mania/difficulty/preprocessing/mania_difficulty_hit_object.h"
#include "pppp/mania/difficulty/skills/strain.h"
#include "pppp/mania/mania_beatmap.h"
#include "pppp/utils/sort/osu_legacy.h"
#include <cmath>
#include <vector>

namespace pppp { namespace mania { namespace difficulty {
    namespace {
        void create_difficulty_attributes(ManiaDifficultyAttributes& out, const ManiaBeatmap& pb,
                                          skills::Strain& strain) {
            out.star_rating = strain.difficulty_value() * DIFFICULTY_MULTIPLIER;
            out.max_combo = max_combo(pb);
        }
    } // namespace

    void preprocessing::create_hit_objects(std::vector<preprocessing::ManiaDifficultyHitObject>& out,
                                           std::vector<preprocessing::DifficultyHitObject*>& object_ptrs,
                                           const ManiaBeatmap& pb) {
        out.clear();
        object_ptrs.clear();

        const int total_columns = pb.total_columns();
        if (pb.objects.size() < 2 || total_columns <= 0) {
            return;
        }

        std::vector<ManiaHitObject> sorted = pb.objects;
        utils::sort::legacy_sort(&sorted[0], sorted.size(), object::hit_object_start_time_rounded);

        std::vector<const preprocessing::ManiaDifficultyHitObject*> last_in_column(
            static_cast<size_t>(total_columns), 0);

        out.reserve(sorted.size() - 1);
        object_ptrs.reserve(sorted.size() - 1);

        for (size_t i = 1; i < sorted.size(); i++) {
            const ManiaHitObject& object = sorted[i];
            const ManiaHitObject& last_object = sorted[i - 1];

            out.push_back(preprocessing::ManiaDifficultyHitObject(object, last_object, pb.clock_rate,
                                                                  object_ptrs, last_in_column,
                                                                  static_cast<int>(out.size())));
            last_in_column[static_cast<size_t>(object.column)] = &out.back();
            object_ptrs.push_back(&out.back());
        }
    }

    Status calculate_difficulty(ManiaDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        ManiaBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        out = ManiaDifficultyAttributes();

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        std::vector<preprocessing::ManiaDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        skills::Strain strain(mods, mod_count, pb.total_columns());
        for (size_t i = 0; i < dho_list.size(); i++) {
            strain.process(dho_list[i]);
        }

        create_difficulty_attributes(out, pb, strain);

        return StatusCode::OK;
    }

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<ManiaDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count) {
        times.clear();
        attributes.clear();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        ManiaBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        skills::Strain strain(mods, mod_count, pb.total_columns());

        ManiaBeatmap progressive = pb;
        progressive.objects.clear();

        std::vector<preprocessing::ManiaDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        std::vector<ManiaHitObject> sorted = pb.objects;
        utils::sort::legacy_sort(&sorted[0], sorted.size(), object::hit_object_start_time_rounded);

        size_t current = 0;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            progressive.objects.push_back(pb.objects[i]);

            while (current < dho_list.size() && sorted[current + 1].end_time <= pb.objects[i].end_time) {
                strain.process(dho_list[current]);

                current++;
            }

            ManiaDifficultyAttributes step;
            create_difficulty_attributes(step, progressive, strain);
            times.push_back(pb.objects[i].end_time);
            attributes.push_back(step);
        }

        return StatusCode::OK;
    }

    Status calculate_strains(ManiaStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count) {
        out = ManiaStrains();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        ManiaBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        const double section_length = 400;

        std::vector<preprocessing::ManiaDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        skills::Strain strain(mods, mod_count, pb.total_columns());
        if (!pb.objects.empty()) {
            for (size_t i = 0; i < dho_list.size(); i++) {
                strain.process(dho_list[i]);
            }
        }

        if (!dho_list.empty()) {
            out.start_time =
                (std::ceil(dho_list[0].start_time / section_length) * section_length - section_length) *
                pb.clock_rate;
        }
        out.section_length = section_length * pb.clock_rate;

        strain.get_current_strain_peaks(out.strain);

        return StatusCode::OK;
    }
}}} // namespace pppp::mania::difficulty
