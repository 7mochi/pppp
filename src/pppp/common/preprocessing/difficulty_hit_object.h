// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_PREPROCESSING_DIFFICULTY_HIT_OBJECT_H
#define PPPP_COMMON_PREPROCESSING_DIFFICULTY_HIT_OBJECT_H

#include <vector>

namespace pppp { namespace common { namespace preprocessing {
    class DifficultyHitObject {
    public:
        /// The index of this DifficultyHitObject in the list of all DifficultyHitObjects.
        int index;

        /// The start time of the HitObject this difficulty hit object wraps.
        double base_object_start_time;

        /// The start time of the last HitObject which occurs before the base object.
        double last_object_start_time;

        /// Amount of time elapsed between base_object_start_time and last_object_start_time, adjusted by
        /// clockrate.
        double delta_time;

        /// Clockrate adjusted start time of the base object.
        double start_time;

        /// Clockrate adjusted end time of the base object.
        double end_time;

        /// Beatmap playback rate.
        double clock_rate;

        double hit_window_great;

        /// Creates a new DifficultyHitObject.
        /// @param base_object_time_in The start time of the HitObject which this DifficultyHitObject wraps.
        /// @param last_object_time_in The start time of the last HitObject which occurs before the wrapped
        /// one in the beatmap.
        /// @param clock_rate_in The rate at which the gameplay clock is run at.
        /// @param objects The list of DifficultyHitObjects in the current beatmap.
        /// @param index_in The index of this DifficultyHitObject in the objects list.
        DifficultyHitObject(double base_object_time_in, double last_object_time_in, double clock_rate_in,
                            const std::vector<DifficultyHitObject*>& objects, int index_in);

        DifficultyHitObject* previous(int skip_count = 0) const;

        DifficultyHitObject* next(int skip_count = 0) const;

    private:
        const std::vector<DifficultyHitObject*>* difficulty_hit_objects;
    };
}}} // namespace pppp::common::preprocessing

#endif
