#ifndef FOSU_SLIDER_EVENT_H
#define FOSU_SLIDER_EVENT_H

#include <fosu/compiler.h>
#include <fosu/slider_path.h>

namespace fosu {

    struct SliderEventType {
        enum Value { Head, Tick, Repeat, LegacyLastTick, Tail };
    };

    struct SliderEvent {
        SliderEventType::Value type;
        double time;
        fosu_int32 span_index;
        double span_start_time;
        double path_progress;
        PathPoint position; // Relative to the slider head, like SliderPath points.
    };

} // namespace fosu

#endif
