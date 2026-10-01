// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/mods/osu_mod_hidden.h"

namespace pppp { namespace osu { namespace mods {
    bool has_hidden_body_fade(const pppp::mods::Mod* mods, size_t mod_count) {
        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id == pppp::mods::MOD_HD && !mods[i].hidden.only_fade_approach_circles) {
                return true;
            }
        }
        return false;
    }
}}} // namespace pppp::osu::mods
