// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_OBJECT_MANIA_HIT_OBJECT_H
#define PPPP_MANIA_OBJECT_MANIA_HIT_OBJECT_H

#include "pppp/utils/math/csharp.h"

namespace pppp { namespace mania { namespace object {
    struct ManiaHitObject {
        int column;
        double start_time;
        double duration;
        bool hold;

        ManiaHitObject()
            : column(0),
              start_time(0.0),
              duration(0.0),
              hold(false) {}

        double end_time() const { return start_time + duration; }
    };

    inline double get_end_time(const ManiaHitObject& hit_object) { return hit_object.end_time(); }

    /// Orders by start time rounded half to even.
    inline int hit_object_start_time_rounded(const ManiaHitObject& a, const ManiaHitObject& b) {
        return pppp::utils::math::round_half_even_int(a.start_time) -
               pppp::utils::math::round_half_even_int(b.start_time);
    }
}}} // namespace pppp::mania::object

namespace pppp { namespace mania {
    typedef object::ManiaHitObject ManiaHitObject;
}} // namespace pppp::mania

#endif
