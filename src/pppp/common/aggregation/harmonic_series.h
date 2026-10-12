// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_AGGREGATION_HARMONIC_SERIES_H
#define PPPP_COMMON_AGGREGATION_HARMONIC_SERIES_H

#include <vector>

namespace pppp { namespace common { namespace aggregation {
    struct HarmonicSeries {
        struct Result {
            double difficulty;
            double weight_sum;
        };

        /// Harmonic series (https://en.wikipedia.org/wiki/Harmonic_series_(mathematics)) summation
        /// @param difficulties List of difficulties to sum.
        /// @param harmonic_scale Scaling factor applied as HarmonicScale / (1 + index) during weight
        /// calculations. A higher value will increase the influence of the hardest object difficulties
        /// during summation.
        /// @param decay_exponent Exponent that controls the rate of which decay increases as the index
        /// increases. Values closer to 1 decay faster whilst lower values give more weight to lower
        /// object difficulties.
        /// @returns difficulty: Summation result. weightSum: The sum of difficulty weights, calculated
        /// during summation.
        static Result aggregate(const std::vector<double>& difficulties, double harmonic_scale = 1.0,
                                double decay_exponent = 0.9);
    };
}}} // namespace pppp::common::aggregation

#endif
