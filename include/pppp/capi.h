#ifndef PPPP_CAPI_H
#define PPPP_CAPI_H

#include <stddef.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(PPPP_C_BUILD)
#define PPPP_C_API __declspec(dllexport)
#elif defined(PPPP_C_USE_DLL)
#define PPPP_C_API __declspec(dllimport)
#else
#define PPPP_C_API
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PPPP_C_API __attribute__((visibility("default")))
#else
#define PPPP_C_API
#endif

// NOLINTBEGIN(readability-identifier-naming): the C naming convention has no namespaces, so every
// name here carries the prefix and keeps the underscores.

#if defined(_MSC_VER) && _MSC_VER < 1310
typedef __int64 pppp_int64;
typedef unsigned __int64 pppp_uint64;
#elif defined(__GNUC__)
__extension__ typedef long long pppp_int64;
__extension__ typedef unsigned long long pppp_uint64;
#else
typedef long long pppp_int64;
typedef unsigned long long pppp_uint64;
#endif

typedef int pppp_int32;
typedef unsigned int pppp_uint32;

typedef enum pppp_result {
    PPPP_OK = 0,
    PPPP_INVALID_ARGUMENT,
    PPPP_ALLOCATION,
    PPPP_PARSE,
    PPPP_NO_SLIDER_PATH_BACKEND,
    PPPP_SLIDER_PATH
} pppp_result;

/// Which ruleset a tagged attribute set belongs to. The values are the beatmap's `Mode`.
typedef enum pppp_ruleset {
    PPPP_RULESET_OSU = 0,
    PPPP_RULESET_TAIKO,
    PPPP_RULESET_CATCH,
    PPPP_RULESET_MANIA
} pppp_ruleset;

typedef enum pppp_slider_event_type {
    PPPP_SLIDER_EVENT_TICK = 0,

    /// Occurs just before the tail. Should generally be ignored.
    PPPP_SLIDER_EVENT_LEGACY_LAST_TICK = 1,

    PPPP_SLIDER_EVENT_HEAD = 2,
    PPPP_SLIDER_EVENT_TAIL = 3,
    PPPP_SLIDER_EVENT_REPEAT = 4
} pppp_slider_event_type;

/// The length of `pppp_score_info::statistics` and `::maximum_statistics`.
#define PPPP_HIT_RESULT_COUNT 18

typedef struct pppp_vector2 {
    float x;
    float y;
} pppp_vector2;

/// A representation of all top-level difficulty settings for a beatmap.
typedef struct pppp_beatmap_difficulty {
    double drain_rate;
    double circle_size;
    double overall_difficulty;
    double approach_rate;
    double slider_multiplier;
    double slider_tick_rate;
} pppp_beatmap_difficulty;

/// Describes a point in time on a slider given special meaning.
typedef struct pppp_slider_event_descriptor {
    pppp_int32 type;
    double time;
    pppp_int32 span_index;
    double span_start_time;
    double path_progress;
    pppp_vector2 position;
} pppp_slider_event_descriptor;

/// A HitObject describes an object in a Beatmap.
typedef struct pppp_hit_object {
    pppp_vector2 position;
    pppp_uint32 type;
    pppp_uint32 hitsound;
    double start_time;
    double end_time;
    pppp_int32 new_combo;
    pppp_int32 combo_offset;
    pppp_int32 slider;
} pppp_hit_object;

typedef struct pppp_timing_point {
    double time;
    double beat_length;
    pppp_int32 meter;
    pppp_int32 uninherited;
    pppp_uint32 effects;
} pppp_timing_point;

typedef struct pppp_break_period {
    double start_time;
    double end_time;
} pppp_break_period;

