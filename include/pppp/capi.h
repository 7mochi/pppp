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

#if defined(__GNUC__)
#define PPPP_C_EXTENSION __extension__
#else
#define PPPP_C_EXTENSION
#endif

#define PPPP_C_ABI_VERSION 1

// NOLINTBEGIN(readability-identifier-naming): the C naming convention has no namespaces, so every
// name here starts with the prefix and keeps the underscores.

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
    PPPP_SLIDER_PATH,
    PPPP_INTERNAL,
    PPPP_NOT_PREPARED
} pppp_result;

/// The ruleset an attribute set belongs to. The values are the beatmap's `Mode`.
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

/// A two-dimensional point.
typedef struct pppp_vector2 {
    /// The x coordinate.
    float x;
    /// The y coordinate.
    float y;
} pppp_vector2;

/// A representation of all top-level difficulty settings for a beatmap.
typedef struct pppp_beatmap_difficulty {
    /// The drain rate (HP drain).
    double drain_rate;
    /// The circle size.
    double circle_size;
    /// The overall difficulty.
    double overall_difficulty;
    /// The approach rate.
    double approach_rate;
    /// The slider velocity multiplier.
    double slider_multiplier;
    /// The number of slider ticks per beat.
    double slider_tick_rate;
} pppp_beatmap_difficulty;

/// Describes a point in time on a slider given special meaning.
typedef struct pppp_slider_event_descriptor {
    /// The event type.
    pppp_int32 type;
    /// The time of the event relative to the slider start.
    double time;
    /// The index of the slider span the event belongs to.
    pppp_int32 span_index;
    /// The start time of that span.
    double span_start_time;
    /// The progress along the slider path at the event.
    double path_progress;
    /// The position of the event on the playfield.
    pppp_vector2 position;
} pppp_slider_event_descriptor;

/// A HitObject describes an object in a Beatmap.
typedef struct pppp_hit_object {
    /// The position of the object on the playfield.
    pppp_vector2 position;
    /// The object type bitfield.
    pppp_uint32 type;
    /// The hitsound bitfield.
    pppp_uint32 hitsound;
    /// The start time in milliseconds.
    double start_time;
    /// The end time in milliseconds.
    double end_time;
    /// Whether the object starts a new combo.
    pppp_int32 new_combo;
    /// The combo colour offset.
    pppp_int32 combo_offset;
    /// The index of the object's slider, or -1 if it is not a slider.
    pppp_int32 slider;
} pppp_hit_object;

/// A timing point from the beatmap.
typedef struct pppp_timing_point {
    /// The time of the timing point in milliseconds.
    double time;
    /// The beat length, or the slider velocity for inherited points.
    double beat_length;
    /// The number of beats per measure.
    pppp_int32 meter;
    /// Whether this is an uninherited timing point.
    pppp_int32 uninherited;
    /// The effects bitfield.
    pppp_uint32 effects;
} pppp_timing_point;

/// A break in the beatmap.
typedef struct pppp_break_period {
    /// The start time of the break in milliseconds.
    double start_time;
    /// The end time of the break in milliseconds.
    double end_time;
} pppp_break_period;

