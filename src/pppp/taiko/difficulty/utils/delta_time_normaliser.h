// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_UTILS_DELTA_TIME_NORMALISER_H
#define PPPP_TAIKO_DIFFICULTY_UTILS_DELTA_TIME_NORMALISER_H

#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace utils {
    /// Normalises delta time values for the difficulty hit objects.
    ///
    /// Combines delta time values that differ by at most margin_of_error and replaces each value
    /// with the median of its range. This is used to reduce timing noise and improve rhythm
    /// grouping consistency, especially for maps with inconsistent or off-snapped timing.
    void normalise_delta_times(std::vector<double>& out, const std::vector<double>& delta_times,
                               double margin_of_error);
}}}} // namespace pppp::taiko::difficulty::utils

#endif
