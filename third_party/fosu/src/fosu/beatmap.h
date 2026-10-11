#ifndef FOSU_BEATMAP_H
#define FOSU_BEATMAP_H

#include <cstring>
#include <utility>

#include <fosu/arena.h>
#include <fosu/beatmap_header.h>
#include <fosu/compiler.h>
#include <fosu/nonstd_config.h>
#include <fosu/result.h>
#include <fosu/slider_event.h>
#include <fosu/slider_path.h>

namespace fosu {
    struct HitObject {
        float x;
        float y;
        fosu::u32 type;
        fosu::u32 hitsound;
        double time;
        double end_time; // milliseconds; 0 for sliders unless calculation is requested
        fosu::u32 slider; // index into Beatmap::sliders, or kNoSlider
        bool new_combo;
        fosu::u8 combo_skip;
        nonstd::string_view hit_sample;

        // The position before stacking moved it (Beatmap::raw_positions holds the exact values).
        std::pair<float, float> raw_position(PathPoint stack_offset = PathPoint::zero()) const {
            return std::make_pair(x - stack_offset.x, y - stack_offset.y);
        }

        static const fosu::u32 kNoSlider = 0xFFFFFFFFu;

        // A type value may carry several kind bits; these mask them the way the
        // official decoder resolves one kind, so 130 (slider|hold) is a slider.
        bool is_circle() const { return (type & 1) != 0; }
        bool is_slider() const { return (type & 3) == 2; }
        bool is_spinner() const { return (type & 11) == 8; }
        bool is_hold() const { return (type & 139) == 128; }
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
        fosu::u32 degree;
        // Range relative to the owning Slider's control-point range.
        fosu::u32 point_begin;
        fosu::u32 point_count;
    };

    struct Slider {
        fosu::u32 point_begin; // range into Beatmap::slider_points
        fosu::u32 point_count;
        fosu::u32 segment_begin; // range into Beatmap::slider_segments
        fosu::u32 segment_count;
        fosu::i32 slides; // 1 = no repeats
        // The only path type for legacy sliders, and the first type for a modern
        // multi-segment path.
        CurveType::Value curve_type;
        double length; // declared pixel length; zero uses the natural path for duration
        nonstd::string_view edge_sounds;
        nonstd::string_view edge_sets;
    };

    struct TimingPoint {
        double time;
        double beat_length;
        fosu::i32 meter;
        SampleSet::Value sample_set;
        fosu::i32 sample_index;
        fosu::i32 volume;
        bool uninherited;
        fosu::u32 effects;
    };

    struct Break {
        double start;
        double end;
    };

    struct ParseStats {
        fosu::u32 fast_path_lines;
        fosu::u32 slow_path_lines;
        fosu::u32 malformed_lines;
        fosu::u32 storyboard_lines;

        ParseStats()
            : fast_path_lines(0),
              slow_path_lines(0),
              malformed_lines(0),
              storyboard_lines(0) {}
    };

    struct Stacking {
        fosu::i32 stack_height;
        PathPoint stack_offset;
    };

    struct Beatmap : BeatmapHeader {
        nonstd::span<Break> breaks;
        nonstd::span<fosu::u32> combo_colours;
        nonstd::span<TimingPoint> timing_points;
        nonstd::span<HitObject> hit_objects;
        nonstd::span<Slider> sliders;
        nonstd::span<CurveSegment> slider_segments;
        nonstd::span<SliderPoint> slider_points;
        nonstd::span<double> velocity_presets;
        // Empty unless requested; otherwise indexed identically to sliders.
        nonstd::span<SliderPath> slider_paths;
        nonstd::span<nonstd::span<SliderEvent> > slider_events;
        nonstd::span<nonstd::span<SliderEvent> > catch_slider_events;
        // Empty unless osu!standard stacking was requested; indexed by hit object.
        nonstd::span<Stacking> stacking;
        // Positions / control points as parsed, before stacking shifted them (empty unless
        // stacking ran). Exact values matter to consumers recomputing slider geometry.
        nonstd::span<PathPoint> raw_positions;
        nonstd::span<SliderPoint> raw_slider_points;
        ParseStats stats;

        // Deep copy into `destination` (arrays and strings). Fails with AllocationFailure and
        // leaves `destination` where it was.
        Result<Beatmap> copy(Arena& destination) const;

    private:
        template <typename T>
        static bool copy_array(Arena& destination, nonstd::span<T>& span) {
            if (span.empty()) {
                return true;
            }
            T* values = arena_push_array<T>(&destination, span.size());
            if (!values) {
                return false;
            }
            std::memcpy(static_cast<void*>(values), span.data(), span.size_bytes());
            span = nonstd::span<T>(values, span.size());
            return true;
        }

        static bool copy_string(Arena& destination, nonstd::string_view& text) {
            if (text.empty()) {
                return true;
            }
            char* bytes = static_cast<char*>(arena_push(&destination, text.size(), 1));
            if (!bytes) {
                return false;
            }
            std::memcpy(bytes, text.data(), text.size());
            text = nonstd::string_view(bytes, text.size());
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
