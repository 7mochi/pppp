// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/taiko_performance_attributes.h"

namespace pppp { namespace taiko { namespace difficulty {
    TaikoPerformanceAttributes::TaikoPerformanceAttributes()
        : difficulty(0.0),
          accuracy(0.0),
          estimated_unstable_rate() {}
}}} // namespace pppp::taiko::difficulty
