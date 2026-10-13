// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/mods/osu_mod_hidden.h"
#include "pppp/utils/linq/csharp.h"

namespace pppp { namespace osu { namespace mods {
    namespace linq = ::pppp::utils::linq;

    bool has_hidden_body_fade(const pppp::mods::Mod* mods, size_t mod_count) {
        return linq::from(mods, mod_count)
            .any(linq::bind(&pppp::mods::Mod::id, linq::_1) == pppp::mods::MOD_HD &&
                 !linq::bind(&pppp::mods::HiddenSettings::only_fade_approach_circles,
                             linq::bind(&pppp::mods::Mod::hidden, linq::_1)));
    }
}}} // namespace pppp::osu::mods
