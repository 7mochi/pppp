#ifndef PPPP_CAPI_H
#define PPPP_CAPI_H

#include "pppp/version.h" /* IWYU pragma: export */
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

/* NOLINTBEGIN(readability-identifier-naming): Public names use the Pppp_ prefix and underscores
   because C has no namespaces. */

#if defined(_MSC_VER) && _MSC_VER < 1310
typedef __int64 PpppInt64;
typedef unsigned __int64 PpppUint64;
#elif defined(__GNUC__)
__extension__ typedef long long PpppInt64;
__extension__ typedef unsigned long long PpppUint64;
#else
typedef long long PpppInt64;
typedef unsigned long long PpppUint64;
#endif

typedef int PpppInt32;
typedef unsigned int PpppUint32;
typedef PpppInt32 PpppBool;

/* A call that can fail returns PpppErrorCode. The error message belongs to the calling thread.
   A successful call clears it, and a failed call sets it. GetLastMessage returns NULL after
   success, and its text stays valid until the next call on that thread. BufferSizeQuery is not
   an error. It reports the required size, and the call should be repeated with a buffer. */
typedef enum PpppErrorCode {
    Pppp_BufferSizeQuery = -1,
    Pppp_Success = 0,
    Pppp_InvalidArgument = 1,
    Pppp_RulesetUnavailable = 2,
    Pppp_UnexpectedRuleset = 3,
    Pppp_Parse = 4,
    Pppp_Allocation = 5,
    Pppp_NoSliderPathBackend = 6,
    Pppp_SliderPath = 7,
    Pppp_Internal = 8,
    Pppp_Failure = 127
} PpppErrorCode;

/* Handles hide their contents. Destroy accepts NULL and does nothing. */

typedef struct PpppBeatmap PpppBeatmap;
typedef struct PpppRuleset PpppRuleset;
typedef struct PpppMods PpppMods;
typedef struct PpppOsuDifficultyCalculator PpppOsuDifficultyCalculator;
typedef struct PpppTaikoDifficultyCalculator PpppTaikoDifficultyCalculator;
typedef struct PpppCatchDifficultyCalculator PpppCatchDifficultyCalculator;
typedef struct PpppManiaDifficultyCalculator PpppManiaDifficultyCalculator;
typedef struct PpppTimedResult PpppTimedResult;
typedef struct PpppStrains PpppStrains;
typedef struct PpppOsuPerformanceCalculator PpppOsuPerformanceCalculator;
typedef struct PpppTaikoPerformanceCalculator PpppTaikoPerformanceCalculator;
typedef struct PpppCatchPerformanceCalculator PpppCatchPerformanceCalculator;
typedef struct PpppManiaPerformanceCalculator PpppManiaPerformanceCalculator;

typedef struct PpppNativeBeatmap {
    PpppBeatmap* handle;
    PpppInt32 ruleset_id;
    PpppInt64 beatmap_id;
    float approach_rate;
    float drain_rate;
    float overall_difficulty;
    float circle_size;
    double slider_multiplier;
    double slider_tick_rate;
    PpppInt32 format_version;
    float stack_leniency;
} PpppNativeBeatmap;

typedef struct PpppNativeRuleset {
    PpppRuleset* handle;
    PpppInt32 ruleset_id;
} PpppNativeRuleset;

typedef struct PpppNativeMods {
    PpppMods* handle;
} PpppNativeMods;

/* A value is present when has_value is nonzero. */

typedef struct PpppOptionalInt64 {
    PpppBool has_value;
    PpppInt64 value;
} PpppOptionalInt64;

typedef struct PpppOptionalDouble {
    PpppBool has_value;
    double value;
} PpppOptionalDouble;

/* Hit counts are separate fields. maximum_statistics is not exposed. */
typedef struct PpppNativeScoreInfo {
    PpppRuleset* ruleset;
    PpppBeatmap* beatmap;
    PpppMods* mods; /* NULL means no mods */
    PpppInt32 max_combo;
    double accuracy;
    PpppOptionalInt64 legacy_total_score;
    PpppInt32 count_miss;
    PpppInt32 count_meh;
    PpppInt32 count_ok;
    PpppInt32 count_good;
    PpppInt32 count_great;
    PpppInt32 count_perfect;
    PpppInt32 count_small_tick_miss;
    PpppInt32 count_small_tick_hit;
    PpppInt32 count_large_tick_miss;
    PpppInt32 count_large_tick_hit;
    PpppInt32 count_slider_tail_hit;
} PpppNativeScoreInfo;

