// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/beatmaps/beatmap.h"

namespace pppp { namespace beatmaps {
    Beatmap::Beatmap()
        : format_version(14),
          mode(0),
          stack_leniency(0.7) {
        difficulty.drain_rate = 5.0;
        difficulty.circle_size = 5.0;
        difficulty.overall_difficulty = 5.0;
        difficulty.approach_rate = 5.0;
        difficulty.slider_multiplier = 1.4;
        difficulty.slider_tick_rate = 1.0;

        slider_path.recompute = 0;
        slider_path.release = 0;
        slider_path.ctx = 0;
    }

    Beatmap::~Beatmap() { clear(); }

    void Beatmap::clear() {
        if (slider_path.release) {
            slider_path.release(slider_path.ctx);
        }
        slider_path.recompute = 0;
        slider_path.release = 0;
        slider_path.ctx = 0;

        hit_objects.clear();
        sliders.clear();
        timing_points.clear();
        breaks.clear();
    }
}} // namespace pppp::beatmaps
