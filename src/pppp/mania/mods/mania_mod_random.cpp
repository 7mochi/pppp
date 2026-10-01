// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_random.h"
#include "pppp/utils/random/csharp.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    void apply_random(ManiaBeatmap& pb, int seed) {
        const int columns = pb.total_columns();
        utils::DotNetRandom rng(seed);

        std::vector<int> keys(static_cast<size_t>(columns));
        std::vector<int> order(static_cast<size_t>(columns));
        for (int i = 0; i < columns; i++) {
            keys[static_cast<size_t>(i)] = rng.next();
            order[static_cast<size_t>(i)] = i;
        }

        for (int i = 1; i < columns; i++) {
            const int key = order[static_cast<size_t>(i)];
            int j = i;
            while (j > 0 && keys[static_cast<size_t>(order[static_cast<size_t>(j - 1)])] >
                                keys[static_cast<size_t>(key)]) {
                order[static_cast<size_t>(j)] = order[static_cast<size_t>(j - 1)];
                j--;
            }
            order[static_cast<size_t>(j)] = key;
        }

        for (size_t i = 0; i < pb.objects.size(); i++) {
            pb.objects[i].column = order[static_cast<size_t>(pb.objects[i].column)];
        }
    }
}}} // namespace pppp::mania::mods
