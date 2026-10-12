// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/aggregation/harmonic_series.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>
#include <functional>

namespace pppp { namespace common { namespace aggregation {
    HarmonicSeries::Result HarmonicSeries::aggregate(const std::vector<double>& difficulties,
                                                     double harmonic_scale, double decay_exponent) {
        Result result;
        result.difficulty = 0;
        result.weight_sum = 0;

        if (difficulties.empty()) {
            return result;
        }

        std::vector<double> sorted(difficulties.begin(), difficulties.end());
        std::sort(sorted.begin(), sorted.end(), std::greater<double>());

        int index = 0;
        for (size_t i = 0; i < sorted.size(); i++) {
            if (!(sorted[i] > 0)) {
                continue;
            }

            // Use a harmonic sum that considers each object of the map according to a predefined weight.
            double weight =
                (1 + (harmonic_scale / (1 + index))) /
                (utils::pow(static_cast<double>(index), decay_exponent) + 1 + (harmonic_scale / (1 + index)));

            result.weight_sum += weight;

            result.difficulty += sorted[i] * weight;
            index += 1;
        }

        return result;
    }
}}} // namespace pppp::common::aggregation
