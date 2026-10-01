// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/preprocessing/mania_difficulty_hit_object.h"

namespace pppp { namespace mania { namespace difficulty { namespace preprocessing {
    ManiaDifficultyHitObject::ManiaDifficultyHitObject(
        const ManiaHitObject& object, const ManiaHitObject& last_object, double rate,
        const std::vector<DifficultyHitObject*>& objects,
        const std::vector<const ManiaDifficultyHitObject*>& previous_in_column, int object_index)
        : DifficultyHitObject(object.start_time, last_object.start_time, rate, objects, object_index),
          column(0),
          column_strain_time(0.0) {
        end_time = object.end_time / rate;
        column = object.column;
        const ManiaDifficultyHitObject* previous_in_column_object =
            previous_in_column[static_cast<size_t>(object.column)];
        column_strain_time =
            previous_in_column_object ? start_time - previous_in_column_object->start_time : start_time;

        if (index == 0) {
            previous_in_columns.assign(previous_in_column.size(), 0);
        } else {
            const ManiaDifficultyHitObject* prev_note =
                static_cast<const ManiaDifficultyHitObject*>(previous(0));
            previous_in_columns = prev_note->previous_in_columns;

            // intentionally depends on processing order to match live.
            previous_in_columns[static_cast<size_t>(prev_note->column)] = prev_note;
        }
    }
}}}} // namespace pppp::mania::difficulty::preprocessing
