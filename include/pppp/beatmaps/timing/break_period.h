// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_TIMING_BREAK_PERIOD_H
#define PPPP_BEATMAPS_TIMING_BREAK_PERIOD_H

namespace pppp { namespace beatmaps { namespace timing {
    struct BreakPeriod {
        /// The break start time.
        double start_time;

        /// The break end time.
        double end_time;
    };
}}} // namespace pppp::beatmaps::timing

#endif
