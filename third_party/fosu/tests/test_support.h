#ifndef FOSU_TESTS_SUPPORT_H
#define FOSU_TESTS_SUPPORT_H

// Shared helpers of the port's test suites (upstream: tests/support/test.h).

#include <cstdio>
#include <cstdlib>
#include <string>

#include <fosu/parser.h>

#include "harness.h"

inline fosu::Beatmap& require_parse(fosu::Result<fosu::Beatmap*> parsed) {
    CHECK(parsed.ok());
    if (parsed.failed()) {
        std::abort();
    }
    return *parsed.value();
}

// Parses with a parser that lives as long as the test binary: the returned beatmap is a shallow
// copy whose strings and arrays stay valid until the next parse_str call.
inline fosu::Beatmap parse_str(const std::string& s,
                               const fosu::ParseOptions& options = fosu::ParseOptions()) {
    static fosu::Parser parser;
    return require_parse(parser.parse(s.data(), s.size(), options));
}

inline std::string to_str(long value) {
    char buffer[32];
    std::sprintf(buffer, "%ld", value);
    return buffer;
}

inline bool operator==(fosu::StringView a, const std::string& b) {
    return a == fosu::StringView(b.data(), b.size());
}

// Canonical text form of every field of a Beatmap (floating point as bit patterns), so that
// equality means bit-identical parsing (upstream: tests/support/canonical_dump.h).
struct Canonical {
    std::string out;

    void raw(const void* p, size_t n) { out.append(static_cast<const char*>(p), n); }
    void f64(double v) { raw(&v, sizeof(v)); }
    void f32(float v) { raw(&v, sizeof(v)); }
    void i64(fosu_int64 v) { raw(&v, sizeof(v)); }
    void u32(fosu_uint32 v) { raw(&v, sizeof(v)); }
    void str(fosu::StringView v) {
        u32(static_cast<fosu_uint32>(v.size()));
        raw(v.data(), v.size());
    }
    void point(fosu::PathPoint p) {
        f32(p.x);
        f32(p.y);
    }
};

