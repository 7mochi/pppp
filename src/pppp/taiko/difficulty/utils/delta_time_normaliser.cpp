// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/utils/delta_time_normaliser.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace taiko { namespace difficulty { namespace utils {
    void normalise_delta_times(std::vector<double>& out, const std::vector<double>& delta_times,
                               double margin_of_error) {
        out = delta_times;

        std::vector<double> distinct = delta_times;
        std::sort(distinct.begin(), distinct.end());
        distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());

        std::vector<double> medians(distinct.size(), 0.0);
        size_t begin = 0;
        while (begin < distinct.size()) {
            size_t end = begin + 1;
            // Add to the current group if within margin of error
            while (end < distinct.size() && std::fabs(distinct[end] - distinct[begin]) <= margin_of_error) {
                end++;
            }

            // Compute median for each group
            size_t count = end - begin;
            size_t mid = count / 2;
            double median = count % 2 == 1 ? distinct[begin + mid]
                                           : (distinct[begin + mid - 1] + distinct[begin + mid]) / 2.0;
            for (size_t k = begin; k < end; k++) {
                medians[k] = median;
            }

            // Otherwise begin a new group
            begin = end;
        }

        // Assign each hitobjects deltaTime the corresponding median value
        for (size_t i = 0; i < out.size(); i++) {
            size_t position = static_cast<size_t>(std::lower_bound(distinct.begin(), distinct.end(), out[i]) -
                                                  distinct.begin());
            if (position < distinct.size() && distinct[position] == out[i]) {
                out[i] = medians[position];
            }
        }
    }
}}}} // namespace pppp::taiko::difficulty::utils
