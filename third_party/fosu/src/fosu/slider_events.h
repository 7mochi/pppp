#ifndef FOSU_SLIDER_EVENTS_H
#define FOSU_SLIDER_EVENTS_H

// Event ordering and endpoint exclusion follow ppy's SliderEventGenerator.
// Copyright (c) ppy Pty Ltd <contact@ppy.sh>; MIT licence in slider_geometry.h.
#include <algorithm>

#include <fosu/slider_timing.h>

namespace fosu { namespace internal {

    // Builds the events of one slider (upstream: a lambda capturing the object, timing and path).
    struct SliderEventFactory {
        const HitObject* object;
        const SliderTiming* timing;
        const SliderPath* path;

        SliderEvent operator()(SliderEventType::Value type, double time, int span, double progress) const {
            SliderEvent event;
            event.type = type;
            event.time = time;
            event.span_index = span;
            event.span_start_time = object->time + span * timing->span_duration;
            event.path_progress = progress;
            event.position = slider_position_at(*path, progress);
            return event;
        }
    };

    inline bool build_slider_events(const HitObject& object, const SliderTiming& timing,
                                    const SliderPath& path, double raw_tick_distance, Arena* result_arena,
                                    size_t& remaining, Span<SliderEvent>& out) {
        const double length = std::min(100000.0, path.distance());
        const double tick_distance = clamp_value(raw_tick_distance, 0.0, length);
        const double minimum_from_end = timing.velocity * 10;
        size_t tick_count = 0;
        // Count with the same repeated addition used to place ticks, avoiding a
        // division/rounding disagreement at an exact tick or exclusion boundary.
        if (tick_distance > 0) {
            for (double d = tick_distance; d <= length && d < length - minimum_from_end; d += tick_distance) {
                ++tick_count;
            }
        }
        const size_t count = 2 + static_cast<size_t>(timing.spans) * (tick_count + 1);
        if (count > remaining) {
            return false;
        }
        remaining -= count;
        SliderEvent* events = arena_push_array<SliderEvent>(result_arena, count);
        if (!events) {
            return false;
        }
        size_t next = 0;
        SliderEventFactory event;
        event.object = &object;
        event.timing = &timing;
        event.path = &path;
        events[next++] = event(SliderEventType::Head, object.time, 0, 0);
        for (int span = 0; span < timing.spans; ++span) {
            const double start = object.time + span * timing.span_duration;
            double d = tick_distance;
            for (size_t tick = 0; tick < tick_count; ++tick, d += tick_distance) {
                const double progress = d / length;
                const double time_progress = span % 2 ? 1 - progress : progress;
                const size_t index_in_span = span % 2 ? tick_count - tick - 1 : tick;
                events[next + index_in_span] = event(
                    SliderEventType::Tick, start + time_progress * timing.span_duration, span, progress);
            }
            next += tick_count;
            if (span < timing.spans - 1) {
                events[next++] =
                    event(SliderEventType::Repeat, start + timing.span_duration, span, (span + 1) % 2);
            }
        }
        const int final_span = timing.spans - 1;
        const double final_span_start = object.time + final_span * timing.span_duration;
        const double duration = timing.spans * timing.span_duration;
        const double legacy_time =
            std::max(object.time + duration / 2, final_span_start + timing.span_duration - 36);
        double legacy_progress;
        if (timing.span_duration == 0) {
            legacy_progress = timing.spans % 2;
        } else {
            legacy_progress = (legacy_time - final_span_start) / timing.span_duration;
            if (timing.spans % 2 == 0) {
                legacy_progress = 1 - legacy_progress;
            }
        }
        events[next++] = event(SliderEventType::LegacyLastTick, legacy_time, final_span, legacy_progress);
        // SliderEventGenerator gives the tail startTime + spanCount * SpanDuration, which is
        // not Slider.EndTime: EndTime folds the span count into the division and the two differ
        // by an ulp, enough to move a catch strain time off its neighbours'.
        events[next++] = event(SliderEventType::Tail, object.time + timing.spans * timing.span_duration,
                               final_span, timing.spans % 2);
        out = Span<SliderEvent>(events, count);
        return true;
    }

    inline bool set_slider_events(Beatmap& map, Arena* result_arena, Arena* scratch_arena,
                                  Span<double> stacking_end_times = Span<double>()) {
        if (map.sliders.empty()) {
            return true;
        }
        const TempArena temp(scratch_arena);
        SliderTiming* timings = arena_push_array<SliderTiming>(scratch_arena, map.sliders.size());
        Span<SliderEvent>* ranges = arena_push_array<Span<SliderEvent> >(result_arena, map.sliders.size());
        Span<SliderEvent>* catch_ranges =
            arena_push_array<Span<SliderEvent> >(result_arena, map.sliders.size());
        if (!timings || !ranges || !catch_ranges ||
            !set_slider_end_times(map, scratch_arena, Span<SliderTiming>(timings, map.sliders.size()),
                                  stacking_end_times)) {
            return false;
        }
        size_t remaining = 1u << 20; // Bound pathological repeat/tick expansion.
        for (size_t index = 0; index < map.hit_objects.size(); ++index) {
            const HitObject& object = map.hit_objects[index];
            if (object.slider == HitObject::kNoSlider) {
                continue;
            }
            const SliderPath& path = map.slider_paths[object.slider];
            const SliderTiming& timing = timings[object.slider];
            if (!build_slider_events(object, timing, path, timing.tick_distance, result_arena, remaining,
                                     ranges[object.slider])) {
                return false;
            }
            catch_ranges[object.slider] = Span<SliderEvent>();
            if (timing.catch_tick_distance != timing.tick_distance &&
                !build_slider_events(object, timing, path, timing.catch_tick_distance, result_arena,
                                     remaining, catch_ranges[object.slider])) {
                return false;
            }
        }
        map.slider_events = Span<Span<SliderEvent> >(ranges, map.sliders.size());
        map.catch_slider_events = Span<Span<SliderEvent> >(catch_ranges, map.sliders.size());
        return true;
    }

}} // namespace fosu::internal

#endif
