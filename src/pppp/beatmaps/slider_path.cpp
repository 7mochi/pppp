// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/beatmaps/slider_path.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/search/csharp.h"
#include <cmath>

namespace pppp { namespace beatmaps {
    namespace {
        double progress_to_distance(double progress, const std::vector<double>& cumulative_lengths) {
            double distance = cumulative_lengths.empty() ? 0.0 : cumulative_lengths.back();
            return utils::math::clamp(progress, 0.0, 1.0) * distance;
        }

        int index_of_distance(const std::vector<double>& cumulative_lengths, double d) {
            int i = utils::search::binary_search(cumulative_lengths, d);
            if (i < 0) {
                i = ~i;
            }
            return i;
        }

        pppp::utils::Vector2 interpolate_vertices(const std::vector<pppp::utils::Vector2>& path,
                                                  const std::vector<double>& cumulative_lengths, int i,
                                                  double d) {
            if (path.empty()) {
                pppp::utils::Vector2 zero = {0.0f, 0.0f};
                return zero;
            }
            if (i <= 0) {
                return path.front();
            }
            if (i >= static_cast<int>(path.size())) {
                return path.back();
            }

            double d0 = cumulative_lengths[i - 1], d1 = cumulative_lengths[i];

            // Avoid division by and almost-zero number in case two points are extremely close to each other.
            if (std::fabs(d0 - d1) <= 1e-7) {
                return path[i - 1];
            }

            double w = (d - d0) / (d1 - d0);
            return path[i - 1] + (path[i] - path[i - 1]) * static_cast<float>(w);
        }
    } // namespace

    pppp::utils::Vector2 slider_position_at(const std::vector<pppp::utils::Vector2>& path,
                                            const std::vector<double>& cumulative_lengths, double progress) {
        double d = progress_to_distance(progress, cumulative_lengths);
        return interpolate_vertices(path, cumulative_lengths, index_of_distance(cumulative_lengths, d), d);
    }

    pppp::utils::Vector2 slider_position_at_undecimated(const pppp::beatmaps::Slider& slider,
                                                        double progress) {
        if (!slider.undecimated_path.empty()) {
            return slider_position_at(slider.undecimated_path, slider.undecimated_cumulative_lengths,
                                      progress);
        }
        return slider_position_at(slider.path, slider.cumulative_lengths, progress);
    }
}} // namespace pppp::beatmaps