/// A slider together with views of the arrays it owns. Every pointer is borrowed from the beatmap
/// and stays valid until `pppp_beatmap_free`.
typedef struct pppp_slider {
    pppp_int32 slides;
    double expected_length;

    const pppp_uint32* node_sounds;
    size_t node_sound_count;

    const pppp_vector2* control_points;
    size_t control_point_count;

    const pppp_vector2* path;
    size_t path_count;

    const double* cumulative_lengths;
    size_t cumulative_length_count;

    const pppp_vector2* undecimated_path;
    size_t undecimated_path_count;

    const double* undecimated_cumulative_lengths;
    size_t undecimated_cumulative_length_count;

    const pppp_slider_event_descriptor* events;
    size_t event_count;

    const pppp_slider_event_descriptor* catch_events;
    size_t catch_event_count;
} pppp_slider;

/// The state of a play. `has_legacy_total_score` says whether `legacy_total_score` applies.
typedef struct pppp_score_info {
    pppp_int32 statistics[PPPP_HIT_RESULT_COUNT];
    pppp_int32 maximum_statistics[PPPP_HIT_RESULT_COUNT];
    pppp_int32 max_combo;
    double accuracy;
    pppp_int32 has_legacy_total_score;
    pppp_int64 legacy_total_score;
} pppp_score_info;

typedef struct pppp_osu_difficulty_attributes {
    double star_rating;
    pppp_int32 max_combo;
    double aim_difficulty;
    double speed_difficulty;
    double reading_difficulty;
    double flashlight_difficulty;
    double slider_factor;
    double aim_difficult_strain_count;
    double speed_difficult_strain_count;
    double reading_difficult_note_count;
    double aim_difficult_slider_count;
    double aim_top_weighted_slider_factor;
    double speed_top_weighted_slider_factor;
    double speed_note_count;
    pppp_int32 hit_circle_count;
    pppp_int32 slider_count;
    pppp_int32 large_tick_count;
    pppp_int32 spinner_count;
    double nested_score_per_object;
    double legacy_score_base_multiplier;
    double maximum_legacy_combo_score;
} pppp_osu_difficulty_attributes;

typedef struct pppp_taiko_difficulty_attributes {
    double star_rating;
    pppp_int32 max_combo;
    double mechanical_difficulty;
    double rhythm_difficulty;
    double reading_difficulty;
    double colour_difficulty;
    double stamina_difficulty;
    double mono_stamina_factor;
    double consistency_factor;
    double stamina_top_strains;
} pppp_taiko_difficulty_attributes;

typedef struct pppp_catch_difficulty_attributes {
    double star_rating;
    pppp_int32 max_combo;
} pppp_catch_difficulty_attributes;

typedef struct pppp_mania_difficulty_attributes {
    double star_rating;
    pppp_int32 max_combo;
} pppp_mania_difficulty_attributes;

typedef struct pppp_osu_performance_attributes {
    double total;
    double aim;
    double speed;
    double accuracy;
    double flashlight;
    double reading;
    double effective_miss_count;
    double combo_based_estimated_miss_count;
    double score_based_estimated_miss_count;
    pppp_int32 has_score_based_estimated_miss_count;
    double aim_estimated_slider_breaks;
    double speed_estimated_slider_breaks;
    double speed_deviation;
    pppp_int32 has_speed_deviation;
} pppp_osu_performance_attributes;

typedef struct pppp_taiko_performance_attributes {
    double total;
    double difficulty;
    double accuracy;
    double estimated_unstable_rate;
    pppp_int32 has_estimated_unstable_rate;
} pppp_taiko_performance_attributes;

typedef struct pppp_catch_performance_attributes {
    double total;
} pppp_catch_performance_attributes;

typedef struct pppp_mania_performance_attributes {
    double total;
    double difficulty;
} pppp_mania_performance_attributes;

/// The four rulesets' difficulty attributes in one value: a tag plus the four mode structs.
typedef struct pppp_difficulty_attributes {
    pppp_int32 ruleset;
    double star_rating;
    pppp_int32 max_combo;
    pppp_osu_difficulty_attributes osu;
    pppp_taiko_difficulty_attributes taiko;
    pppp_catch_difficulty_attributes fruits;
    pppp_mania_difficulty_attributes mania;
} pppp_difficulty_attributes;

