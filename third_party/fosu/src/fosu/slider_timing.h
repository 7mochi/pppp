#ifndef FOSU_SLIDER_TIMING_H
#define FOSU_SLIDER_TIMING_H

#include <algorithm>
#include <limits>

#include <fosu/engine/parsing/numbers.h>
#include <fosu/slider_geometry.h>

namespace fosu { namespace internal {

    struct SliderTimingChange {
        double time;
        double beat_length; // NaN means this group does not change the beat length.
        double velocity;
        size_t input_order;
        bool generate_ticks;
    };

    struct SliderTiming {
        double velocity;
        double tick_distance;
        double catch_tick_distance;
        double span_duration;
        int spans;
    };

    inline bool is_nan(double value) { return value != value; }

    struct TimingChangeOrder {
        bool operator()(const SliderTimingChange& a, const SliderTimingChange& b) const {
            return a.time < b.time || (a.time == b.time && a.input_order < b.input_order);
        }
    };

    inline bool set_slider_end_times(Beatmap& map, Arena* scratch_arena,
                                     Span<SliderTiming> timings = Span<SliderTiming>(),
                                     Span<double> stacking_end_times = Span<double>()) {
        if (map.sliders.empty()) {
            return true;
        }
        const TempArena temp(scratch_arena);
        SliderTimingChange* changes =
            arena_push_array<SliderTimingChange>(scratch_arena, map.timing_points.size());
        if (!changes && !map.timing_points.empty()) {
            return false;
        }
        size_t count = 0;
        for (size_t i = 0; i < map.timing_points.size();) {
            const double time = map.timing_points[i].time;
            const TimingPoint* red = 0;
            const TimingPoint* green = 0;
            do {
                const TimingPoint& point = map.timing_points[i++];
                if (point.uninherited) {
                    if (!red) {
                        red = &point;
                    }
                } else {
                    green = &point;
                }
            } while (i < map.timing_points.size() && map.timing_points[i].time == time);
            const TimingPoint& speed = green ? *green : *red;
            SliderTimingChange& change = changes[count++];
            change.time = time;
            change.beat_length =
                red ? clamp_value(red->beat_length, 6.0, 60000.0) : std::numeric_limits<double>::quiet_NaN();
            change.velocity = speed.beat_length < 0 ? clamp_value(100.0 / -speed.beat_length, 0.1, 10.0) : 1;
            change.input_order = i;
            change.generate_ticks = !is_nan(speed.beat_length);
        }
        bool ordered = true;
        for (size_t i = 1; i < count; ++i) {
            if (changes[i].time < changes[i - 1].time) {
                ordered = false;
                break;
            }
        }
        if (!ordered) {
            // osu! discards redundant velocity changes as it reads them. Sorting first
            // would revive a discarded point if a later line changes an earlier time.
            // Keep this input-order resolution off the usual ordered-map path.
            for (size_t i = 0; i < count; ++i) {
                double latest = -std::numeric_limits<double>::infinity();
                double velocity = 1;
                bool ticks = true;
                for (size_t j = 0; j < i; ++j) {
                    if (!is_nan(changes[j].velocity) && changes[j].time <= changes[i].time &&
                        changes[j].time >= latest) {
                        latest = changes[j].time;
                        velocity = changes[j].velocity;
                        ticks = changes[j].generate_ticks;
                    }
                }
                if (changes[i].velocity == velocity && changes[i].generate_ticks == ticks) {
                    changes[i].velocity = std::numeric_limits<double>::quiet_NaN();
                }
            }
            std::sort(changes, changes + count, TimingChangeOrder());
        }
        double beat_length = 1000, velocity = 1;
        bool generate_ticks = true;
        // The first red point supplies BPM even to objects preceding it. Green
        // points, unlike red points, never apply before their timestamp.
        for (size_t i = 0; i < count; ++i) {
            if (!is_nan(changes[i].beat_length)) {
                beat_length = changes[i].beat_length;
                for (size_t j = i + 1; j < count && changes[j].time == changes[i].time; ++j) {
                    if (!is_nan(changes[j].beat_length)) {
                        beat_length = changes[j].beat_length;
                    }
                }
                break;
            }
        }
        size_t next = 0;
        for (size_t index = 0; index < map.hit_objects.size(); ++index) {
            HitObject& object = map.hit_objects[index];
            if (object.slider == HitObject::kNoSlider) {
                continue;
            }
            while (next < count && changes[next].time <= object.time) {
                if (!is_nan(changes[next].beat_length)) {
                    beat_length = changes[next].beat_length;
                }
                if (!is_nan(changes[next].velocity)) {
                    velocity = changes[next].velocity;
                    generate_ticks = changes[next].generate_ticks;
                }
                ++next;
            }
            const Slider& slider = map.sliders[object.slider];
            const Span<const SliderPoint> control_points =
                map.slider_points.subspan(slider.point_begin, slider.point_count);
            const Span<const CurveSegment> segments =
                map.slider_segments.subspan(slider.segment_begin, slider.segment_count);
            const Result<double> distance = !map.slider_paths.empty()
                                                ? Result<double>(map.slider_paths[object.slider].distance())
                                                : slider_distance(object, slider, control_points, segments,
                                                                  scratch_arena, map.format_version >= 128);
            if (distance.failed()) {
                return false;
            }
            // osu!/catch (lazer Slider.ApplyDefaultsToSelf): Velocity = 100 * SliderMultiplier /
            // GetPrecisionAdjustedBeatLength(), where the slider-velocity beat length goes through a
            // float cast and a [10, 1000] clamp to match stable. Other modes keep the plain value.
            double pixels_per_millisecond;
            if (map.mode == 0 || map.mode == 2) {
                const double bpm_multiplier =
                    clamp_value(static_cast<float>(100 / velocity), 10.0f, 1000.0f) / 100.0;
                pixels_per_millisecond = 100 * map.slider_multiplier / (beat_length * bpm_multiplier);
            } else {
                pixels_per_millisecond = 100 * map.slider_multiplier * velocity / beat_length;
            }
            // osu! suppresses repeats on effectively zero-length paths. Keep the
            // encoded span count on Slider, but use the effective count for duration.
            const int spans = distance.value() <= 1e-7 ? 1 : slider.slides;
            object.end_time = object.time + spans * distance.value() / pixels_per_millisecond;
            if (!stacking_end_times.empty()) {
                // Standard stacking uses the osu! ruleset slider velocity, including its
                // legacy float precision adjustment. Keep the public end time as the
                // generic decoder value and expose this difference only to stacking.
                const double adjusted_multiplier =
                    clamp_value(static_cast<float>(100 / velocity), 10.0f, 1000.0f) / 100.0;
                const double adjusted_beat_length = beat_length * adjusted_multiplier;
                const double stacking_velocity = 100 * map.slider_multiplier / adjusted_beat_length;
                stacking_end_times[object.slider] =
                    object.time + spans * distance.value() / stacking_velocity;
            }
            if (!timings.empty()) {
                const double catch_tick_distance = pixels_per_millisecond * beat_length /
                                                   map.slider_tick_rate *
                                                   (map.format_version < 8 ? 1 / velocity : 1);
                const double tick_distance = generate_ticks || map.mode == 2
                                                 ? catch_tick_distance
                                                 : std::numeric_limits<double>::infinity();
                SliderTiming& timing = timings[object.slider];
                timing.velocity = pixels_per_millisecond;
                timing.tick_distance = tick_distance;
                timing.catch_tick_distance = catch_tick_distance;
                timing.span_duration = (spans * distance.value() / pixels_per_millisecond) / spans;
                timing.spans = spans;
            }
        }
        return true;
    }

}} // namespace fosu::internal

#endif
