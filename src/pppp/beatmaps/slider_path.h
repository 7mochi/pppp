// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_SLIDER_PATH_H
#define PPPP_BEATMAPS_SLIDER_PATH_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/utils/vector2.h"
#include <vector>

namespace pppp { namespace beatmaps {
    /// Computes the position on the slider at a given progress that ranges from 0 (beginning of the path)
    /// to 1 (end of the path).
    /// @param path The path vertices.
    /// @param cumulative_lengths The cumulative length up to each vertex.
    /// @param progress Ranges from 0 (beginning of the path) to 1 (end of the path).
    pppp::utils::Vector2 slider_position_at(const std::vector<pppp::utils::Vector2>& path,
                                            const std::vector<double>& cumulative_lengths, double progress);

    /// Computes the position on the slider at a given progress that ranges from 0 (beginning of the
    /// path) to 1 (end of the path), sampling the undecimated path where the slider carries one.
    /// @param slider The slider whose path is sampled.
    /// @param progress Ranges from 0 (beginning of the path) to 1 (end of the path).
    pppp::utils::Vector2 slider_position_at_undecimated(const pppp::beatmaps::Slider& slider,
                                                        double progress);
}} // namespace pppp::beatmaps

#endif
