// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_DIFFICULTY_PREPROCESSING_CATCH_DIFFICULTY_HIT_OBJECT_H
#define PPPP_FRUITS_DIFFICULTY_PREPROCESSING_CATCH_DIFFICULTY_HIT_OBJECT_H

#include "pppp/common/preprocessing/difficulty_hit_object.h"
#include "pppp/fruits/catch_beatmap.h"
#include <vector>

namespace pppp { namespace fruits { namespace difficulty { namespace preprocessing {
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    const float ABSOLUTE_PLAYER_POSITIONING_ERROR = 16.0f;
    const float NORMALIZED_HALF_CATCHER_WIDTH = 41.0f;

    struct CatchDifficultyHitObject : DifficultyHitObject {
        /// Normalized position of the base object.
        float normalized_position;

        /// Normalized position of LastObject.
        float last_normalized_position;

        /// Normalized position of the player required to catch the base object, assuming the player
        /// moves as little as possible.
        float player_position;

        /// Normalized position of the player after catching the last object.
        float last_player_position;

        /// Normalized distance between the last player position and the current player position.
        /// The sign of the value indicates the direction of the movement: negative is left and
        /// positive is right.
        float distance_moved;

        /// Normalized distance the player has to move from the last player position in order to catch
        /// the base object at its normalized position.
        /// The sign of the value indicates the direction of the movement: negative is left and
        /// positive is right.
        float exact_distance_moved;

        /// Milliseconds elapsed since the start time of the previous difficulty hit object, with a
        /// minimum of 40ms.
        double strain_time;

        float last_distance_to_hyper_dash;
        bool last_hyper_dash;

        CatchDifficultyHitObject(const object::CatchHitObject& object, const object::CatchHitObject& last,
                                 double rate, float half_catcher_width,
                                 const std::vector<DifficultyHitObject*>& objects, int object_index);

        void set_movement_state();
    };

    void create_hit_objects(std::vector<CatchDifficultyHitObject>& out,
                            std::vector<DifficultyHitObject*>& object_ptrs, CatchBeatmap& pb);
}}}} // namespace pppp::fruits::difficulty::preprocessing

#endif
