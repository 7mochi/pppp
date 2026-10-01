// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_OBJECT_MANIA_HIT_OBJECT_H
#define PPPP_MANIA_OBJECT_MANIA_HIT_OBJECT_H

#include "pppp/utils/math/csharp.h"

namespace pppp { namespace mania { namespace object {
    struct ManiaHitObject {
        int column;
        double start_time;
        double end_time;
        bool hold;

        ManiaHitObject()
            : column(0),
              start_time(0.0),
              end_time(0.0),
              hold(false) {}
    };

    inline bool hit_object_earlier(const ManiaHitObject& a, const ManiaHitObject& b) {
        return a.start_time < b.start_time;
    }

    /// Orders by start time rounded half to even.
    inline int hit_object_start_time_rounded(const ManiaHitObject& a, const ManiaHitObject& b) {
        return utils::math::round_half_even_int(a.start_time) -
               utils::math::round_half_even_int(b.start_time);
    }
}}} // namespace pppp::mania::object

namespace pppp { namespace mania {
    typedef object::ManiaHitObject ManiaHitObject;
}} // namespace pppp::mania

#endif
