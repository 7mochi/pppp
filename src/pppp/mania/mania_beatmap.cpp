// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mania_beatmap.h"

namespace pppp { namespace mania {
    ManiaBeatmap::ManiaBeatmap()
        : target_columns(4),
          dual(false),
          clock_rate(1.0),
          is_for_current_ruleset(false) {}

    int max_combo(const ManiaBeatmap& pb) {
        int combo = 0;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            combo += 1;
            if (pb.objects[i].hold) {
                combo += static_cast<int>((pb.objects[i].end_time - pb.objects[i].start_time) / 100);
            }
        }
        return combo;
    }
}} // namespace pppp::mania
