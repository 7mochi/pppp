// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_mirror.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    void apply_mirror(ManiaBeatmap& pb) {
        const int columns = pb.total_columns();

        for (size_t i = 0; i < pb.objects.size(); i++) {
            pb.objects[i].column = columns - 1 - pb.objects[i].column;
        }
    }
}}} // namespace pppp::mania::mods
