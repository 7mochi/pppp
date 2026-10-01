// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_LEGACY_HIT_OBJECT_TYPE_H
#define PPPP_BEATMAPS_LEGACY_HIT_OBJECT_TYPE_H

namespace pppp { namespace beatmaps {
    enum LegacyHitObjectType { slider = 1 << 1, spinner = 1 << 3, hold = 1 << 7 };
}} // namespace pppp::beatmaps

#endif
