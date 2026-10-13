// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mania_beatmap.h"
#include "pppp/utils/linq/csharp.h"

namespace pppp { namespace mania {
    namespace {
        int max_combo_for_object(const ManiaHitObject& hit_object) {
            if (hit_object.hold) {
                return 1 + static_cast<int>((hit_object.end_time - hit_object.start_time) / 100);
            }

            return 1;
        }
    } // namespace

    ManiaBeatmap::ManiaBeatmap()
        : target_columns(4),
          dual(false),
          clock_rate(1.0),
          is_for_current_ruleset(false) {}

    int max_combo(const ManiaBeatmap& pb) {
        return pppp::utils::linq::from(pb.objects)
            .sum(pppp::utils::linq::bind(&max_combo_for_object, pppp::utils::linq::_1));
    }
}} // namespace pppp::mania