/// A slider and the arrays that describe it. Every pointer belongs to the beatmap and stays valid
/// until `pppp_beatmap_free`.
typedef struct pppp_slider {
    /// The number of slides.
    pppp_int32 slides;
    /// The expected length of the slider.
    double expected_length;

    /// The hitsound of each node.
    const pppp_uint32* node_sounds;
    size_t node_sound_count;

    /// The control points that define the path.
    const pppp_vector2* control_points;
    size_t control_point_count;

    /// The decimated points of the path.
    const pppp_vector2* path;
    size_t path_count;

    /// The cumulative length at each path point.
    const double* cumulative_lengths;
    size_t cumulative_length_count;

    /// The undecimated points of the path.
    const pppp_vector2* undecimated_path;
    size_t undecimated_path_count;

    /// The cumulative length at each undecimated path point.
    const double* undecimated_cumulative_lengths;
    size_t undecimated_cumulative_length_count;

    /// The legacy events on the slider.
    const pppp_slider_event_descriptor* events;
    size_t event_count;

    /// The events used by the osu!catch conversion.
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

/// Difficulty attributes for osu!standard mode.
typedef struct pppp_osu_difficulty_attributes {
    /// The difficulty corresponding to the aim skill.
    double aim_difficulty;
    /// The difficulty corresponding to the speed skill.
    double speed_difficulty;
    /// The difficulty corresponding to the reading skill.
    double reading_difficulty;
    /// The difficulty corresponding to the flashlight skill.
    double flashlight_difficulty;
    /// How much of the aim difficulty is contributed by hitcircles or sliders. A value closer to 1.0
    /// indicates mostly hitcircles; a value closer to 0.0 indicates mostly sliders.
    double slider_factor;
    /// The number of difficult aim strains.
    double aim_difficult_strain_count;
    /// The number of difficult speed strains.
    double speed_difficult_strain_count;
    /// The number of difficult reading notes.
    double reading_difficult_note_count;
    /// The number of sliders weighted by difficulty.
    double aim_difficult_slider_count;
    /// How much of the aim difficult strain count is contributed by hitcircles or sliders.
    double aim_top_weighted_slider_factor;
    /// How much of the speed difficult strain count is contributed by hitcircles or sliders.
    double speed_top_weighted_slider_factor;
    /// The number of clickable objects weighted by difficulty.
    double speed_note_count;
    /// The number of hitcircles in the beatmap.
    pppp_int32 hit_circle_count;
    /// The number of sliders in the beatmap.
    pppp_int32 slider_count;
    /// The number of large ticks in the beatmap.
    pppp_int32 large_tick_count;
    /// The number of spinners in the beatmap.
    pppp_int32 spinner_count;
    /// The nested score per object.
    double nested_score_per_object;
    /// The base multiplier for legacy scoring.
    double legacy_score_base_multiplier;
    /// The maximum legacy combo score.
    double maximum_legacy_combo_score;
} pppp_osu_difficulty_attributes;

/// Difficulty attributes for osu!taiko mode.
typedef struct pppp_taiko_difficulty_attributes {
    /// The difficulty corresponding to the mechanical skills in osu!taiko, combining colour and stamina.
    double mechanical_difficulty;
    /// The difficulty corresponding to the rhythm skill.
    double rhythm_difficulty;
    /// The difficulty corresponding to the reading skill.
    double reading_difficulty;
    /// The difficulty corresponding to the colour skill.
    double colour_difficulty;
    /// The difficulty corresponding to the stamina skill.
    double stamina_difficulty;
    /// The ratio of stamina difficulty from single-colour streams to total stamina difficulty.
    double mono_stamina_factor;
    /// The factor corresponding to the consistency of a map.
    double consistency_factor;
    /// The top stamina strains.
    double stamina_top_strains;
} pppp_taiko_difficulty_attributes;

/// Performance attributes for osu!standard mode.
typedef struct pppp_osu_performance_attributes {
    /// The aim portion of the final performance points.
    double aim;
    /// The speed portion of the final performance points.
    double speed;
    /// The accuracy portion of the final performance points.
    double accuracy;
    /// The flashlight portion of the final performance points.
    double flashlight;
    /// The reading portion of the final performance points.
    double reading;
    /// Misses including an approximated amount of slider breaks.
    double effective_miss_count;
    /// Estimated miss count based on the player's combo.
    double combo_based_estimated_miss_count;
    /// Estimated miss count based on the player's score, if available.
    double score_based_estimated_miss_count;
    pppp_int32 has_score_based_estimated_miss_count;
    /// Estimated number of slider breaks that affected the aim portion.
    double aim_estimated_slider_breaks;
    /// Estimated number of slider breaks that affected the speed portion.
    double speed_estimated_slider_breaks;
    /// Approximated unstable rate, if available.
    double speed_deviation;
    pppp_int32 has_speed_deviation;
} pppp_osu_performance_attributes;

/// Performance attributes for osu!taiko mode.
typedef struct pppp_taiko_performance_attributes {
    /// The difficulty portion of the final performance points.
    double difficulty;
    /// The accuracy portion of the final performance points.
    double accuracy;
    /// Approximated unstable rate, if available.
    double estimated_unstable_rate;
    pppp_int32 has_estimated_unstable_rate;
} pppp_taiko_performance_attributes;

/// Performance attributes for osu!mania mode.
typedef struct pppp_mania_performance_attributes {
    /// The difficulty portion of the final performance points.
    double difficulty;
} pppp_mania_performance_attributes;

/// Base class for difficulty attributes, as output by the difficulty calculator.
typedef struct pppp_difficulty_attributes {
    /// The ruleset of these attributes.
    pppp_int32 ruleset;
    /// The combined star rating of all skills.
    double star_rating;
    /// The maximum achievable combo.
    pppp_int32 max_combo;
    PPPP_C_EXTENSION union {
        pppp_osu_difficulty_attributes osu;
        pppp_taiko_difficulty_attributes taiko;
    };
} pppp_difficulty_attributes;

/// Base class for performance attributes.
typedef struct pppp_performance_attributes {
    /// The ruleset of these attributes.
    pppp_int32 ruleset;
    /// The final calculated performance points.
    double total;
    PPPP_C_EXTENSION union {
        pppp_osu_performance_attributes osu;
        pppp_taiko_performance_attributes taiko;
        pppp_mania_performance_attributes mania;
    };
} pppp_performance_attributes;

/// Wraps a `pppp_difficulty_attributes` and adds a time value for which the attribute is valid.
/// Output by `pppp_calculate_timed_difficulty`.
typedef struct pppp_timed_difficulty_attributes {
    /// The non-clock-adjusted time value at which the attributes take effect.
    double time;

    /// The attributes.
    pppp_difficulty_attributes attributes;
} pppp_timed_difficulty_attributes;

/// The result of `pppp_calculate_timed_difficulty`. Free it with `pppp_timed_difficulty_free`.
typedef struct pppp_timed_difficulty pppp_timed_difficulty;

/// The result of `pppp_calculate_strains`. Free it with `pppp_strains_free`.
typedef struct pppp_strains pppp_strains;

/// The series of an osu! strain graph, taken from a `pppp_strains`. An empty series has a null
/// pointer and a count of 0.
typedef struct pppp_osu_strains {
    /// The aim strains.
    const double* aim;
    size_t aim_count;
    /// The aim strains without sliders.
    const double* aim_no_sliders;
    size_t aim_no_sliders_count;
    /// The speed strains.
    const double* speed;
    size_t speed_count;
    /// The reading strains.
    const double* reading;
    size_t reading_count;
    /// The flashlight strains.
    const double* flashlight;
    size_t flashlight_count;
} pppp_osu_strains;

/// The strain series of an osu!taiko map, suitable for plotting difficulty over time.
typedef struct pppp_taiko_strains {
    /// The colour strains.
    const double* colour;
    size_t colour_count;
    /// The reading strains.
    const double* reading;
    size_t reading_count;
    /// The rhythm strains.
    const double* rhythm;
    size_t rhythm_count;
    /// The stamina strains.
    const double* stamina;
    size_t stamina_count;
    /// The single-colour stamina strains.
    const double* single_colour_stamina;
    size_t single_colour_stamina_count;
} pppp_taiko_strains;

/// The strain series of an osu!catch map, suitable for plotting difficulty over time.
typedef struct pppp_catch_strains {
    /// The movement strains.
    const double* movement;
    size_t movement_count;
} pppp_catch_strains;

/// The strain series of an osu!mania map, suitable for plotting difficulty over time.
typedef struct pppp_mania_strains {
    /// The strains.
    const double* strain;
    size_t strain_count;
} pppp_mania_strains;

/// A parsed mod set, made with `pppp_mods_parse` or `pppp_mods_from_legacy` and freed with
/// `pppp_mods_free`.
typedef struct pppp_mods pppp_mods;

/// The input of `pppp_calculate_difficulty`. The first field is `version`, set to
/// `PPPP_DIFFICULTY_OPTIONS_VERSION` (or use `PPPP_DIFFICULTY_OPTIONS_INIT`); zero the rest.
/// `mods` may be null for no mods.
typedef struct pppp_difficulty_options {
    pppp_uint32 version;
    const pppp_mods* mods;
    pppp_int32 ruleset;
    pppp_int32 has_ruleset;
    double clock_rate;
    pppp_int32 has_clock_rate;
} pppp_difficulty_options;

/// The input of `pppp_calculate_performance`. The first field is `version`, set to
/// `PPPP_PERFORMANCE_OPTIONS_VERSION` (or use `PPPP_PERFORMANCE_OPTIONS_INIT`); zero the rest.
/// `mods` may be null for no mods, and `score`, `combo`, `accuracy` and `misses` only apply when
/// their `has_*` field is set.
/// When `difficulty` is given, `has_difficulty` must be set, the attributes are used as they came
/// out of `pppp_calculate_difficulty` and the difficulty is not calculated again; the performance
/// is then calculated for the ruleset those attributes belong to.
typedef struct pppp_performance_options {
    pppp_uint32 version;
    const pppp_mods* mods;
    pppp_score_info score;
    pppp_int32 has_score;
    pppp_int32 combo;
    pppp_int32 has_combo;
    double accuracy;
    pppp_int32 has_accuracy;
    pppp_int32 misses;
    pppp_int32 has_misses;
    const pppp_difficulty_attributes* difficulty;
    pppp_int32 has_difficulty;
} pppp_performance_options;

#define PPPP_DIFFICULTY_OPTIONS_VERSION 1
#define PPPP_PERFORMANCE_OPTIONS_VERSION 1
/// The INIT macros set the version and zero every other field.
#define PPPP_DIFFICULTY_OPTIONS_INIT {PPPP_DIFFICULTY_OPTIONS_VERSION, 0, 0, 0, 0.0, 0}
#define PPPP_PERFORMANCE_OPTIONS_INIT                                                                        \
    {PPPP_PERFORMANCE_OPTIONS_VERSION, 0, {{0}, {0}, 0, 0.0, 0, 0}, 0, 0, 0, 0.0, 0, 0, 0, 0, 0}

/// A loaded beatmap. Everything it returns belongs to it; free the beatmap with
/// `pppp_beatmap_free`.
typedef struct pppp_beatmap pppp_beatmap;

#ifdef __cplusplus
extern "C" {
#endif

/// The version as `major.minor.patch`. The returned string is static; do not free it.
PPPP_C_API const char* pppp_version(void);

/// The C ABI version. A binding compiled against this header can compare it with the loaded library.
PPPP_C_API pppp_int32 pppp_abi_version(void);

/// The parsing engine in use: `"scalar"`, `"avx2"` or `"neon"`. The returned string is static; do not
/// free it.
PPPP_C_API const char* pppp_backend(void);

/// The message of a result, in English. The returned string is static; do not free it.
PPPP_C_API const char* pppp_result_message(pppp_result result);

/// Parse osu!'s own mod specification list (`"HD,DT:speed_change=1.4"`). On success `out`
/// receives a new mod set that the caller must release with `pppp_mods_free`. On failure `out` is
/// left untouched.
PPPP_C_API pppp_result pppp_mods_parse(const char* specification, pppp_mods** out);

/// Convert osu!stable's mod bitmask, classic is not added.
PPPP_C_API pppp_result pppp_mods_from_legacy(pppp_uint32 bits, pppp_int32 ruleset, pppp_mods** out);

/// Add the Classic mod, for scores that osu!stable set and that should calculate the way stable
/// does. Adding it twice is allowed and does nothing the second time; null fails with
/// `PPPP_INVALID_ARGUMENT`.
PPPP_C_API pppp_result pppp_mods_add_classic(pppp_mods* mods);

/// Release a mod set. Passing null is allowed and does nothing.
PPPP_C_API void pppp_mods_free(pppp_mods* mods);

/// Load a beatmap from a `.osu` file path. On success `out` receives a new beatmap that the caller
/// must release with `pppp_beatmap_free`. On failure `out` is left untouched. The loaded beatmap has
/// no data views yet; call `pppp_beatmap_prepare_data` before reading them. The calculations do not
/// need it.
PPPP_C_API pppp_result pppp_beatmap_from_file(const char* path, pppp_beatmap** out);

/// Load a beatmap from the contents of a `.osu` file already in memory, as `pppp_beatmap_from_file`
/// does from a path. The loaded beatmap has no data views yet; call `pppp_beatmap_prepare_data`
/// before reading them. The calculations do not need it.
PPPP_C_API pppp_result pppp_beatmap_from_bytes(const void* data, size_t size, pppp_beatmap** out);

/// Release a beatmap. Passing null is allowed and does nothing.
PPPP_C_API void pppp_beatmap_free(pppp_beatmap* map);

/// Build the beatmap's hit objects, sliders, timing points and breaks. Idempotent. Not
/// thread-safe: call it before sharing the handle. The calculations do not need it.
PPPP_C_API pppp_result pppp_beatmap_prepare_data(pppp_beatmap* map);

PPPP_C_API pppp_int32 pppp_beatmap_format_version(const pppp_beatmap* map);
PPPP_C_API pppp_int32 pppp_beatmap_mode(const pppp_beatmap* map);
PPPP_C_API double pppp_beatmap_stack_leniency(const pppp_beatmap* map);
PPPP_C_API pppp_result pppp_beatmap_get_difficulty(const pppp_beatmap* map, pppp_beatmap_difficulty* out);

/// The map's hit objects, sliders, timing points and breaks. Every returned pointer belongs to the
/// beatmap and stays valid until `pppp_beatmap_free`; null is written when the map has none of that
/// kind. Each of the four fails with `PPPP_NOT_PREPARED` if `pppp_beatmap_prepare_data` has not been
/// called.
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

/// Calculates the difficulty of the beatmap using a specific mod combination and returns a set of
/// TimedDifficultyAttributes representing the difficulty at every relevant time value in the
/// beatmap. The options are `pppp_calculate_difficulty`'s. On success `out` receives a new result
/// that the caller must release with `pppp_timed_difficulty_free`. On failure `out` is left
/// untouched.
PPPP_C_API pppp_result pppp_calculate_timed_difficulty(const pppp_beatmap* map,
                                                       const pppp_difficulty_options* options,
                                                       pppp_timed_difficulty** out);

/// The entries of a timed result. The returned pointer belongs to the result and stays valid until
/// `pppp_timed_difficulty_free`; null is written when there are none.
PPPP_C_API pppp_result pppp_timed_difficulty_entries(const pppp_timed_difficulty* timed,
                                                     const pppp_timed_difficulty_attributes** out,
                                                     size_t* count);

/// Release a timed result. Passing null is allowed and does nothing.
PPPP_C_API void pppp_timed_difficulty_free(pppp_timed_difficulty* timed);

/// Perform the difficulty calculation but instead of evaluating the skill strains, return them as
/// is. The options are `pppp_calculate_difficulty`'s. On success `out` receives a new result
/// that the caller must release with `pppp_strains_free`. On failure `out` is left untouched.
PPPP_C_API pppp_result pppp_calculate_strains(const pppp_beatmap* map, const pppp_difficulty_options* options,
                                              pppp_strains** out);

/// The ruleset of a strain graph, the time its first section starts at and the length of each
/// section, both in milliseconds.
PPPP_C_API pppp_result pppp_strains_info(const pppp_strains* strains, pppp_int32* ruleset, double* start_time,
                                         double* section_length);

/// The series of a strain graph. The pointers belong to the result and stay valid until
/// `pppp_strains_free`. Each fails with `PPPP_INVALID_ARGUMENT` when the graph is another
/// ruleset's.
PPPP_C_API pppp_result pppp_strains_osu(const pppp_strains* strains, pppp_osu_strains* out);
PPPP_C_API pppp_result pppp_strains_taiko(const pppp_strains* strains, pppp_taiko_strains* out);
PPPP_C_API pppp_result pppp_strains_catch(const pppp_strains* strains, pppp_catch_strains* out);
PPPP_C_API pppp_result pppp_strains_mania(const pppp_strains* strains, pppp_mania_strains* out);

/// Release a strain graph. Passing null is allowed and does nothing.
PPPP_C_API void pppp_strains_free(pppp_strains* strains);

#ifdef __cplusplus
} // extern "C"
#endif

// NOLINTEND(readability-identifier-naming)

#endif
