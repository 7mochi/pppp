// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/preprocessing/difficulty_hit_object.h"
#include <cstddef>

namespace pppp { namespace common { namespace preprocessing {
    DifficultyHitObject::DifficultyHitObject(double base_object_time_in, double last_object_time_in,
                                             double clock_rate_in,
                                             const std::vector<DifficultyHitObject*>& objects, int index_in)
        : index(index_in),
          base_object_start_time(base_object_time_in),
          last_object_start_time(last_object_time_in),
          delta_time((base_object_time_in - last_object_time_in) / clock_rate_in),
          start_time(base_object_time_in / clock_rate_in),
          end_time(0.0),
          clock_rate(clock_rate_in),
          hit_window_great(0.0),
          difficulty_hit_objects(&objects) {}

    DifficultyHitObject* DifficultyHitObject::previous(int skip_count) const {
        int target = index - (skip_count + 1);
        return target >= 0 && target < static_cast<int>(difficulty_hit_objects->size())
                   ? (*difficulty_hit_objects)[static_cast<size_t>(target)]
                   : 0;
    }

    DifficultyHitObject* DifficultyHitObject::next(int skip_count) const {
        int target = index + (skip_count + 1);
        return target >= 0 && target < static_cast<int>(difficulty_hit_objects->size())
                   ? (*difficulty_hit_objects)[static_cast<size_t>(target)]
                   : 0;
    }
}}} // namespace pppp::common::preprocessing
