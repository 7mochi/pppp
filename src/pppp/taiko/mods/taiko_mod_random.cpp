// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/mods/taiko_mod_random.h"
#include "pppp/utils/random/csharp.h"
#include <cstddef>

namespace pppp { namespace taiko { namespace mods {
    void apply_random(TaikoBeatmap& pb, int seed) {
        utils::DotNetRandom rng(seed);

        for (size_t i = 0; i < pb.objects.size(); i++) {
            if (pb.objects[i].kind == object::OBJECT_HIT) {
                pb.objects[i].type = rng.next(2) == 0 ? object::HIT_TYPE_CENTRE : object::HIT_TYPE_RIM;
            }
        }
    }
}}} // namespace pppp::taiko::mods
