// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_BEATMAP_CONVERTER_H
#define PPPP_BEATMAPS_BEATMAP_CONVERTER_H

#include <cstddef>

namespace pppp { namespace beatmaps {
    /// Invoked when a HitObject has been converted.
    /// The first argument contains the HitObject that was converted.
    /// The second argument contains the HitObjects that were output from the conversion process.
    template <typename T>
    struct ObjectConverted {
        void (*invoke)(size_t original, const T* converted, size_t count, void* context);
        void* context;

        ObjectConverted()
            : invoke(0),
              context(0) {}
    };
}} // namespace pppp::beatmaps

#endif
