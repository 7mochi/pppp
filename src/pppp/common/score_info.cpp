// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/common/score_info.h"
#include <cstring>

namespace pppp { namespace common {
    ScoreInfo::ScoreInfo() {
        std::memset(statistics, 0, sizeof(statistics));
        std::memset(maximum_statistics, 0, sizeof(maximum_statistics));

        max_combo = 0;
        accuracy = 0.0;
        legacy_total_score.reset();
        mods = 0;
        mod_count = 0;
    }
}} // namespace pppp::common
