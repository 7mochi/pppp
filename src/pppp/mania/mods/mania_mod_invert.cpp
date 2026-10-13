// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_invert.h"
#include "pppp/utils/linq/csharp.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    void apply_invert(ManiaBeatmap& pb, const pppp::beatmaps::control_points::ControlPointInfo& info) {
        std::vector<ManiaHitObject> new_objects;

        const std::vector<pppp::utils::linq::Grouping<int, ManiaHitObject> > columns =
            pppp::utils::linq::from(pb.objects)
                .group_by(pppp::utils::linq::bind(&ManiaHitObject::column, pppp::utils::linq::_1))
                .to_list();
        for (size_t c = 0; c < columns.size(); c++) {
            const pppp::utils::linq::Grouping<int, ManiaHitObject>& column = columns[c];
            std::vector<ManiaHitObject> new_column_objects;

            const std::vector<double> locations =
                pppp::utils::linq::from(column.elements)
                    .where(!pppp::utils::linq::bind(&ManiaHitObject::hold, pppp::utils::linq::_1))
                    .select(pppp::utils::linq::bind(&ManiaHitObject::start_time, pppp::utils::linq::_1))
                    .concat(pppp::utils::linq::from(column.elements)
                                .where(pppp::utils::linq::bind(&ManiaHitObject::hold, pppp::utils::linq::_1))
                                .select(pppp::utils::linq::bind(&ManiaHitObject::start_time,
                                                                pppp::utils::linq::_1))
                                .to_list())
                    .order_by(pppp::utils::linq::_1)
                    .to_list();

            for (int i = 0; i < static_cast<int>(locations.size()) - 1; i++) {
                // Full duration of the hold note.
                double duration = locations[i + 1] - locations[i];

                // Beat length at the end of the hold note.
                const double beat_length = (info.timing_point_at(locations[i + 1]) != 0
                                                ? info.timing_point_at(locations[i + 1])->beat_length
                                                : pppp::beatmaps::control_points::DEFAULT_BEAT_LENGTH);

                // Decrease the duration by at most a 1/4 beat to ensure there's no instantaneous notes.
                duration = std::max(duration / 2, duration - beat_length / 4);

                ManiaHitObject hold;
                hold.column = column.key;
                hold.start_time = locations[i];
                hold.end_time = locations[i] + duration;
                hold.hold = true;
                new_column_objects.push_back(hold);
            }

            new_objects.insert(new_objects.end(), new_column_objects.begin(), new_column_objects.end());
        }

        std::vector<ManiaHitObject> objects =
            pppp::utils::linq::from(new_objects)
                .order_by(pppp::utils::linq::bind(&ManiaHitObject::start_time, pppp::utils::linq::_1))
                .to_list();
        pb.objects.swap(objects);
    }
}}} // namespace pppp::mania::mods
