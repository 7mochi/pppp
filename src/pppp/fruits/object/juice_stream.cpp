// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/object/juice_stream.h"
#include "pppp/beatmaps/slider_path.h"

namespace pppp { namespace fruits { namespace object {
    CatchHitObject create_juice_stream(const pppp::beatmaps::Slider& slider, double head_x, double time,
                                       int slider_index) {
        CatchHitObject stream;
        stream.kind = OBJECT_JUICE_STREAM;
        stream.time = time;
        stream.original_x = head_x;
        stream.x_offset = 0.0;
        stream.distance_to_hyper_dash = 0.0;
        stream.hyper_dash = false;
        stream.slider_index = slider_index;

        const std::vector<pppp::beatmaps::SliderEventDescriptor>& events =
            slider.catch_events.empty() ? slider.events : slider.catch_events;
        bool have_last = false;
        double last_time = 0.0;
        double last_progress = 0.0;

        for (size_t i = 0; i < events.size(); i++) {
            const pppp::beatmaps::SliderEventDescriptor& e = events[i];

            // generate tiny droplets since the last point
            if (have_last) {
                double since_last_tick = static_cast<int>(e.time) - static_cast<int>(last_time);

                if (since_last_tick > 80) {
                    double time_between_tiny = since_last_tick;
                    while (time_between_tiny > 100) {
                        time_between_tiny /= 2;
                    }

                    // NOLINTNEXTLINE(cert-flp30-c)
                    for (double t = time_between_tiny; t < since_last_tick; t += time_between_tiny) {
                        double progress =
                            last_progress + (t / since_last_tick) * (e.path_progress - last_progress);

                        CatchHitObject tiny;
                        tiny.kind = OBJECT_TINY_DROPLET;
                        tiny.time = t + last_time;
                        tiny.original_x =
                            head_x + pppp::beatmaps::slider_position_at_undecimated(slider, progress).x;
                        tiny.x_offset = 0.0;
                        tiny.distance_to_hyper_dash = 0.0;
                        tiny.hyper_dash = false;
                        tiny.slider_index = -1;
                        stream.nested.push_back(tiny);
                    }
                }
            }

            have_last = true;
            // this also includes LastTick and this is used for TinyDroplet generation above.
            // this means that the final segment of TinyDroplets are increasingly mistimed where LastTick is
            // being applied.
            last_time = e.time;
            last_progress = e.path_progress;

            if (e.type == pppp::beatmaps::SLIDER_EVENT_LEGACY_LAST_TICK) {
                continue;
            }

            CatchHitObject object;
            object.kind = e.type == pppp::beatmaps::SLIDER_EVENT_TICK ? OBJECT_DROPLET : OBJECT_FRUIT;
            object.time = e.time;
            object.original_x =
                head_x + pppp::beatmaps::slider_position_at_undecimated(slider, e.path_progress).x;
            object.x_offset = 0.0;
            object.distance_to_hyper_dash = 0.0;
            object.hyper_dash = false;
            object.slider_index = -1;
            stream.nested.push_back(object);
        }

        return stream;
    }
}}} // namespace pppp::fruits::object
