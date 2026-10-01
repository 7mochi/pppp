#ifndef FOSU_BEATMAP_HEADER_H
#define FOSU_BEATMAP_HEADER_H

#include <fosu/compiler.h>
#include <fosu/enums.h>
#include <fosu/span.h>

namespace fosu {

    struct BeatmapHeader {
        int format_version;

        // [General]
        StringView audio_filename;
        fosu_int32 audio_lead_in;
        fosu_int32 preview_time;
        fosu_int32 countdown;
        SampleSet::Value sample_set;
        fosu_int32 sample_volume;
        double stack_leniency;
        fosu_int32 mode;
        bool letterbox_in_breaks;
        bool widescreen_storyboard;
        bool epilepsy_warning;
        bool special_style;
        bool use_skin_sprites;
        bool samples_match_playback_rate;
        fosu_int32 countdown_offset;
        StringView overlay_position;
        StringView skin_preference;

        // [Editor]
        StringView bookmarks; // raw comma list
        double distance_spacing;
        fosu_int32 beat_divisor;
        fosu_int32 grid_size;
        double timeline_zoom;

        // [Metadata]
        StringView title;
        StringView title_unicode;
        StringView artist;
        StringView artist_unicode;
        StringView creator;
        StringView version;
        StringView source;
        StringView tags;
        fosu_int64 beatmap_id;
        fosu_int64 beatmap_set_id;

        // [Difficulty]
        double hp;
        double cs;
        double od;
        double ar;
        double slider_multiplier;
        double slider_tick_rate;

        // [Events]
        StringView background;
        StringView video;

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
