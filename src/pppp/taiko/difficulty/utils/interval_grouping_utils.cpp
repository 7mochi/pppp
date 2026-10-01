// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/utils/interval_grouping_utils.h"
#include "pppp/utils/precision.h"
#include <cstddef>

namespace pppp { namespace taiko { namespace difficulty { namespace utils {
    namespace {
        void create_next_group(std::vector<int>& group, const std::vector<double>& intervals, size_t& i) {
            size_t count = intervals.size();

            // This never compares the first two elements in the group.
            // This sounds wrong but is apparently "as intended"
            // (https://github.com/ppy/osu/pull/31636#discussion_r1942673329)
            group.push_back(static_cast<int>(i));
            i++;

            for (; i + 1 < count; i++) {
                if (!pppp::utils::almost_equals(intervals[i], intervals[i + 1], MARGIN_OF_ERROR)) {
                    // When an interval change occurs, include the object with the differing interval in the
                    // case it increased
                    // See https://github.com/ppy/osu/pull/31636#discussion_r1942368372 for rationale.
                    if (intervals[i + 1] > intervals[i] + MARGIN_OF_ERROR) {
                        group.push_back(static_cast<int>(i));
                        i++;
                    }
                    return;
                }

                // No interval change occurred
                group.push_back(static_cast<int>(i));
            }

            // Check if the last two objects in the object form a "flat" rhythm pattern within the specified
            // margin of error.
            // If true, add the current object to the group and increment the index to process the next
            // object.
            if (count > 2 && i < count &&
                pppp::utils::almost_equals(intervals[count - 1], intervals[count - 2], MARGIN_OF_ERROR)) {
                group.push_back(static_cast<int>(i));
                i++;
            }
        }
    } // namespace

    void group_by_interval(std::vector<std::vector<int> >& out, const std::vector<double>& intervals) {
        out.clear();

        size_t i = 0;
        while (i < intervals.size()) {
            std::vector<int> group;
            create_next_group(group, intervals, i);
            out.push_back(group);
        }
    }
}}}} // namespace pppp::taiko::difficulty::utils
