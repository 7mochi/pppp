// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_DIFFICULTY_PREPROCESSING_MANIA_DIFFICULTY_HIT_OBJECT_H
#define PPPP_MANIA_DIFFICULTY_PREPROCESSING_MANIA_DIFFICULTY_HIT_OBJECT_H

#include "pppp/common/preprocessing/difficulty_hit_object.h"
#include "pppp/mania/mania_beatmap.h"
#include <vector>

namespace pppp { namespace mania { namespace difficulty { namespace preprocessing {
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    struct ManiaDifficultyHitObject : DifficultyHitObject {
        int column;
        double column_strain_time;
        std::vector<const ManiaDifficultyHitObject*> previous_in_columns;

        ManiaDifficultyHitObject(const ManiaHitObject& object, const ManiaHitObject& last_object, double rate,
                                 const std::vector<DifficultyHitObject*>& objects,
                                 const std::vector<const ManiaDifficultyHitObject*>& previous_in_column,
                                 int object_index);
    };

    void create_hit_objects(std::vector<ManiaDifficultyHitObject>& out,
                            std::vector<DifficultyHitObject*>& object_ptrs, const ManiaBeatmap& pb);
}}}} // namespace pppp::mania::difficulty::preprocessing

#endif
