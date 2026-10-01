// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_hold_off.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    void apply_hold_off(ManiaBeatmap& pb) {
        std::vector<ManiaHitObject> kept;
        std::vector<ManiaHitObject> converted;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            if (pb.objects[i].hold) {
                ManiaHitObject note = pb.objects[i];
                note.end_time = note.start_time;
                note.hold = false;
                converted.push_back(note);
            } else {
                kept.push_back(pb.objects[i]);
            }
        }

        for (size_t i = 0; i < converted.size(); i++) {
            kept.push_back(converted[i]);
        }

        std::stable_sort(kept.begin(), kept.end(), object::hit_object_earlier);
        pb.objects.swap(kept);
    }
}}} // namespace pppp::mania::mods