typedef struct PpppOsuDifficultyAttributes {
    double star_rating;
    PpppInt32 max_combo;
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
    PpppInt32 hit_circle_count;
    PpppInt32 slider_count;
    PpppInt32 large_tick_count;
    PpppInt32 spinner_count;
    double nested_score_per_object;
    double legacy_score_base_multiplier;
    double maximum_legacy_combo_score;
} PpppOsuDifficultyAttributes;

typedef struct PpppTaikoDifficultyAttributes {
    double star_rating;
    PpppInt32 max_combo;
    double mechanical_difficulty;
    double rhythm_difficulty;
    double reading_difficulty;
    double colour_difficulty;
    double stamina_difficulty;
    double mono_stamina_factor;
    double consistency_factor;
    double stamina_top_strains;
} PpppTaikoDifficultyAttributes;

typedef struct PpppCatchDifficultyAttributes {
    double star_rating;
    PpppInt32 max_combo;
} PpppCatchDifficultyAttributes;

typedef struct PpppManiaDifficultyAttributes {
    double star_rating;
    PpppInt32 max_combo;
} PpppManiaDifficultyAttributes;

typedef struct PpppOsuPerformanceAttributes {
    double total;
    double aim;
    double speed;
    double accuracy;
    double flashlight;
    double reading;
    double effective_miss_count;
    double combo_based_estimated_miss_count;
    PpppOptionalDouble score_based_estimated_miss_count;
    double aim_estimated_slider_breaks;
    double speed_estimated_slider_breaks;
    PpppOptionalDouble speed_deviation;
} PpppOsuPerformanceAttributes;

typedef struct PpppTaikoPerformanceAttributes {
    double total;
    double difficulty;
    double accuracy;
    PpppOptionalDouble estimated_unstable_rate;
} PpppTaikoPerformanceAttributes;

typedef struct PpppCatchPerformanceAttributes {
    double total;
} PpppCatchPerformanceAttributes;

typedef struct PpppManiaPerformanceAttributes {
    double total;
    double difficulty;
} PpppManiaPerformanceAttributes;

typedef struct PpppTimedOsuDifficultyAttributes {
    double time;
    PpppOsuDifficultyAttributes attributes;
} PpppTimedOsuDifficultyAttributes;

typedef struct PpppTimedTaikoDifficultyAttributes {
    double time;
    PpppTaikoDifficultyAttributes attributes;
} PpppTimedTaikoDifficultyAttributes;

typedef struct PpppTimedCatchDifficultyAttributes {
    double time;
    PpppCatchDifficultyAttributes attributes;
} PpppTimedCatchDifficultyAttributes;

typedef struct PpppTimedManiaDifficultyAttributes {
    double time;
    PpppManiaDifficultyAttributes attributes;
} PpppTimedManiaDifficultyAttributes;

typedef struct PpppNativeOsuDifficultyCalculator {
    PpppOsuDifficultyCalculator* handle;
} PpppNativeOsuDifficultyCalculator;

typedef struct PpppNativeTaikoDifficultyCalculator {
    PpppTaikoDifficultyCalculator* handle;
} PpppNativeTaikoDifficultyCalculator;

typedef struct PpppNativeCatchDifficultyCalculator {
    PpppCatchDifficultyCalculator* handle;
} PpppNativeCatchDifficultyCalculator;

typedef struct PpppNativeManiaDifficultyCalculator {
    PpppManiaDifficultyCalculator* handle;
} PpppNativeManiaDifficultyCalculator;

typedef struct PpppNativeOsuPerformanceCalculator {
    PpppOsuPerformanceCalculator* handle;
} PpppNativeOsuPerformanceCalculator;

typedef struct PpppNativeTaikoPerformanceCalculator {
    PpppTaikoPerformanceCalculator* handle;
} PpppNativeTaikoPerformanceCalculator;

typedef struct PpppNativeCatchPerformanceCalculator {
    PpppCatchPerformanceCalculator* handle;
} PpppNativeCatchPerformanceCalculator;

