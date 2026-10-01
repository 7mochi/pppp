// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/difficulty/preprocessing/catch_difficulty_hit_object.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>

namespace pppp { namespace fruits { namespace difficulty { namespace preprocessing {
    CatchDifficultyHitObject::CatchDifficultyHitObject(const object::CatchHitObject& object,
                                                       const object::CatchHitObject& last, double rate,
                                                       float half_catcher_width,
                                                       const std::vector<DifficultyHitObject*>& objects,
                                                       int object_index)
        : DifficultyHitObject(object.time, last.time, rate, objects, object_index),
          normalized_position(0.0),
          last_normalized_position(0.0),
          player_position(0.0),
          last_player_position(0.0),
          distance_moved(0.0),
          exact_distance_moved(0.0),
          strain_time(0.0),
          last_distance_to_hyper_dash(0.0f),
          last_hyper_dash(false) {
        // We will scale everything by this factor, so we can assume a uniform CircleSize among beatmaps.
        float scaling_factor = NORMALIZED_HALF_CATCHER_WIDTH / half_catcher_width;

        end_time = object.time / rate;
        hit_window_great = 0.0;

        normalized_position = static_cast<float>(object.effective_x()) * scaling_factor;
        last_normalized_position = static_cast<float>(last.effective_x()) * scaling_factor;

        // Every strain interval is hard capped at the equivalent of 375 BPM streaming speed as a safety
        // measure
        strain_time = std::max(40.0, delta_time);

        last_distance_to_hyper_dash = static_cast<float>(last.distance_to_hyper_dash);
        last_hyper_dash = last.hyper_dash;

        set_movement_state();
    }

    void CatchDifficultyHitObject::set_movement_state() {
        last_player_position =
            index == 0 ? last_normalized_position
                       : static_cast<const CatchDifficultyHitObject*>(previous(0))->player_position;

        player_position = static_cast<float>(utils::math::clamp(
            last_player_position,
            normalized_position - (NORMALIZED_HALF_CATCHER_WIDTH - ABSOLUTE_PLAYER_POSITIONING_ERROR),
            normalized_position + (NORMALIZED_HALF_CATCHER_WIDTH - ABSOLUTE_PLAYER_POSITIONING_ERROR)));

        distance_moved = player_position - last_player_position;

        // For the exact position we consider that the catcher is in the correct position for both objects
        exact_distance_moved = normalized_position - last_player_position;

        // After a hyperdash we ARE in the correct position. Always!
        if (last_hyper_dash) {
            player_position = normalized_position;
        }
    }
}}}} // namespace pppp::fruits::difficulty::preprocessing
