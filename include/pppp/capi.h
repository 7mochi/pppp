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

#ifdef __cplusplus
}
#endif

/* NOLINTEND(readability-identifier-naming) */

#endif
