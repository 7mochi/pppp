// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_hold_off.h"
#include "pppp/utils/linq/csharp.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    namespace linq = ::pppp::utils::linq;

    void apply_hold_off(ManiaBeatmap& pb) {
        std::vector<ManiaHitObject> new_objects;

        const std::vector<ManiaHitObject> hold_notes =
            linq::from(pb.objects).where(linq::bind(&ManiaHitObject::hold, linq::_1)).to_list();
        for (size_t i = 0; i < hold_notes.size(); i++) {
            // Add a note for the beginning of the hold note
            ManiaHitObject note;
            note.column = hold_notes[i].column;
            note.start_time = hold_notes[i].start_time;
            note.duration = 0;
            new_objects.push_back(note);
        }

        std::vector<ManiaHitObject> objects = linq::from(pb.objects)
                                                  .where(!linq::bind(&ManiaHitObject::hold, linq::_1))
                                                  .concat(new_objects)
                                                  .order_by(linq::bind(&ManiaHitObject::start_time, linq::_1))
                                                  .to_list();
        pb.objects.swap(objects);
    }
}}} // namespace pppp::mania::mods