typedef struct PpppNativeManiaPerformanceCalculator {
    PpppManiaPerformanceCalculator* handle;
} PpppNativeManiaPerformanceCalculator;

#ifdef __cplusplus
extern "C" {
#endif

PPPP_C_API const char* Pppp_Version(void);
PPPP_C_API PpppUint32 Pppp_VersionNumber(void);
PPPP_C_API const char* Pppp_Backend(void);

PPPP_C_API const char* Pppp_ErrorHandler_GetLastMessage(void);

PPPP_C_API PpppErrorCode Pppp_Beatmap_CreateFromFile(const char* path, PpppNativeBeatmap* out);
PPPP_C_API PpppErrorCode Pppp_Beatmap_CreateFromText(const char* text, PpppNativeBeatmap* out);
PPPP_C_API PpppErrorCode Pppp_Beatmap_CreateFromBytes(const void* data, size_t size, PpppNativeBeatmap* out);

/* Buffer getters follow the same rules. If buffer is NULL, each getter writes the required size
   to *size and returns Pppp_BufferSizeQuery. Otherwise it copies at most *size elements. String
   getters truncate and end with a NUL byte. A negative *size returns Pppp_InvalidArgument. */

PPPP_C_API PpppErrorCode Pppp_Beatmap_GetTitle(PpppBeatmap* beatmap, char* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Beatmap_GetArtist(PpppBeatmap* beatmap, char* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Beatmap_GetVersion(PpppBeatmap* beatmap, char* buffer, PpppInt32* size);

/* Releases your reference to the beatmap. After this, the handle is no longer valid, but the map
   can stay loaded if a calculator still has it. */
PPPP_C_API void Pppp_Beatmap_Destroy(PpppBeatmap* beatmap);

PPPP_C_API PpppErrorCode Pppp_Ruleset_CreateFromId(PpppInt32 id, PpppNativeRuleset* out);

/* Short names are "osu", "taiko", "fruits" (osu!catch) and "mania". An unknown name returns
   Pppp_RulesetUnavailable. */
PPPP_C_API PpppErrorCode Pppp_Ruleset_CreateFromShortName(const char* name, PpppNativeRuleset* out);
PPPP_C_API PpppErrorCode Pppp_Ruleset_GetShortName(PpppRuleset* ruleset, char* buffer, PpppInt32* size);
PPPP_C_API void Pppp_Ruleset_Destroy(PpppRuleset* ruleset);

PPPP_C_API PpppErrorCode Pppp_Mods_Create(PpppNativeMods* out);
PPPP_C_API PpppErrorCode Pppp_Mods_Parse(PpppMods* mods, const char* specification);

/* The ruleset id is 0..3. An unknown id returns Pppp_RulesetUnavailable. */
PPPP_C_API PpppErrorCode Pppp_Mods_FromLegacy(PpppMods* mods, PpppUint32 bits, PpppInt32 ruleset);
PPPP_C_API PpppErrorCode Pppp_Mods_AddClassic(PpppMods* mods);
PPPP_C_API PpppErrorCode Pppp_Mods_Contains(PpppMods* mods, const char* acronym, PpppBool* out);
PPPP_C_API PpppErrorCode Pppp_Mods_GetCount(PpppMods* mods, PpppInt32* out);
PPPP_C_API PpppErrorCode Pppp_Mods_GetAcronym(PpppMods* mods, PpppInt32 index, char* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Mods_GetClockRate(PpppMods* mods, double* out);

/* The ruleset id is 0..3. An unknown id returns Pppp_RulesetUnavailable. */
PPPP_C_API PpppErrorCode Pppp_Mods_GetLegacyBits(PpppMods* mods, PpppInt32 ruleset, PpppUint32* out);
PPPP_C_API void Pppp_Mods_Destroy(PpppMods* mods);

/* The ruleset handle must match the calculator. Otherwise Create returns
   Pppp_UnexpectedRuleset. The calculator keeps the beatmap alive. */

PPPP_C_API PpppErrorCode Pppp_OsuDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                             PpppNativeOsuDifficultyCalculator* out);
PPPP_C_API PpppErrorCode Pppp_OsuDifficultyCalculator_Calculate(PpppOsuDifficultyCalculator* calculator,
                                                                PpppMods* mods,
                                                                PpppOsuDifficultyAttributes* out);
PPPP_C_API PpppErrorCode Pppp_OsuDifficultyCalculator_CalculateTimed(PpppOsuDifficultyCalculator* calculator,
                                                                     PpppMods* mods, PpppTimedResult** out);
PPPP_C_API PpppErrorCode Pppp_OsuDifficultyCalculator_CalculateStrains(
    PpppOsuDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** out);
PPPP_C_API PpppErrorCode Pppp_OsuDifficultyCalculator_SetClockRate(PpppOsuDifficultyCalculator* calculator,
                                                                   double clock_rate);
PPPP_C_API PpppErrorCode Pppp_OsuDifficultyCalculator_ClearClockRate(PpppOsuDifficultyCalculator* calculator);
PPPP_C_API void Pppp_OsuDifficultyCalculator_Destroy(PpppOsuDifficultyCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_TaikoDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                               PpppNativeTaikoDifficultyCalculator* out);
PPPP_C_API PpppErrorCode Pppp_TaikoDifficultyCalculator_Calculate(PpppTaikoDifficultyCalculator* calculator,
                                                                  PpppMods* mods,
                                                                  PpppTaikoDifficultyAttributes* out);
PPPP_C_API PpppErrorCode Pppp_TaikoDifficultyCalculator_CalculateTimed(
    PpppTaikoDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** out);
PPPP_C_API PpppErrorCode Pppp_TaikoDifficultyCalculator_CalculateStrains(
    PpppTaikoDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** out);
PPPP_C_API PpppErrorCode
Pppp_TaikoDifficultyCalculator_SetClockRate(PpppTaikoDifficultyCalculator* calculator, double clock_rate);
PPPP_C_API PpppErrorCode
Pppp_TaikoDifficultyCalculator_ClearClockRate(PpppTaikoDifficultyCalculator* calculator);
PPPP_C_API void Pppp_TaikoDifficultyCalculator_Destroy(PpppTaikoDifficultyCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_CatchDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                               PpppNativeCatchDifficultyCalculator* out);
PPPP_C_API PpppErrorCode Pppp_CatchDifficultyCalculator_Calculate(PpppCatchDifficultyCalculator* calculator,
                                                                  PpppMods* mods,
                                                                  PpppCatchDifficultyAttributes* out);
PPPP_C_API PpppErrorCode Pppp_CatchDifficultyCalculator_CalculateTimed(
    PpppCatchDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** out);
PPPP_C_API PpppErrorCode Pppp_CatchDifficultyCalculator_CalculateStrains(
    PpppCatchDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** out);
PPPP_C_API PpppErrorCode
Pppp_CatchDifficultyCalculator_SetClockRate(PpppCatchDifficultyCalculator* calculator, double clock_rate);
PPPP_C_API PpppErrorCode
Pppp_CatchDifficultyCalculator_ClearClockRate(PpppCatchDifficultyCalculator* calculator);
PPPP_C_API void Pppp_CatchDifficultyCalculator_Destroy(PpppCatchDifficultyCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_ManiaDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                               PpppNativeManiaDifficultyCalculator* out);
PPPP_C_API PpppErrorCode Pppp_ManiaDifficultyCalculator_Calculate(PpppManiaDifficultyCalculator* calculator,
                                                                  PpppMods* mods,
                                                                  PpppManiaDifficultyAttributes* out);
PPPP_C_API PpppErrorCode Pppp_ManiaDifficultyCalculator_CalculateTimed(
    PpppManiaDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** out);
PPPP_C_API PpppErrorCode Pppp_ManiaDifficultyCalculator_CalculateStrains(
    PpppManiaDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** out);
PPPP_C_API PpppErrorCode
Pppp_ManiaDifficultyCalculator_SetClockRate(PpppManiaDifficultyCalculator* calculator, double clock_rate);
PPPP_C_API PpppErrorCode
Pppp_ManiaDifficultyCalculator_ClearClockRate(PpppManiaDifficultyCalculator* calculator);
PPPP_C_API void Pppp_ManiaDifficultyCalculator_Destroy(PpppManiaDifficultyCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_OsuPerformanceCalculator_Create(PpppNativeOsuPerformanceCalculator* out);
PPPP_C_API PpppErrorCode Pppp_OsuPerformanceCalculator_Calculate(
    PpppOsuPerformanceCalculator* calculator, const PpppNativeScoreInfo* score,
    const PpppOsuDifficultyAttributes* difficulty, PpppOsuPerformanceAttributes* out);
PPPP_C_API void Pppp_OsuPerformanceCalculator_Destroy(PpppOsuPerformanceCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_TaikoPerformanceCalculator_Create(PpppNativeTaikoPerformanceCalculator* out);
PPPP_C_API PpppErrorCode Pppp_TaikoPerformanceCalculator_Calculate(
    PpppTaikoPerformanceCalculator* calculator, const PpppNativeScoreInfo* score,
    const PpppTaikoDifficultyAttributes* difficulty, PpppTaikoPerformanceAttributes* out);
PPPP_C_API void Pppp_TaikoPerformanceCalculator_Destroy(PpppTaikoPerformanceCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_CatchPerformanceCalculator_Create(PpppNativeCatchPerformanceCalculator* out);
PPPP_C_API PpppErrorCode Pppp_CatchPerformanceCalculator_Calculate(
    PpppCatchPerformanceCalculator* calculator, const PpppNativeScoreInfo* score,
    const PpppCatchDifficultyAttributes* difficulty, PpppCatchPerformanceAttributes* out);
PPPP_C_API void Pppp_CatchPerformanceCalculator_Destroy(PpppCatchPerformanceCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_ManiaPerformanceCalculator_Create(PpppNativeManiaPerformanceCalculator* out);
PPPP_C_API PpppErrorCode Pppp_ManiaPerformanceCalculator_Calculate(
    PpppManiaPerformanceCalculator* calculator, const PpppNativeScoreInfo* score,
    const PpppManiaDifficultyAttributes* difficulty, PpppManiaPerformanceAttributes* out);
PPPP_C_API void Pppp_ManiaPerformanceCalculator_Destroy(PpppManiaPerformanceCalculator* calculator);

PPPP_C_API PpppErrorCode Pppp_TimedResult_GetCount(PpppTimedResult* result, PpppInt32* out);
PPPP_C_API PpppErrorCode Pppp_TimedResult_GetOsu(PpppTimedResult* result,
                                                 PpppTimedOsuDifficultyAttributes* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_TimedResult_GetTaiko(PpppTimedResult* result,
                                                   PpppTimedTaikoDifficultyAttributes* buffer,
                                                   PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_TimedResult_GetCatch(PpppTimedResult* result,
                                                   PpppTimedCatchDifficultyAttributes* buffer,
                                                   PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_TimedResult_GetMania(PpppTimedResult* result,
                                                   PpppTimedManiaDifficultyAttributes* buffer,
                                                   PpppInt32* size);
PPPP_C_API void Pppp_TimedResult_Destroy(PpppTimedResult* result);

PPPP_C_API PpppErrorCode Pppp_Strains_GetRuleset(PpppStrains* strains, PpppInt32* out);
PPPP_C_API PpppErrorCode Pppp_Strains_GetStartTime(PpppStrains* strains, double* out);
PPPP_C_API PpppErrorCode Pppp_Strains_GetSectionLength(PpppStrains* strains, double* out);

PPPP_C_API PpppErrorCode Pppp_Strains_GetOsuAim(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetOsuAimNoSliders(PpppStrains* strains, double* buffer,
                                                         PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetOsuSpeed(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetOsuReading(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetOsuFlashlight(PpppStrains* strains, double* buffer, PpppInt32* size);

PPPP_C_API PpppErrorCode Pppp_Strains_GetTaikoColour(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetTaikoReading(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetTaikoRhythm(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetTaikoStamina(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetTaikoSingleColourStamina(PpppStrains* strains, double* buffer,
                                                                  PpppInt32* size);

PPPP_C_API PpppErrorCode Pppp_Strains_GetCatchMovement(PpppStrains* strains, double* buffer, PpppInt32* size);
PPPP_C_API PpppErrorCode Pppp_Strains_GetManiaStrain(PpppStrains* strains, double* buffer, PpppInt32* size);

PPPP_C_API void Pppp_Strains_Destroy(PpppStrains* strains);

#ifdef __cplusplus
}
#endif

/* NOLINTEND(readability-identifier-naming) */

#endif
