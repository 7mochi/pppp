// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/mods/taiko_mod_swap.h"
#include <cstddef>

namespace pppp { namespace taiko { namespace mods {
    void apply_swap(TaikoBeatmap& pb) {
        for (size_t i = 0; i < pb.objects.size(); i++) {
            if (pb.objects[i].kind == object::OBJECT_HIT) {
                pb.objects[i].type = pb.objects[i].type == object::HIT_TYPE_CENTRE ? object::HIT_TYPE_RIM
                                                                                   : object::HIT_TYPE_CENTRE;
            }
        }
    }
}}} // namespace pppp::taiko::mods
