// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/object/osu_playfield.h"

namespace pppp { namespace osu { namespace object {
    pppp::utils::Vector2 playfield_centre() {
        return pppp::utils::vec2(PLAYFIELD_WIDTH / 2, PLAYFIELD_HEIGHT / 2);
    }
}}} // namespace pppp::osu::object
