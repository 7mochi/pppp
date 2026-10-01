// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/fruits/catch_beatmap.h"
#include "pppp/fruits/difficulty/preprocessing/catch_difficulty_hit_object.h"
#include "pppp/fruits/difficulty/skills/movement.h"
#include <cmath>
#include <vector>

namespace pppp { namespace fruits { namespace difficulty {
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

    Result::Value calculate_difficulty(CatchDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                       const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return Result::INVALID_ARGUMENT;
        }

        CatchBeatmap pb;
        Result::Value status = build(pb, beatmap, mods, mod_count);
        if (status != Result::OK) {
            return status;
        }

        out = CatchDifficultyAttributes();

        if (pb.objects.empty()) {
            return Result::OK;
        }

        std::vector<preprocessing::CatchDifficultyHitObject> dho_list;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        preprocessing::create_hit_objects(dho_list, dho_ptrs, pb);

        skills::Movement movement(mods, mod_count);
        for (size_t i = 0; i < dho_list.size(); i++) {
            movement.process(dho_list[i]);
        }

        out.star_rating = std::sqrt(movement.difficulty_value()) * DIFFICULTY_MULTIPLIER;
        out.max_combo = max_combo(pb);

        return Result::OK;
    }
}}} // namespace pppp::fruits::difficulty
