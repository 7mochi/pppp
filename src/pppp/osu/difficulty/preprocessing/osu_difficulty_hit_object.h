// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_DIFFICULTY_PREPROCESSING_OSU_DIFFICULTY_HIT_OBJECT_H
#define PPPP_OSU_DIFFICULTY_PREPROCESSING_OSU_DIFFICULTY_HIT_OBJECT_H

#include "pppp/common/preprocessing/difficulty_hit_object.h"
#include "pppp/config.h" // IWYU pragma: export
#include "pppp/utils/vector2.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace osu {
    struct OsuBeatmap;
    namespace object {
        struct OsuHitObject;
    }
}} // namespace pppp::osu

namespace pppp { namespace osu { namespace difficulty { namespace preprocessing {
    /// A distance by which all distances should be scaled in order to assume a uniform circle size.
    const double NORMALISED_RADIUS =
        50.0; // Change radius to 50 to make 100 the diameter. Easier for mental maths.

    const double NORMALISED_DIAMETER = NORMALISED_RADIUS * 2.0;

    const double MIN_DELTA_TIME = 25.0;

    const double MAXIMUM_SLIDER_RADIUS = NORMALISED_RADIUS * 2.4;
    const double ASSUMED_SLIDER_RADIUS = NORMALISED_RADIUS * 1.8;

    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    /// The raw, offset and stacked positions of the playable objects, computed once and shared by the
    /// difficulty hit objects.
    struct PositionCtx {
        std::vector<pppp::utils::Vector2> raw;
        std::vector<pppp::utils::Vector2> offset;
        std::vector<pppp::utils::Vector2> stacked;

        pppp::utils::Vector2 nested(size_t index, pppp::utils::Vector2 rel) const {
            return (raw[index] + rel) + offset[index];
        }
    };

    struct OsuDifficultyHitObject : DifficultyHitObject {
        pppp::utils::Vector2 base_stacked_position;
        pppp::utils::Vector2 base_stacked_end_position;
        bool base_is_slider;
        bool base_is_spinner;

        pppp::utils::Vector2 last_stacked_position;
        bool last_is_slider;

        /// delta_time capped to a minimum of MIN_DELTA_TIME ms.
        double adjusted_delta_time;

        /// Amount of time elapsed between the previous object's end_time and start_time, capped to
        /// a minimum of MIN_DELTA_TIME ms.
        double last_object_end_delta_time;

        /// Time (in ms) between the object first appearing and the time it needs to be clicked.
        /// time_preempt adjusted by clock rate.
        double preempt;

        double time_fade_in;

        /// Normalised distance from the start position of the previous object to the start position
        /// of this object.
        double jump_distance;

        /// Normalised distance from the "lazy" end position of the previous object to the start
        /// position of this object. The "lazy" end position is the position at which the cursor
        /// ends up if the previous hitobject is followed with as minimal movement as possible (i.e.
        /// on the edge of slider follow circles).
        double lazy_jump_distance;

        /// Normalised shortest distance to consider for a jump between the previous object and this
        /// object. This is bounded from above by lazy_jump_distance, and is smaller than the former
        /// if a more natural path is able to be taken through the previous object.
        double minimum_jump_distance;

        /// The time taken to travel through minimum_jump_distance, with a minimum value of 25ms.
        double minimum_jump_time;

        /// Normalised distance between the start and end position of this object.
        double travel_distance;

        /// The time taken to travel through travel_distance, with a minimum value of 25ms for
        /// sliders.
        double travel_time;

        /// The position of the cursor at the point of completion of this object if it is a slider
        /// and was hit with as few movements as possible.
        nonstd::optional<pppp::utils::Vector2> lazy_end_position;

        /// The distance travelled by the cursor upon completion of this object if it is a slider and
        /// was hit with as few movements as possible.
        double lazy_travel_distance;

        /// The time taken by the cursor upon completion of this object if it is a slider and was hit
        /// with as few movements as possible.
        double lazy_travel_time;

        /// Angle the player has to take to hit this object.
        /// Calculated as the angle between the circles (current-2, current-1, current).
        nonstd::optional<double> angle;

        /// Angle of the vector created between current and current-1, normalised to consider
        /// symmetrical vectors in any axis to be the same angle.
        nonstd::optional<double> normalised_vector_angle;

        /// Selective bonus for maps with higher circle size.
        double small_circle_bonus;

        /// Object's immediate OverallDifficulty value calculated from the raw hitwindow.
        double overall_difficulty;

        double circle_size;
        int repeat_count;

        OsuDifficultyHitObject(const object::OsuHitObject& object, const object::OsuHitObject& last,
                               double rate, const std::vector<DifficultyHitObject*>& objects,
                               int object_index, const OsuBeatmap& pb, const PositionCtx& pos);
    };

    void create_hit_objects(std::vector<OsuDifficultyHitObject>& out,
                            std::vector<DifficultyHitObject*>& object_ptrs, const OsuBeatmap& pb);

    double opacity_at(const OsuDifficultyHitObject& obj, double time, bool hidden);

    /// Returns how possible is it to doubletap this object together with the next one and get perfect
    /// judgement in range from 0 to 1
    double calculate_double_tap_feasibility(const OsuDifficultyHitObject& prev_obj,
                                            const OsuDifficultyHitObject& curr_obj);
}}}} // namespace pppp::osu::difficulty::preprocessing

#endif
