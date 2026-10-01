#ifndef FOSU_BEATMAP_H
#define FOSU_BEATMAP_H

#include <cstring>
#include <utility>

#include <fosu/arena.h>
#include <fosu/beatmap_header.h>
#include <fosu/compiler.h>
#include <fosu/result.h>
#include <fosu/slider_event.h>
#include <fosu/slider_path.h>
#include <fosu/span.h>

namespace fosu {

    struct HitObject {
        float x;
        float y;
        fosu_uint32 type;
        fosu_uint32 hitsound;
        double time;
        double end_time; // milliseconds; 0 for sliders unless calculation is requested
        fosu_uint32 slider; // index into Beatmap::sliders, or kNoSlider
        bool new_combo;
        fosu_uint8 combo_skip;
        StringView hit_sample;

        // The position before stacking moved it (Beatmap::raw_positions holds the exact values).
        std::pair<float, float> raw_position(PathPoint stack_offset = PathPoint::zero()) const {
            return std::make_pair(x - stack_offset.x, y - stack_offset.y);
        }

        static const fosu_uint32 kNoSlider = 0xFFFFFFFFu;

        bool is_circle() const { return (type & 1) != 0; }
        bool is_slider() const { return (type & 2) != 0; }
        bool is_spinner() const { return (type & 8) != 0; }
        bool is_hold() const { return (type & 128) != 0; }
        bool is_new_combo() const { return new_combo; }
    };

    struct SliderPoint {
        float x;
        float y;
    };

    struct CurveSegment {
        CurveType::Value type;
        // Present only for an explicit lazer B-spline degree (for example, B2).
        // An absent degree on Bezier means an ordinary Bezier segment.
        bool has_degree;
        fosu_uint32 degree;
        // Range relative to the owning Slider's control-point range.
        fosu_uint32 point_begin;
        fosu_uint32 point_count;
    };

    struct Slider {
        fosu_uint32 point_begin; // range into Beatmap::slider_points
        fosu_uint32 point_count;
        fosu_uint32 segment_begin; // range into Beatmap::slider_segments
        fosu_uint32 segment_count;
        fosu_int32 slides; // 1 = no repeats
        // The only path type for legacy sliders, and the first type for a modern
        // multi-segment path.
        CurveType::Value curve_type;
        double length; // declared pixel length; zero uses the natural path for duration
        StringView edge_sounds;
        StringView edge_sets;
    };

    struct TimingPoint {
        double time;
        double beat_length;
        fosu_int32 meter;
        SampleSet::Value sample_set;
        fosu_int32 sample_index;
        fosu_int32 volume;
        bool uninherited;
        fosu_uint32 effects;
    };

    struct Break {
        double start;
        double end;
    };

    struct ParseStats {
        fosu_uint32 fast_path_lines;
        fosu_uint32 slow_path_lines;
        fosu_uint32 malformed_lines;
        fosu_uint32 storyboard_lines;

        ParseStats()
            : fast_path_lines(0),
              slow_path_lines(0),
              malformed_lines(0),
              storyboard_lines(0) {}
    };

    struct Stacking {
        fosu_int32 stack_height;
        PathPoint stack_offset;
    };

    struct Beatmap : BeatmapHeader {
        Span<Break> breaks;
        Span<fosu_uint32> combo_colours;
        Span<TimingPoint> timing_points;
        Span<HitObject> hit_objects;
        Span<Slider> sliders;
        Span<CurveSegment> slider_segments;
        Span<SliderPoint> slider_points;
        Span<double> velocity_presets;
        // Empty unless requested; otherwise indexed identically to sliders.
        Span<SliderPath> slider_paths;
        Span<Span<SliderEvent> > slider_events;
        Span<Span<SliderEvent> > catch_slider_events;
        // Empty unless osu!standard stacking was requested; indexed by hit object.
        Span<Stacking> stacking;
        // Positions / control points as parsed, before stacking shifted them (empty unless
        // stacking ran). Exact values matter to consumers recomputing slider geometry.
        Span<PathPoint> raw_positions;
        Span<SliderPoint> raw_slider_points;
        ParseStats stats;

