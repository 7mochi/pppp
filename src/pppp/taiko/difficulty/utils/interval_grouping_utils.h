// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_UTILS_INTERVAL_GROUPING_UTILS_H
#define PPPP_TAIKO_DIFFICULTY_UTILS_INTERVAL_GROUPING_UTILS_H

#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace utils {
    // The margin of error when comparing intervals for grouping, or snapping intervals to a common value.
    const double MARGIN_OF_ERROR = 5.0;

    void group_by_interval(std::vector<std::vector<int> >& out, const std::vector<double>& intervals);
}}}} // namespace pppp::taiko::difficulty::utils

#endif
