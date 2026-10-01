// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_PERFORMANCE_ATTRIBUTES_H
#define PPPP_COMMON_PERFORMANCE_ATTRIBUTES_H

namespace pppp { namespace common {
    struct PerformanceAttributes {
        /// Calculated score performance points.
        double total;

        PerformanceAttributes();
    };
}} // namespace pppp::common

#endif