/// The four rulesets' performance attributes in one value: a tag plus the four mode structs.
typedef struct pppp_performance_attributes {
    pppp_int32 ruleset;
    double total;
    pppp_osu_performance_attributes osu;
    pppp_taiko_performance_attributes taiko;
    pppp_catch_performance_attributes fruits;
    pppp_mania_performance_attributes mania;
} pppp_performance_attributes;

/// The input of `pppp_calculate_difficulty`. Zero-initialize it; `mods` may be null for no mods.
typedef struct pppp_difficulty_options {
    const char* mods;
    pppp_int32 ruleset;
    pppp_int32 has_ruleset;
    double clock_rate;
    pppp_int32 has_clock_rate;
} pppp_difficulty_options;

/// The input of `pppp_calculate_performance`. Zero-initialize it; `mods` may be null for no mods,
/// and `score`, `combo`, `accuracy` and `misses` only apply when their `has_*` field is set.
typedef struct pppp_performance_options {
    const char* mods;
    pppp_score_info score;
    pppp_int32 has_score;
    pppp_int32 combo;
    pppp_int32 has_combo;
    double accuracy;
    pppp_int32 has_accuracy;
    pppp_int32 misses;
    pppp_int32 has_misses;
} pppp_performance_options;

/// A loaded beatmap. It owns everything it hands out, and is freed with `pppp_beatmap_free`.
typedef struct pppp_beatmap pppp_beatmap;

#ifdef __cplusplus
extern "C" {
#endif

/// The version as `major.minor.patch`. The returned string is static; do not free it.
PPPP_C_API const char* pppp_version(void);

/// Load a beatmap from a `.osu` file path. On success `out` receives a new beatmap that the caller
/// must release with `pppp_beatmap_free`. On failure `out` is left untouched.
PPPP_C_API pppp_result pppp_beatmap_from_file(const char* path, pppp_beatmap** out);

/// Release a beatmap. Passing null is allowed and does nothing.
PPPP_C_API void pppp_beatmap_free(pppp_beatmap* map);

PPPP_C_API pppp_int32 pppp_beatmap_format_version(const pppp_beatmap* map);
PPPP_C_API pppp_int32 pppp_beatmap_mode(const pppp_beatmap* map);
PPPP_C_API double pppp_beatmap_stack_leniency(const pppp_beatmap* map);
PPPP_C_API pppp_result pppp_beatmap_get_difficulty(const pppp_beatmap* map, pppp_beatmap_difficulty* out);

/// The map's hit objects, sliders, timing points and breaks. Every returned pointer is borrowed
/// from the beatmap and stays valid until `pppp_beatmap_free`; null is written when the map has
/// none of that kind.
PPPP_C_API pppp_result pppp_beatmap_hit_objects(const pppp_beatmap* map, const pppp_hit_object** out,
                                                size_t* count);
PPPP_C_API pppp_result pppp_beatmap_sliders(const pppp_beatmap* map, const pppp_slider** out, size_t* count);
PPPP_C_API pppp_result pppp_beatmap_timing_points(const pppp_beatmap* map, const pppp_timing_point** out,
                                                  size_t* count);
PPPP_C_API pppp_result pppp_beatmap_breaks(const pppp_beatmap* map, const pppp_break_period** out,
                                           size_t* count);

/// Perform the difficulty calculation. `options` may be null for no mods, the map's own ruleset and
/// a clock rate of 1.0.
PPPP_C_API pppp_result pppp_calculate_difficulty(const pppp_beatmap* map,
                                                 const pppp_difficulty_options* options,
                                                 pppp_difficulty_attributes* out);

/// Perform the performance calculation. `options` may be null for no mods and an empty play.
PPPP_C_API pppp_result pppp_calculate_performance(const pppp_beatmap* map,
                                                  const pppp_performance_options* options,
                                                  pppp_performance_attributes* out);

#ifdef __cplusplus
} // extern "C"
#endif

// NOLINTEND(readability-identifier-naming)

#endif
