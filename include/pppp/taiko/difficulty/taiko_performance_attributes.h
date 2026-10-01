// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_TAIKO_PERFORMANCE_ATTRIBUTES_H
#define PPPP_TAIKO_DIFFICULTY_TAIKO_PERFORMANCE_ATTRIBUTES_H

#include "pppp/common/performance_attributes.h"
#include "pppp/config.h" // IWYU pragma: export

namespace pppp { namespace taiko { namespace difficulty {
    struct TaikoPerformanceAttributes : pppp::common::PerformanceAttributes {
        double difficulty;
        double accuracy;
        nonstd::optional<double> estimated_unstable_rate;

        TaikoPerformanceAttributes();
    };
}}} // namespace pppp::taiko::difficulty

#endif
