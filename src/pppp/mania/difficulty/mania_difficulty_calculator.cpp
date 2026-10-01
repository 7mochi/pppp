// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/mania/difficulty/preprocessing/mania_difficulty_hit_object.h"
#include "pppp/mania/difficulty/skills/strain.h"
#include "pppp/mania/mania_beatmap.h"
#include "pppp/utils/sort/osu_legacy.h"
#include <vector>

namespace pppp { namespace mania { namespace difficulty {

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

    Result::Value calculate_difficulty(ManiaDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                       const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return Result::INVALID_ARGUMENT;
        }

        ManiaBeatmap pb;
        Result::Value status = build(pb, beatmap, mods, mod_count);
        if (status != Result::OK) {
            return status;
        }

        out = ManiaDifficultyAttributes();

        if (pb.objects.empty()) {
            return Result::OK;
        }

        std::vector<preprocessing::ManiaDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        skills::Strain strain(mods, mod_count, pb.total_columns());
        for (size_t i = 0; i < dho_list.size(); i++) {
            strain.process(dho_list[i]);
        }

        out.star_rating = strain.difficulty_value() * DIFFICULTY_MULTIPLIER;
        out.max_combo = max_combo(pb);

        return Result::OK;
    }
}}} // namespace pppp::mania::difficulty
