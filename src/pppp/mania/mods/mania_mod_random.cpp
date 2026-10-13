// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/mods/mania_mod_random.h"
#include "pppp/utils/linq/csharp.h"
#include "pppp/utils/random/csharp.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace mods {
    namespace linq = ::pppp::utils::linq;

    void apply_random(ManiaBeatmap& pb, int seed) {
        utils::DotNetRandom rng(seed);

        int available_columns = pb.total_columns();
        const std::vector<int> shuffled_columns =
            linq::range(0, available_columns)
                .order_by(
                    linq::bind(static_cast<int (utils::DotNetRandom::*)()>(&utils::DotNetRandom::next), &rng))
                .to_list();

        for (size_t i = 0; i < pb.objects.size(); i++) {
            pb.objects[i].column = shuffled_columns[static_cast<size_t>(pb.objects[i].column)];
        }
    }
}}} // namespace pppp::mania::mods
