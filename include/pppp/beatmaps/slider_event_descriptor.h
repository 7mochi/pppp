// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_BEATMAPS_SLIDER_EVENT_DESCRIPTOR_H
#define PPPP_BEATMAPS_SLIDER_EVENT_DESCRIPTOR_H

#include "pppp/utils/vector2.h"

namespace pppp { namespace beatmaps {
    /// Historically, slider's final tick (aka the place where the slider would receive a final
    /// judgement) was offset by -36 ms. Originally this was done to workaround a technical detail
    /// (unimportant), but over the years it has become an expectation of players that you don't need to
    /// hold until the true end of the slider. This very small amount of leniency makes it easier to jump
    /// away from fast sliders to the next hit object.
    ///
    /// After discussion on how this should be handled going forward, players have unanimously stated that
    /// this lenience should remain in some way. These days, this is implemented in the drawable
    /// implementation of Slider in the osu! ruleset.
    ///
    /// We need to keep the `SLIDER_EVENT_LEGACY_LAST_TICK` *only* for osu!catch conversion, which relies
    /// on it to generate tiny ticks correctly.
    const double TAIL_LENIENCY = -36;

    enum SliderEventType {
        SLIDER_EVENT_TICK = 0,

        /// Occurs just before the tail. @see TAIL_LENIENCY
        /// Should generally be ignored.
        SLIDER_EVENT_LEGACY_LAST_TICK = 1,

        SLIDER_EVENT_HEAD = 2,
        SLIDER_EVENT_TAIL = 3,
        SLIDER_EVENT_REPEAT = 4
    };

    /// Describes a point in time on a slider given special meaning.
    /// Should be used by rulesets to visualise the slider.
    struct SliderEventDescriptor {
        /// The type of event.
        SliderEventType type;

        /// The time of this event.
        double time;

        /// The zero-based index of the span. In the case of repeat sliders, this will increase after
        /// each repeat.
        int span_index;

        /// The time at which the contained span_index begins.
        double span_start_time;

        /// The progress along the slider's path at which this event occurs.
        double path_progress;

        /// The position on the path at path_progress, resolved once when the descriptor is built.
        pppp::utils::Vector2 position;
    };
}} // namespace pppp::beatmaps

#endif
