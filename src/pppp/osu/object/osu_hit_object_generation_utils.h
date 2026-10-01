// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OBJECT_OSU_HIT_OBJECT_GENERATION_UTILS_H
#define PPPP_OSU_OBJECT_OSU_HIT_OBJECT_GENERATION_UTILS_H

#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/osu/object/osu_playfield.h"
#include "pppp/utils/random/csharp.h"
#include "pppp/utils/vector2.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace osu {
    struct OsuBeatmap;
    namespace object {
        // The relative distance to the edge of the playfield before objects' positions should start to "turn
        // around" and curve towards the middle.
        // The closer the hit objects draw to the border, the sharper the turn
        const float PLAYFIELD_EDGE_RATIO = 0.375f;
        const float BORDER_DISTANCE_X = PLAYFIELD_WIDTH * PLAYFIELD_EDGE_RATIO;
        const float BORDER_DISTANCE_Y = PLAYFIELD_HEIGHT * PLAYFIELD_EDGE_RATIO;

        /// Number of previous hitobjects to be shifted together when an object is being moved.
        const int PRECEDING_HITOBJECTS_TO_SHIFT = 10;

        struct ObjectPositionInfo {
            /// The jump angle from the previous hit object to this one, relative to the previous hit object's
            /// jump angle.
            /// @remarks relative_angle of the first hit object in a beatmap represents the absolute angle
            /// from playfield center to the object.
            /// @example If relative_angle is 0, the player's cursor doesn't need to change its direction of
            /// movement when passing the previous object to reach this one.
            float relative_angle;

            /// The jump distance from the previous hit object to this one.
            /// @remarks distance_from_previous of the first hit object in a beatmap is relative to the
            /// playfield center.
            float distance_from_previous;

            /// The rotation of the hit object, relative to its jump angle.
            /// For sliders, this is defined as the angle from the slider's start position to the end of its
            /// path, relative to its jump angle. For hit circles and spinners, this property is ignored.
            float rotation;

            /// The hit object associated with this ObjectPositionInfo.
            size_t object;
        };

        /// Rotates vector "initial" towards vector "destination".
        /// @param initial The vector to be rotated.
        /// @param destination The vector that "initial" should be rotated towards.
        /// @param rotation_ratio How much "initial" should be rotated. 0 means no rotation. 1 means "initial"
        /// is fully rotated to equal "destination".
        /// @returns The rotated vector.
        pppp::utils::Vector2 rotate_vector_towards_vector(pppp::utils::Vector2 initial,
                                                          pppp::utils::Vector2 destination,
                                                          float rotation_ratio);

        /// Rotate a hit object away from the playfield edge, while keeping a constant distance
        /// from the previous object.
        /// @remarks The extent of rotation depends on the position of the hit object. Hit objects
        /// closer to the playfield edge will be rotated to a larger extent.
        /// @param prev_object_pos Position of the previous hit object.
        /// @param pos_relative_to_prev Position of the hit object to be rotated, relative to the previous hit
        /// object.
        /// @param rotation_ratio The extent of rotation. 0 means the hit object is never rotated. 1 means the
        /// hit object will be fully rotated towards playfield center when it is originally at playfield edge.
        /// @returns The new position of the hit object, relative to the previous one.
        pppp::utils::Vector2 rotate_away_from_edge(pppp::utils::Vector2 prev_object_pos,
                                                   pppp::utils::Vector2 pos_relative_to_prev,
                                                   float rotation_ratio);

        /// Flips the position of the Slider around its start position horizontally.
        /// @param pb The beatmap whose slider is flipped.
        /// @param slider_index The slider to be flipped.
        void flip_slider_in_place_horizontally(OsuBeatmap& pb, int slider_index);

        /// Rotate a slider about its start position by the specified angle.
        /// @param pb The beatmap whose slider is rotated.
        /// @param slider_index The slider to be rotated.
        /// @param rotation The angle, measured in radians, to rotate the slider by.
        void rotate_slider(OsuBeatmap& pb, int slider_index, float rotation);

        /// @param pb The beatmap ho is a part of.
        /// @param ho The OsuHitObject that should be checked.
        /// @param downbeats_only If true, this method only returns true if ho is on a downbeat.
        /// If false, it returns true if ho is on any beat.
        /// @returns true if ho is on a (down-)beat, false otherwise.
        bool is_hit_object_on_beat(const OsuBeatmap& pb, const OsuHitObject& ho, bool downbeats_only);

        /// Generates a random number from a normal distribution using the Box-Muller transform.
        float random_gaussian(utils::DotNetRandom& rng, float mean, float std_dev);

        /// Generate a list of ObjectPositionInfos containing information for how the given list of
        /// OsuHitObjects are positioned.
        /// @param pb The beatmap whose hit objects are processed.
        /// @returns A list of ObjectPositionInfos describing how each hit object is positioned relative to
        /// the previous one.
        std::vector<ObjectPositionInfo> generate_position_infos(const OsuBeatmap& pb);

        /// Reposition the hit objects according to the information in `infos`.
        /// @param pb The beatmap whose hit objects are repositioned.
        /// @param infos Position information for each hit object.
        void reposition_hit_objects(OsuBeatmap& pb, const std::vector<ObjectPositionInfo>& infos);
    } // namespace object
}} // namespace pppp::osu

#endif
