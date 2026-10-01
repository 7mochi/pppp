// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_DIFFICULTY_MANIA_PERFORMANCE_ATTRIBUTES_H
#define PPPP_MANIA_DIFFICULTY_MANIA_PERFORMANCE_ATTRIBUTES_H

#include "pppp/common/performance_attributes.h"

namespace pppp { namespace mania { namespace difficulty {
    struct ManiaPerformanceAttributes : pppp::common::PerformanceAttributes {
        double difficulty;

        ManiaPerformanceAttributes();
    };
}}} // namespace pppp::mania::difficulty

#endif