        // Deep copy into `destination` (arrays and strings). Fails with AllocationFailure and
        // leaves `destination` where it was.
        Result<Beatmap> copy(Arena& destination) const;

    private:
        template <typename T>
        static bool copy_array(Arena& destination, Span<T>& span) {
            if (span.empty()) {
                return true;
            }
            T* values = arena_push_array<T>(&destination, span.size());
            if (!values) {
                return false;
            }
            std::memcpy(values, span.data(), span.size_bytes());
            span = Span<T>(values, span.size());
            return true;
        }

        static bool copy_string(Arena& destination, StringView& text) {
            if (text.empty()) {
                return true;
            }
            char* bytes = static_cast<char*>(arena_push(&destination, text.size(), 1));
            if (!bytes) {
                return false;
            }
            std::memcpy(bytes, text.data(), text.size());
            text = StringView(bytes, text.size());
            return true;
        }
    };

    inline Result<Beatmap> Beatmap::copy(Arena& destination) const {
        const size_t checkpoint = arena_pos(&destination);
        Beatmap result(*this); // shallow: spans and strings still point into this
        bool ok =
            copy_array(destination, result.breaks) && copy_array(destination, result.combo_colours) &&
            copy_array(destination, result.timing_points) && copy_array(destination, result.hit_objects) &&
            copy_array(destination, result.sliders) && copy_array(destination, result.slider_segments) &&
            copy_array(destination, result.slider_points) &&
            copy_array(destination, result.velocity_presets) &&
            copy_array(destination, result.slider_paths) && copy_array(destination, result.slider_events) &&
            copy_array(destination, result.catch_slider_events) && copy_array(destination, result.stacking) &&
            copy_array(destination, result.raw_positions) &&
            copy_array(destination, result.raw_slider_points) &&
            copy_string(destination, result.audio_filename) &&
            copy_string(destination, result.overlay_position) &&
            copy_string(destination, result.skin_preference) && copy_string(destination, result.bookmarks) &&
            copy_string(destination, result.title) && copy_string(destination, result.title_unicode) &&
            copy_string(destination, result.artist) && copy_string(destination, result.artist_unicode) &&
            copy_string(destination, result.creator) && copy_string(destination, result.version) &&
            copy_string(destination, result.source) && copy_string(destination, result.tags) &&
            copy_string(destination, result.background) && copy_string(destination, result.video);
        for (size_t i = 0; ok && i < result.hit_objects.size(); i++) {
            ok = copy_string(destination, result.hit_objects[i].hit_sample);
        }
        for (size_t i = 0; ok && i < result.sliders.size(); i++) {
            ok = copy_string(destination, result.sliders[i].edge_sounds) &&
                 copy_string(destination, result.sliders[i].edge_sets);
        }
        for (size_t i = 0; ok && i < result.slider_events.size(); i++) {
            ok = copy_array(destination, result.slider_events[i]);
        }
        for (size_t i = 0; ok && i < result.catch_slider_events.size(); i++) {
            ok = copy_array(destination, result.catch_slider_events[i]);
        }
        for (size_t i = 0; ok && i < result.slider_paths.size(); i++) {
            ok = copy_array(destination, result.slider_paths[i].points) &&
                 copy_array(destination, result.slider_paths[i].cumulative_lengths) &&
                 copy_array(destination, result.slider_paths[i].undecimated_points) &&
                 copy_array(destination, result.slider_paths[i].undecimated_cumulative_lengths);
        }
        if (!ok) {
            arena_pop_to(&destination, checkpoint);
            return Error(ErrorCode::AllocationFailure);
        }
        return result;
    }

} // namespace fosu

#endif
