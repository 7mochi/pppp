// The string lookup tables of the parser (upstream builds them at compile time with
// consteval; the port builds them once at program start-up).

#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/section_names.h>
#include <fosu/engine/sections/difficulty.h>
#include <fosu/engine/sections/editor.h>
#include <fosu/engine/sections/events.h>
#include <fosu/engine/sections/general.h>
#include <fosu/engine/sections/metadata.h>

namespace fosu { namespace internal {

    namespace {

        template <typename T, size_t N>
        inline size_t entry_count(const T (&)[N]) {
            return N;
        }

        const StringEntry<Section::Value> section_entries[] = {
            {"[General]", Section::General},   {"[Editor]", Section::Editor},
            {"[Metadata]", Section::Metadata}, {"[Difficulty]", Section::Difficulty},
            {"[Events]", Section::Events},     {"[TimingPoints]", Section::TimingPoints},
            {"[Colours]", Section::Colours},   {"[HitObjects]", Section::HitObjects},
        };

        const StringEntry<fosu_int32> countdown_entries[] = {
            {"None", 0},
            {"Normal", 1},
            {"HalfSpeed", 2},
            {"DoubleSpeed", 3},
        };

        const StringEntry<fosu_int32> sample_set_entries[] = {
            {"None", 0},
            {"Normal", 1},
            {"Soft", 2},
            {"Drum", 3},
        };

        const StringEntry<FieldParser> general_entries[] = {
            {"AudioFilename", assign_field_text<&BeatmapHeader::audio_filename>},
            {"AudioLeadIn",
             assign_field_value<fosu_int32, &BeatmapHeader::audio_lead_in, parse_field_integer>},
            {"PreviewTime", parse_preview_time},
            {"CountdownOffset",
             assign_field_value<fosu_int32, &BeatmapHeader::countdown_offset, parse_field_integer>},
            {"Countdown", assign_field_value<fosu_int32, &BeatmapHeader::countdown, parse_countdown>},
            {"SampleSet",
             assign_field_value<SampleSet::Value, &BeatmapHeader::sample_set, parse_field_sample_set>},
            {"SampleVolume",
             assign_field_value<fosu_int32, &BeatmapHeader::sample_volume, parse_field_integer>},
            {"SamplesMatchPlaybackRate",
             assign_field_value<bool, &BeatmapHeader::samples_match_playback_rate, parse_field_boolean>},
            {"StackLeniency", assign_field_value<double, &BeatmapHeader::stack_leniency, parse_field_float>},
            {"Mode", assign_field_value<fosu_int32, &BeatmapHeader::mode, parse_mode>},
            {"LetterboxInBreaks",
             assign_field_value<bool, &BeatmapHeader::letterbox_in_breaks, parse_field_boolean>},
            {"WidescreenStoryboard",
             assign_field_value<bool, &BeatmapHeader::widescreen_storyboard, parse_field_boolean>},
            {"EpilepsyWarning",
             assign_field_value<bool, &BeatmapHeader::epilepsy_warning, parse_field_boolean>},
            {"SpecialStyle", assign_field_value<bool, &BeatmapHeader::special_style, parse_field_boolean>},
            {"UseSkinSprites", parse_skin_sprites},
            {"OverlayPosition", assign_field_text<&BeatmapHeader::overlay_position>},
            {"SkinPreference", assign_field_text<&BeatmapHeader::skin_preference>},
        };

        const StringEntry<FieldParser> editor_entries[] = {
            {"Bookmarks", assign_field_text<&BeatmapHeader::bookmarks>},
            {"DistanceSpacing",
             assign_field_value<double, &BeatmapHeader::distance_spacing, parse_editor_scale>},
            {"BeatDivisor", assign_field_value<fosu_int32, &BeatmapHeader::beat_divisor, parse_beat_divisor>},
            {"GridSize", assign_field_value<fosu_int32, &BeatmapHeader::grid_size, parse_field_integer>},
            {"TimelineZoom", assign_field_value<double, &BeatmapHeader::timeline_zoom, parse_editor_scale>},
        };

        const StringEntry<FieldParser> metadata_entries[] = {
            {"TitleUnicode", assign_field_text<&BeatmapHeader::title_unicode>},
            {"Title", assign_field_text<&BeatmapHeader::title>},
            {"ArtistUnicode", assign_field_text<&BeatmapHeader::artist_unicode>},
            {"Artist", assign_field_text<&BeatmapHeader::artist>},
            {"Creator", assign_field_text<&BeatmapHeader::creator>},
            {"Version", assign_field_text<&BeatmapHeader::version>},
            {"Source", assign_field_text<&BeatmapHeader::source>},
            {"Tags", assign_field_text<&BeatmapHeader::tags>},
            {"BeatmapSetID", assign_field_widened<fosu_int64, &BeatmapHeader::beatmap_set_id, fosu_int32,
                                                  parse_field_integer>},
            {"BeatmapID",
             assign_field_widened<fosu_int64, &BeatmapHeader::beatmap_id, fosu_int32, parse_field_integer>},
        };

        const StringEntry<FieldParser> difficulty_entries[] = {
            {"HPDrainRate", assign_field_value<double, &BeatmapHeader::hp, parse_difficulty_rating>},
            // parse_document clamps CS after all sections, using the final game mode.
            {"CircleSize", assign_field_value<double, &BeatmapHeader::cs, parse_field_float>},
            {"OverallDifficulty", assign_field_value<double, &BeatmapHeader::od, parse_difficulty_rating>},
            {"SliderMultiplier",
             assign_field_value<double, &BeatmapHeader::slider_multiplier, parse_slider_multiplier>},
            {"SliderTickRate",
             assign_field_value<double, &BeatmapHeader::slider_tick_rate, parse_slider_tick_rate>},
        };

        const StringEntry<EventHandler> event_entries[] = {
            {"0", parse_background_event}, {"1", parse_video_event},     {"Video", parse_video_event},
            {"2", parse_break_event},      {"Break", parse_break_event},
        };

    } // namespace

    const StringLookup<Section::Value> kSectionNames(section_entries, entry_count(section_entries));
    const StringLookup<fosu_int32> kCountdownNames(countdown_entries, entry_count(countdown_entries));
    const StringLookup<fosu_int32> kSampleSetNames(sample_set_entries, entry_count(sample_set_entries));
    const StringLookup<FieldParser> kGeneralFields(general_entries, entry_count(general_entries));
    const StringLookup<FieldParser> kEditorFields(editor_entries, entry_count(editor_entries));
    const StringLookup<FieldParser> kMetadataFields(metadata_entries, entry_count(metadata_entries));
    const StringLookup<FieldParser> kDifficultyFields(difficulty_entries, entry_count(difficulty_entries));
    const StringLookup<EventHandler> kEventHandlers(event_entries, entry_count(event_entries));

}} // namespace fosu::internal
