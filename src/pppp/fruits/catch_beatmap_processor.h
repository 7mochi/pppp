// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_CATCH_BEATMAP_PROCESSOR_H
#define PPPP_FRUITS_CATCH_BEATMAP_PROCESSOR_H

#include "pppp/fruits/catch_beatmap.h"
#include "pppp/utils/random/osu.h"

namespace pppp { namespace fruits {
    const int RNG_SEED = 1337;

    /// Applies an offset to a position, ensuring that the final position remains within the boundary of the
    /// playfield.
    /// @param position The position which the offset should be applied to.
    /// @param amount The amount to offset by.
    void apply_offset(float& position, float amount);

    /// Applies a random offset in a random direction to a position, ensuring that the final position remains
    /// within the boundary of the playfield.
    /// @param position The position which the offset should be applied to.
    /// @param max_offset The maximum offset, cannot exceed 20px.
    /// @param rng The random number generator.
    void apply_random_offset(float& position, double max_offset, utils::LegacyRandom& rng);

    void apply_hard_rock_offset(object::CatchHitObject& object, bool& has_last, float& last_position,
                                double& last_start_time, utils::LegacyRandom& rng);
    void initialise_hyper_dash(CatchBeatmap& pb);

    void apply_position_offsets(CatchBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                                bool hard_rock_offsets);
}} // namespace pppp::fruits

#endif
