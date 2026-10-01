// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_OSU_OBJECT_OSU_PLAYFIELD_H
#define PPPP_OSU_OBJECT_OSU_PLAYFIELD_H

#include "pppp/utils/vector2.h"

namespace pppp { namespace osu { namespace object {
    const float PLAYFIELD_WIDTH = 512.0f;
    const float PLAYFIELD_HEIGHT = 384.0f;

    pppp::utils::Vector2 playfield_centre();
}}} // namespace pppp::osu::object

#endif
