// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/object/banana_shower.h"

namespace pppp { namespace fruits { namespace object {
    CatchHitObject create_banana_shower(double start_time, double end_time) {
        CatchHitObject shower;
        shower.kind = OBJECT_BANANA_SHOWER;
        shower.time = start_time;
        shower.end_time = end_time;
        shower.original_x = 0.0;
        shower.x_offset = 0.0;
        shower.distance_to_hyper_dash = 0.0;
        shower.hyper_dash = false;
        shower.slider_index = -1;

        // Int truncation added to match osu!stable.
        int start = static_cast<int>(start_time);
        int end = static_cast<int>(end_time);
        float spacing = static_cast<float>(end_time - start_time);
        while (spacing > 100) {
            spacing /= 2;
        }

        if (spacing <= 0) {
            return shower;
        }

        // NOLINTNEXTLINE(cert-flp30-c)
        for (float time = static_cast<float>(start); time <= static_cast<float>(end); time += spacing) {
            CatchHitObject banana;
            banana.kind = OBJECT_BANANA;
            banana.time = time;
            banana.end_time = banana.time;
            banana.original_x = 0.0;
            banana.x_offset = 0.0;
            banana.distance_to_hyper_dash = 0.0;
            banana.hyper_dash = false;
            banana.slider_index = -1;
            shower.nested.push_back(banana);
        }

        return shower;
    }
}}} // namespace pppp::fruits::object