inline std::string canonical(const fosu::Beatmap& bm) {
    Canonical c;
    c.u32(static_cast<fosu_uint32>(bm.format_version));
    c.str(bm.audio_filename);
    c.u32(bm.audio_lead_in);
    c.u32(bm.preview_time);
    c.u32(bm.countdown);
    c.u32(bm.sample_set);
    c.u32(bm.sample_volume);
    c.f64(bm.stack_leniency);
    c.u32(bm.mode);
    c.u32(bm.letterbox_in_breaks | bm.widescreen_storyboard << 1 | bm.epilepsy_warning << 2 |
          bm.special_style << 3 | bm.use_skin_sprites << 4 | bm.samples_match_playback_rate << 5);
    c.u32(bm.countdown_offset);
    c.str(bm.overlay_position);
    c.str(bm.skin_preference);
    c.str(bm.bookmarks);
    c.f64(bm.distance_spacing);
    c.u32(bm.beat_divisor);
    c.u32(bm.grid_size);
    c.f64(bm.timeline_zoom);
    c.str(bm.title);
    c.str(bm.title_unicode);
    c.str(bm.artist);
    c.str(bm.artist_unicode);
    c.str(bm.creator);
    c.str(bm.version);
    c.str(bm.source);
    c.str(bm.tags);
    c.i64(bm.beatmap_id);
    c.i64(bm.beatmap_set_id);
    c.f64(bm.hp);
    c.f64(bm.cs);
    c.f64(bm.od);
    c.f64(bm.ar);
    c.f64(bm.slider_multiplier);
    c.f64(bm.slider_tick_rate);
    c.str(bm.background);
    c.str(bm.video);
    c.u32(static_cast<fosu_uint32>(bm.breaks.size()));
    for (size_t i = 0; i < bm.breaks.size(); i++) {
        c.f64(bm.breaks[i].start);
        c.f64(bm.breaks[i].end);
    }
    c.u32(static_cast<fosu_uint32>(bm.combo_colours.size()));
    for (size_t i = 0; i < bm.combo_colours.size(); i++) {
        c.u32(bm.combo_colours[i]);
    }
    c.u32(static_cast<fosu_uint32>(bm.timing_points.size()));
    for (size_t i = 0; i < bm.timing_points.size(); i++) {
        const fosu::TimingPoint& t = bm.timing_points[i];
        c.f64(t.time);
        c.f64(t.beat_length);
        c.u32(t.meter);
        c.u32(t.sample_set);
        c.u32(t.sample_index);
        c.u32(t.volume);
        c.u32(t.uninherited);
        c.u32(t.effects);
    }
    c.u32(static_cast<fosu_uint32>(bm.hit_objects.size()));
    for (size_t i = 0; i < bm.hit_objects.size(); i++) {
        const fosu::HitObject& h = bm.hit_objects[i];
        c.f32(h.x);
        c.f32(h.y);
        c.u32(h.type);
        c.u32(h.hitsound);
        c.f64(h.time);
        c.f64(h.end_time);
        c.u32(h.slider);
        c.u32(h.new_combo | h.combo_skip << 1);
        c.str(h.hit_sample);
    }
    c.u32(static_cast<fosu_uint32>(bm.sliders.size()));
    for (size_t i = 0; i < bm.sliders.size(); i++) {
        const fosu::Slider& s = bm.sliders[i];
        c.u32(s.point_begin);
        c.u32(s.point_count);
        c.u32(s.segment_begin);
        c.u32(s.segment_count);
        c.u32(s.slides);
        c.u32(s.curve_type);
        c.f64(s.length);
        c.str(s.edge_sounds);
        c.str(s.edge_sets);
    }
    c.u32(static_cast<fosu_uint32>(bm.slider_segments.size()));
    for (size_t i = 0; i < bm.slider_segments.size(); i++) {
        const fosu::CurveSegment& s = bm.slider_segments[i];
        c.u32(s.type);
        c.u32(s.has_degree ? s.degree : 0xFFFFFFFFu);
        c.u32(s.point_begin);
        c.u32(s.point_count);
    }
    c.u32(static_cast<fosu_uint32>(bm.slider_points.size()));
    for (size_t i = 0; i < bm.slider_points.size(); i++) {
        c.f32(bm.slider_points[i].x);
        c.f32(bm.slider_points[i].y);
    }
    c.u32(static_cast<fosu_uint32>(bm.velocity_presets.size()));
    for (size_t i = 0; i < bm.velocity_presets.size(); i++) {
        c.f64(bm.velocity_presets[i]);
    }
    c.u32(static_cast<fosu_uint32>(bm.slider_paths.size()));
    for (size_t i = 0; i < bm.slider_paths.size(); i++) {
        const fosu::SliderPath& path = bm.slider_paths[i];
        c.u32(static_cast<fosu_uint32>(path.points.size()));
        for (size_t j = 0; j < path.points.size(); j++) {
            c.point(path.points[j]);
            c.f64(path.cumulative_lengths[j]);
        }
    }
    c.u32(static_cast<fosu_uint32>(bm.slider_events.size()));
    for (size_t i = 0; i < bm.slider_events.size(); i++) {
        c.u32(static_cast<fosu_uint32>(bm.slider_events[i].size()));
        for (size_t j = 0; j < bm.slider_events[i].size(); j++) {
            const fosu::SliderEvent& e = bm.slider_events[i][j];
            c.u32(e.type);
            c.f64(e.time);
            c.u32(e.span_index);
            c.f64(e.span_start_time);
            c.f64(e.path_progress);
            c.point(e.position);
        }
    }
    c.u32(static_cast<fosu_uint32>(bm.stacking.size()));
    for (size_t i = 0; i < bm.stacking.size(); i++) {
        c.u32(bm.stacking[i].stack_height);
        c.point(bm.stacking[i].stack_offset);
    }
    c.u32(static_cast<fosu_uint32>(bm.raw_positions.size()));
    for (size_t i = 0; i < bm.raw_positions.size(); i++) {
        c.point(bm.raw_positions[i]);
    }
    c.u32(static_cast<fosu_uint32>(bm.raw_slider_points.size()));
    for (size_t i = 0; i < bm.raw_slider_points.size(); i++) {
        c.f32(bm.raw_slider_points[i].x);
        c.f32(bm.raw_slider_points[i].y);
    }
    c.u32(bm.stats.fast_path_lines);
    c.u32(bm.stats.slow_path_lines);
    c.u32(bm.stats.malformed_lines);
    c.u32(bm.stats.storyboard_lines);
    return c.out;
}

#endif
