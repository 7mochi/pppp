#ifndef FOSU_BEATMAP_HEADER_H
#define FOSU_BEATMAP_HEADER_H

#include <fosu/compiler.h>
#include <fosu/enums.h>
#include <fosu/nonstd_config.h>

namespace fosu {
    struct BeatmapHeader {
        int format_version;

        // [General]
        nonstd::string_view audio_filename;
        fosu::i32 audio_lead_in;
        fosu::i32 preview_time;
        fosu::i32 countdown;
        SampleSet::Value sample_set;
        fosu::i32 sample_volume;
        double stack_leniency;
        fosu::i32 mode;
        bool letterbox_in_breaks;
        bool widescreen_storyboard;
        bool epilepsy_warning;
        bool special_style;
        bool use_skin_sprites;
        bool samples_match_playback_rate;
        fosu::i32 countdown_offset;
        nonstd::string_view overlay_position;
        nonstd::string_view skin_preference;

        // [Editor]
        nonstd::string_view bookmarks; // raw comma list
        double distance_spacing;
        fosu::i32 beat_divisor;
        fosu::i32 grid_size;
        double timeline_zoom;

        // [Metadata]
        nonstd::string_view title;
        nonstd::string_view title_unicode;
        nonstd::string_view artist;
        nonstd::string_view artist_unicode;
        nonstd::string_view creator;
        nonstd::string_view version;
        nonstd::string_view source;
        nonstd::string_view tags;
        fosu::i64 beatmap_id;
        fosu::i64 beatmap_set_id;

        // [Difficulty]
        double hp;
        double cs;
        double od;
        double ar;
        double slider_multiplier;
        double slider_tick_rate;

        // [Events]
        nonstd::string_view background;
        nonstd::string_view video;

        BeatmapHeader()
            : format_version(14),
              audio_lead_in(0),
              preview_time(-1),
              countdown(1),
              sample_set(SampleSet::Normal),
              sample_volume(100),
              stack_leniency(0.7f),
              mode(0),
              letterbox_in_breaks(false),
              widescreen_storyboard(false),
              epilepsy_warning(false),
              special_style(false),
              use_skin_sprites(false),
              samples_match_playback_rate(false),
              countdown_offset(0),
              distance_spacing(1),
              beat_divisor(4),
              grid_size(0),
              timeline_zoom(1),
              beatmap_id(-1),
              beatmap_set_id(-1),
              hp(5),
              cs(5),
              od(5),
              ar(5),
              slider_multiplier(1.4),
              slider_tick_rate(1) {}
    };
} // namespace fosu

#endif
