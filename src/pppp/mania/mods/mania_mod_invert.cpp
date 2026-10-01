// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_invert.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    void apply_invert(ManiaBeatmap& pb, const pppp::beatmaps::control_points::ControlPointInfo& info) {
        const int columns = pb.total_columns();
        std::vector<ManiaHitObject> out;

        std::vector<int> column_order;
        std::vector<bool> seen(static_cast<size_t>(columns > 0 ? columns : 0), false);
        for (size_t i = 0; i < pb.objects.size(); i++) {
            const int column = pb.objects[i].column;
            if (column >= 0 && static_cast<size_t>(column) < seen.size() &&
                !seen[static_cast<size_t>(column)]) {
                seen[static_cast<size_t>(column)] = true;
                column_order.push_back(column);
            }
        }

        for (size_t c = 0; c < column_order.size(); c++) {
            const int column = column_order[c];

            std::vector<double> locations;
            for (size_t i = 0; i < pb.objects.size(); i++) {
                if (pb.objects[i].column == column && !pb.objects[i].hold) {
                    locations.push_back(pb.objects[i].start_time);
                }
            }
            for (size_t i = 0; i < pb.objects.size(); i++) {
                if (pb.objects[i].column == column && pb.objects[i].hold) {
                    locations.push_back(pb.objects[i].start_time);
                }
            }
            std::stable_sort(locations.begin(), locations.end());

            for (size_t i = 0; i + 1 < locations.size(); i++) {
                // Full duration of the hold note.
                double duration = locations[i + 1] - locations[i];

                // Beat length at the end of the hold note.
                const double beat_length = (info.timing_point_at(locations[i + 1]) != 0
                                                ? info.timing_point_at(locations[i + 1])->beat_length
                                                : pppp::beatmaps::control_points::DEFAULT_BEAT_LENGTH);

                // Decrease the duration by at most a 1/4 beat to ensure there's no instantaneous notes.
                duration = std::max(duration / 2, duration - beat_length / 4);

                ManiaHitObject hold;
                hold.column = column;
                hold.start_time = locations[i];
                hold.end_time = locations[i] + duration;
                hold.hold = true;
                out.push_back(hold);
            }
        }

        std::stable_sort(out.begin(), out.end(), object::hit_object_earlier);
        pb.objects.swap(out);
    }
}}} // namespace pppp::mania::mods
