
# Begin preamble for Python

from .ctypes_preamble import *
from .ctypes_preamble import _variadic_function

# End preamble

_libs = {}
_libdirs = []

# Begin loader

from .ctypes_loader import *

# End loader

add_library_search_dirs([])

# Begin libraries
_libs["pppp_c"] = load_library("pppp_c")

# 1 libraries
# End libraries

# No modules

PpppInt64 = c_longlong# include/pppp/capi.h: 31

PpppUint64 = c_ulonglong# include/pppp/capi.h: 32

PpppInt32 = c_int# include/pppp/capi.h: 35

PpppUint32 = c_uint# include/pppp/capi.h: 36

PpppBool = PpppInt32# include/pppp/capi.h: 37

enum_PpppErrorCode = c_int# include/pppp/capi.h: 55

Pppp_BufferSizeQuery = (-1)# include/pppp/capi.h: 55

Pppp_Success = 0# include/pppp/capi.h: 55

Pppp_InvalidArgument = 1# include/pppp/capi.h: 55

Pppp_RulesetUnavailable = 2# include/pppp/capi.h: 55

Pppp_UnexpectedRuleset = 3# include/pppp/capi.h: 55

Pppp_Parse = 4# include/pppp/capi.h: 55

Pppp_Allocation = 5# include/pppp/capi.h: 55

Pppp_NoSliderPathBackend = 6# include/pppp/capi.h: 55

Pppp_SliderPath = 7# include/pppp/capi.h: 55

Pppp_Internal = 8# include/pppp/capi.h: 55

Pppp_Failure = 127# include/pppp/capi.h: 55

PpppErrorCode = enum_PpppErrorCode# include/pppp/capi.h: 55

# include/pppp/capi.h: 59
class struct_PpppBeatmap(Structure):
    pass

PpppBeatmap = struct_PpppBeatmap# include/pppp/capi.h: 59

# include/pppp/capi.h: 60
class struct_PpppRuleset(Structure):
    pass

PpppRuleset = struct_PpppRuleset# include/pppp/capi.h: 60

# include/pppp/capi.h: 61
class struct_PpppMods(Structure):
    pass

PpppMods = struct_PpppMods# include/pppp/capi.h: 61

# include/pppp/capi.h: 62
class struct_PpppOsuDifficultyCalculator(Structure):
    pass

PpppOsuDifficultyCalculator = struct_PpppOsuDifficultyCalculator# include/pppp/capi.h: 62

# include/pppp/capi.h: 63
class struct_PpppTaikoDifficultyCalculator(Structure):
    pass

PpppTaikoDifficultyCalculator = struct_PpppTaikoDifficultyCalculator# include/pppp/capi.h: 63

# include/pppp/capi.h: 64
class struct_PpppCatchDifficultyCalculator(Structure):
    pass

PpppCatchDifficultyCalculator = struct_PpppCatchDifficultyCalculator# include/pppp/capi.h: 64

# include/pppp/capi.h: 65
class struct_PpppManiaDifficultyCalculator(Structure):
    pass

PpppManiaDifficultyCalculator = struct_PpppManiaDifficultyCalculator# include/pppp/capi.h: 65

# include/pppp/capi.h: 66
class struct_PpppTimedResult(Structure):
    pass

PpppTimedResult = struct_PpppTimedResult# include/pppp/capi.h: 66

# include/pppp/capi.h: 67
class struct_PpppStrains(Structure):
    pass

PpppStrains = struct_PpppStrains# include/pppp/capi.h: 67

# include/pppp/capi.h: 68
class struct_PpppOsuPerformanceCalculator(Structure):
    pass

PpppOsuPerformanceCalculator = struct_PpppOsuPerformanceCalculator# include/pppp/capi.h: 68

# include/pppp/capi.h: 69
class struct_PpppTaikoPerformanceCalculator(Structure):
    pass

PpppTaikoPerformanceCalculator = struct_PpppTaikoPerformanceCalculator# include/pppp/capi.h: 69

# include/pppp/capi.h: 70
class struct_PpppCatchPerformanceCalculator(Structure):
    pass

PpppCatchPerformanceCalculator = struct_PpppCatchPerformanceCalculator# include/pppp/capi.h: 70

# include/pppp/capi.h: 71
class struct_PpppManiaPerformanceCalculator(Structure):
    pass

PpppManiaPerformanceCalculator = struct_PpppManiaPerformanceCalculator# include/pppp/capi.h: 71

# include/pppp/capi.h: 85
class struct_PpppNativeBeatmap(Structure):
    pass

struct_PpppNativeBeatmap.__slots__ = [
    'handle',
    'ruleset_id',
    'beatmap_id',
    'approach_rate',
    'drain_rate',
    'overall_difficulty',
    'circle_size',
    'slider_multiplier',
    'slider_tick_rate',
    'format_version',
    'stack_leniency',
]
struct_PpppNativeBeatmap._fields_ = [
    ('handle', POINTER(PpppBeatmap)),
    ('ruleset_id', PpppInt32),
    ('beatmap_id', PpppInt64),
    ('approach_rate', c_float),
    ('drain_rate', c_float),
    ('overall_difficulty', c_float),
    ('circle_size', c_float),
    ('slider_multiplier', c_double),
    ('slider_tick_rate', c_double),
    ('format_version', PpppInt32),
    ('stack_leniency', c_float),
]

PpppNativeBeatmap = struct_PpppNativeBeatmap# include/pppp/capi.h: 85

# include/pppp/capi.h: 90
class struct_PpppNativeRuleset(Structure):
    pass

struct_PpppNativeRuleset.__slots__ = [
    'handle',
    'ruleset_id',
]
struct_PpppNativeRuleset._fields_ = [
    ('handle', POINTER(PpppRuleset)),
    ('ruleset_id', PpppInt32),
]

PpppNativeRuleset = struct_PpppNativeRuleset# include/pppp/capi.h: 90

# include/pppp/capi.h: 94
class struct_PpppNativeMods(Structure):
    pass

struct_PpppNativeMods.__slots__ = [
    'handle',
]
struct_PpppNativeMods._fields_ = [
    ('handle', POINTER(PpppMods)),
]

PpppNativeMods = struct_PpppNativeMods# include/pppp/capi.h: 94

# include/pppp/capi.h: 101
class struct_PpppOptionalInt64(Structure):
    pass

struct_PpppOptionalInt64.__slots__ = [
    'has_value',
    'value',
]
struct_PpppOptionalInt64._fields_ = [
    ('has_value', PpppBool),
    ('value', PpppInt64),
]

PpppOptionalInt64 = struct_PpppOptionalInt64# include/pppp/capi.h: 101

# include/pppp/capi.h: 106
class struct_PpppOptionalDouble(Structure):
    pass

struct_PpppOptionalDouble.__slots__ = [
    'has_value',
    'value',
]
struct_PpppOptionalDouble._fields_ = [
    ('has_value', PpppBool),
    ('value', c_double),
]

PpppOptionalDouble = struct_PpppOptionalDouble# include/pppp/capi.h: 106

# include/pppp/capi.h: 127
class struct_PpppNativeScoreInfo(Structure):
    pass

struct_PpppNativeScoreInfo.__slots__ = [
    'ruleset',
    'beatmap',
    'mods',
    'max_combo',
    'accuracy',
    'legacy_total_score',
    'count_miss',
    'count_meh',
    'count_ok',
    'count_good',
    'count_great',
    'count_perfect',
    'count_small_tick_miss',
    'count_small_tick_hit',
    'count_large_tick_miss',
    'count_large_tick_hit',
    'count_slider_tail_hit',
]
struct_PpppNativeScoreInfo._fields_ = [
    ('ruleset', POINTER(PpppRuleset)),
    ('beatmap', POINTER(PpppBeatmap)),
    ('mods', POINTER(PpppMods)),
    ('max_combo', PpppInt32),
    ('accuracy', c_double),
    ('legacy_total_score', PpppOptionalInt64),
    ('count_miss', PpppInt32),
    ('count_meh', PpppInt32),
    ('count_ok', PpppInt32),
    ('count_good', PpppInt32),
    ('count_great', PpppInt32),
    ('count_perfect', PpppInt32),
    ('count_small_tick_miss', PpppInt32),
    ('count_small_tick_hit', PpppInt32),
    ('count_large_tick_miss', PpppInt32),
    ('count_large_tick_hit', PpppInt32),
    ('count_slider_tail_hit', PpppInt32),
]

PpppNativeScoreInfo = struct_PpppNativeScoreInfo# include/pppp/capi.h: 127

# include/pppp/capi.h: 151
class struct_PpppOsuDifficultyAttributes(Structure):
    pass

struct_PpppOsuDifficultyAttributes.__slots__ = [
    'star_rating',
    'max_combo',
    'aim_difficulty',
    'speed_difficulty',
    'reading_difficulty',
    'flashlight_difficulty',
    'slider_factor',
    'aim_difficult_strain_count',
    'speed_difficult_strain_count',
    'reading_difficult_note_count',
    'aim_difficult_slider_count',
    'aim_top_weighted_slider_factor',
    'speed_top_weighted_slider_factor',
    'speed_note_count',
    'hit_circle_count',
    'slider_count',
    'large_tick_count',
    'spinner_count',
    'nested_score_per_object',
    'legacy_score_base_multiplier',
    'maximum_legacy_combo_score',
]
struct_PpppOsuDifficultyAttributes._fields_ = [
    ('star_rating', c_double),
    ('max_combo', PpppInt32),
    ('aim_difficulty', c_double),
    ('speed_difficulty', c_double),
    ('reading_difficulty', c_double),
    ('flashlight_difficulty', c_double),
    ('slider_factor', c_double),
    ('aim_difficult_strain_count', c_double),
    ('speed_difficult_strain_count', c_double),
    ('reading_difficult_note_count', c_double),
    ('aim_difficult_slider_count', c_double),
    ('aim_top_weighted_slider_factor', c_double),
    ('speed_top_weighted_slider_factor', c_double),
    ('speed_note_count', c_double),
    ('hit_circle_count', PpppInt32),
    ('slider_count', PpppInt32),
    ('large_tick_count', PpppInt32),
    ('spinner_count', PpppInt32),
    ('nested_score_per_object', c_double),
    ('legacy_score_base_multiplier', c_double),
    ('maximum_legacy_combo_score', c_double),
]

PpppOsuDifficultyAttributes = struct_PpppOsuDifficultyAttributes# include/pppp/capi.h: 151

# include/pppp/capi.h: 164
class struct_PpppTaikoDifficultyAttributes(Structure):
    pass

struct_PpppTaikoDifficultyAttributes.__slots__ = [
    'star_rating',
    'max_combo',
    'mechanical_difficulty',
    'rhythm_difficulty',
    'reading_difficulty',
    'colour_difficulty',
    'stamina_difficulty',
    'mono_stamina_factor',
    'consistency_factor',
    'stamina_top_strains',
]
struct_PpppTaikoDifficultyAttributes._fields_ = [
    ('star_rating', c_double),
    ('max_combo', PpppInt32),
    ('mechanical_difficulty', c_double),
    ('rhythm_difficulty', c_double),
    ('reading_difficulty', c_double),
    ('colour_difficulty', c_double),
    ('stamina_difficulty', c_double),
    ('mono_stamina_factor', c_double),
    ('consistency_factor', c_double),
    ('stamina_top_strains', c_double),
]

PpppTaikoDifficultyAttributes = struct_PpppTaikoDifficultyAttributes# include/pppp/capi.h: 164

# include/pppp/capi.h: 169
class struct_PpppCatchDifficultyAttributes(Structure):
    pass

struct_PpppCatchDifficultyAttributes.__slots__ = [
    'star_rating',
    'max_combo',
]
struct_PpppCatchDifficultyAttributes._fields_ = [
    ('star_rating', c_double),
    ('max_combo', PpppInt32),
]

PpppCatchDifficultyAttributes = struct_PpppCatchDifficultyAttributes# include/pppp/capi.h: 169

# include/pppp/capi.h: 174
class struct_PpppManiaDifficultyAttributes(Structure):
    pass

struct_PpppManiaDifficultyAttributes.__slots__ = [
    'star_rating',
    'max_combo',
]
struct_PpppManiaDifficultyAttributes._fields_ = [
    ('star_rating', c_double),
    ('max_combo', PpppInt32),
]

PpppManiaDifficultyAttributes = struct_PpppManiaDifficultyAttributes# include/pppp/capi.h: 174

# include/pppp/capi.h: 189
class struct_PpppOsuPerformanceAttributes(Structure):
    pass

struct_PpppOsuPerformanceAttributes.__slots__ = [
    'total',
    'aim',
    'speed',
    'accuracy',
    'flashlight',
    'reading',
    'effective_miss_count',
    'combo_based_estimated_miss_count',
    'score_based_estimated_miss_count',
    'aim_estimated_slider_breaks',
    'speed_estimated_slider_breaks',
    'speed_deviation',
]
struct_PpppOsuPerformanceAttributes._fields_ = [
    ('total', c_double),
    ('aim', c_double),
    ('speed', c_double),
    ('accuracy', c_double),
    ('flashlight', c_double),
    ('reading', c_double),
    ('effective_miss_count', c_double),
    ('combo_based_estimated_miss_count', c_double),
    ('score_based_estimated_miss_count', PpppOptionalDouble),
    ('aim_estimated_slider_breaks', c_double),
    ('speed_estimated_slider_breaks', c_double),
    ('speed_deviation', PpppOptionalDouble),
]

PpppOsuPerformanceAttributes = struct_PpppOsuPerformanceAttributes# include/pppp/capi.h: 189

# include/pppp/capi.h: 196
class struct_PpppTaikoPerformanceAttributes(Structure):
    pass

struct_PpppTaikoPerformanceAttributes.__slots__ = [
    'total',
    'difficulty',
    'accuracy',
    'estimated_unstable_rate',
]
struct_PpppTaikoPerformanceAttributes._fields_ = [
    ('total', c_double),
    ('difficulty', c_double),
    ('accuracy', c_double),
    ('estimated_unstable_rate', PpppOptionalDouble),
]

PpppTaikoPerformanceAttributes = struct_PpppTaikoPerformanceAttributes# include/pppp/capi.h: 196

# include/pppp/capi.h: 200
class struct_PpppCatchPerformanceAttributes(Structure):
    pass

struct_PpppCatchPerformanceAttributes.__slots__ = [
    'total',
]
struct_PpppCatchPerformanceAttributes._fields_ = [
    ('total', c_double),
]

PpppCatchPerformanceAttributes = struct_PpppCatchPerformanceAttributes# include/pppp/capi.h: 200

# include/pppp/capi.h: 205
class struct_PpppManiaPerformanceAttributes(Structure):
    pass

struct_PpppManiaPerformanceAttributes.__slots__ = [
    'total',
    'difficulty',
]
struct_PpppManiaPerformanceAttributes._fields_ = [
    ('total', c_double),
    ('difficulty', c_double),
]

PpppManiaPerformanceAttributes = struct_PpppManiaPerformanceAttributes# include/pppp/capi.h: 205

# include/pppp/capi.h: 210
class struct_PpppTimedOsuDifficultyAttributes(Structure):
    pass

struct_PpppTimedOsuDifficultyAttributes.__slots__ = [
    'time',
    'attributes',
]
struct_PpppTimedOsuDifficultyAttributes._fields_ = [
    ('time', c_double),
    ('attributes', PpppOsuDifficultyAttributes),
]

PpppTimedOsuDifficultyAttributes = struct_PpppTimedOsuDifficultyAttributes# include/pppp/capi.h: 210

# include/pppp/capi.h: 215
class struct_PpppTimedTaikoDifficultyAttributes(Structure):
    pass

struct_PpppTimedTaikoDifficultyAttributes.__slots__ = [
    'time',
    'attributes',
]
struct_PpppTimedTaikoDifficultyAttributes._fields_ = [
    ('time', c_double),
    ('attributes', PpppTaikoDifficultyAttributes),
]

PpppTimedTaikoDifficultyAttributes = struct_PpppTimedTaikoDifficultyAttributes# include/pppp/capi.h: 215

# include/pppp/capi.h: 220
class struct_PpppTimedCatchDifficultyAttributes(Structure):
    pass

struct_PpppTimedCatchDifficultyAttributes.__slots__ = [
    'time',
    'attributes',
]
struct_PpppTimedCatchDifficultyAttributes._fields_ = [
    ('time', c_double),
    ('attributes', PpppCatchDifficultyAttributes),
]

PpppTimedCatchDifficultyAttributes = struct_PpppTimedCatchDifficultyAttributes# include/pppp/capi.h: 220

# include/pppp/capi.h: 225
class struct_PpppTimedManiaDifficultyAttributes(Structure):
    pass

struct_PpppTimedManiaDifficultyAttributes.__slots__ = [
    'time',
    'attributes',
]
struct_PpppTimedManiaDifficultyAttributes._fields_ = [
    ('time', c_double),
    ('attributes', PpppManiaDifficultyAttributes),
]

PpppTimedManiaDifficultyAttributes = struct_PpppTimedManiaDifficultyAttributes# include/pppp/capi.h: 225

# include/pppp/capi.h: 229
class struct_PpppNativeOsuDifficultyCalculator(Structure):
    pass

struct_PpppNativeOsuDifficultyCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeOsuDifficultyCalculator._fields_ = [
    ('handle', POINTER(PpppOsuDifficultyCalculator)),
]

PpppNativeOsuDifficultyCalculator = struct_PpppNativeOsuDifficultyCalculator# include/pppp/capi.h: 229

# include/pppp/capi.h: 233
class struct_PpppNativeTaikoDifficultyCalculator(Structure):
    pass

struct_PpppNativeTaikoDifficultyCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeTaikoDifficultyCalculator._fields_ = [
    ('handle', POINTER(PpppTaikoDifficultyCalculator)),
]

PpppNativeTaikoDifficultyCalculator = struct_PpppNativeTaikoDifficultyCalculator# include/pppp/capi.h: 233

# include/pppp/capi.h: 237
class struct_PpppNativeCatchDifficultyCalculator(Structure):
    pass

struct_PpppNativeCatchDifficultyCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeCatchDifficultyCalculator._fields_ = [
    ('handle', POINTER(PpppCatchDifficultyCalculator)),
]

PpppNativeCatchDifficultyCalculator = struct_PpppNativeCatchDifficultyCalculator# include/pppp/capi.h: 237

# include/pppp/capi.h: 241
class struct_PpppNativeManiaDifficultyCalculator(Structure):
    pass

struct_PpppNativeManiaDifficultyCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeManiaDifficultyCalculator._fields_ = [
    ('handle', POINTER(PpppManiaDifficultyCalculator)),
]

PpppNativeManiaDifficultyCalculator = struct_PpppNativeManiaDifficultyCalculator# include/pppp/capi.h: 241

# include/pppp/capi.h: 245
class struct_PpppNativeOsuPerformanceCalculator(Structure):
    pass

struct_PpppNativeOsuPerformanceCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeOsuPerformanceCalculator._fields_ = [
    ('handle', POINTER(PpppOsuPerformanceCalculator)),
]

PpppNativeOsuPerformanceCalculator = struct_PpppNativeOsuPerformanceCalculator# include/pppp/capi.h: 245

# include/pppp/capi.h: 249
class struct_PpppNativeTaikoPerformanceCalculator(Structure):
    pass

struct_PpppNativeTaikoPerformanceCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeTaikoPerformanceCalculator._fields_ = [
    ('handle', POINTER(PpppTaikoPerformanceCalculator)),
]

PpppNativeTaikoPerformanceCalculator = struct_PpppNativeTaikoPerformanceCalculator# include/pppp/capi.h: 249

# include/pppp/capi.h: 253
class struct_PpppNativeCatchPerformanceCalculator(Structure):
    pass

struct_PpppNativeCatchPerformanceCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeCatchPerformanceCalculator._fields_ = [
    ('handle', POINTER(PpppCatchPerformanceCalculator)),
]

PpppNativeCatchPerformanceCalculator = struct_PpppNativeCatchPerformanceCalculator# include/pppp/capi.h: 253

# include/pppp/capi.h: 257
class struct_PpppNativeManiaPerformanceCalculator(Structure):
    pass

struct_PpppNativeManiaPerformanceCalculator.__slots__ = [
    'handle',
]
struct_PpppNativeManiaPerformanceCalculator._fields_ = [
    ('handle', POINTER(PpppManiaPerformanceCalculator)),
]

PpppNativeManiaPerformanceCalculator = struct_PpppNativeManiaPerformanceCalculator# include/pppp/capi.h: 257

# include/pppp/capi.h: 263
for _lib in _libs.values():
    if not _lib.has("Pppp_Version", "cdecl"):
        continue
    Pppp_Version = _lib.get("Pppp_Version", "cdecl")
    Pppp_Version.argtypes = []
    Pppp_Version.restype = c_char_p
    break

# include/pppp/capi.h: 264
for _lib in _libs.values():
    if not _lib.has("Pppp_VersionNumber", "cdecl"):
        continue
    Pppp_VersionNumber = _lib.get("Pppp_VersionNumber", "cdecl")
    Pppp_VersionNumber.argtypes = []
    Pppp_VersionNumber.restype = PpppUint32
    break

# include/pppp/capi.h: 265
for _lib in _libs.values():
    if not _lib.has("Pppp_Backend", "cdecl"):
        continue
    Pppp_Backend = _lib.get("Pppp_Backend", "cdecl")
    Pppp_Backend.argtypes = []
    Pppp_Backend.restype = c_char_p
    break

# include/pppp/capi.h: 267
for _lib in _libs.values():
    if not _lib.has("Pppp_ErrorHandler_GetLastMessage", "cdecl"):
        continue
    Pppp_ErrorHandler_GetLastMessage = _lib.get("Pppp_ErrorHandler_GetLastMessage", "cdecl")
    Pppp_ErrorHandler_GetLastMessage.argtypes = []
    Pppp_ErrorHandler_GetLastMessage.restype = c_char_p
    break

# include/pppp/capi.h: 269
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_CreateFromFile", "cdecl"):
        continue
    Pppp_Beatmap_CreateFromFile = _lib.get("Pppp_Beatmap_CreateFromFile", "cdecl")
    Pppp_Beatmap_CreateFromFile.argtypes = [String, POINTER(PpppNativeBeatmap)]
    Pppp_Beatmap_CreateFromFile.restype = PpppErrorCode
    break

# include/pppp/capi.h: 270
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_CreateFromText", "cdecl"):
        continue
    Pppp_Beatmap_CreateFromText = _lib.get("Pppp_Beatmap_CreateFromText", "cdecl")
    Pppp_Beatmap_CreateFromText.argtypes = [String, POINTER(PpppNativeBeatmap)]
    Pppp_Beatmap_CreateFromText.restype = PpppErrorCode
    break

# include/pppp/capi.h: 271
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_CreateFromBytes", "cdecl"):
        continue
    Pppp_Beatmap_CreateFromBytes = _lib.get("Pppp_Beatmap_CreateFromBytes", "cdecl")
    Pppp_Beatmap_CreateFromBytes.argtypes = [POINTER(None), c_size_t, POINTER(PpppNativeBeatmap)]
    Pppp_Beatmap_CreateFromBytes.restype = PpppErrorCode
    break

# include/pppp/capi.h: 277
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_GetTitle", "cdecl"):
        continue
    Pppp_Beatmap_GetTitle = _lib.get("Pppp_Beatmap_GetTitle", "cdecl")
    Pppp_Beatmap_GetTitle.argtypes = [POINTER(PpppBeatmap), String, POINTER(PpppInt32)]
    Pppp_Beatmap_GetTitle.restype = PpppErrorCode
    break

# include/pppp/capi.h: 278
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_GetArtist", "cdecl"):
        continue
    Pppp_Beatmap_GetArtist = _lib.get("Pppp_Beatmap_GetArtist", "cdecl")
    Pppp_Beatmap_GetArtist.argtypes = [POINTER(PpppBeatmap), String, POINTER(PpppInt32)]
    Pppp_Beatmap_GetArtist.restype = PpppErrorCode
    break

# include/pppp/capi.h: 279
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_GetVersion", "cdecl"):
        continue
    Pppp_Beatmap_GetVersion = _lib.get("Pppp_Beatmap_GetVersion", "cdecl")
    Pppp_Beatmap_GetVersion.argtypes = [POINTER(PpppBeatmap), String, POINTER(PpppInt32)]
    Pppp_Beatmap_GetVersion.restype = PpppErrorCode
    break

# include/pppp/capi.h: 283
for _lib in _libs.values():
    if not _lib.has("Pppp_Beatmap_Destroy", "cdecl"):
        continue
    Pppp_Beatmap_Destroy = _lib.get("Pppp_Beatmap_Destroy", "cdecl")
    Pppp_Beatmap_Destroy.argtypes = [POINTER(PpppBeatmap)]
    Pppp_Beatmap_Destroy.restype = None
    break

# include/pppp/capi.h: 285
for _lib in _libs.values():
    if not _lib.has("Pppp_Ruleset_CreateFromId", "cdecl"):
        continue
    Pppp_Ruleset_CreateFromId = _lib.get("Pppp_Ruleset_CreateFromId", "cdecl")
    Pppp_Ruleset_CreateFromId.argtypes = [PpppInt32, POINTER(PpppNativeRuleset)]
    Pppp_Ruleset_CreateFromId.restype = PpppErrorCode
    break

# include/pppp/capi.h: 289
for _lib in _libs.values():
    if not _lib.has("Pppp_Ruleset_CreateFromShortName", "cdecl"):
        continue
    Pppp_Ruleset_CreateFromShortName = _lib.get("Pppp_Ruleset_CreateFromShortName", "cdecl")
    Pppp_Ruleset_CreateFromShortName.argtypes = [String, POINTER(PpppNativeRuleset)]
    Pppp_Ruleset_CreateFromShortName.restype = PpppErrorCode
    break

# include/pppp/capi.h: 290
for _lib in _libs.values():
    if not _lib.has("Pppp_Ruleset_GetShortName", "cdecl"):
        continue
    Pppp_Ruleset_GetShortName = _lib.get("Pppp_Ruleset_GetShortName", "cdecl")
    Pppp_Ruleset_GetShortName.argtypes = [POINTER(PpppRuleset), String, POINTER(PpppInt32)]
    Pppp_Ruleset_GetShortName.restype = PpppErrorCode
    break

# include/pppp/capi.h: 291
for _lib in _libs.values():
    if not _lib.has("Pppp_Ruleset_Destroy", "cdecl"):
        continue
    Pppp_Ruleset_Destroy = _lib.get("Pppp_Ruleset_Destroy", "cdecl")
    Pppp_Ruleset_Destroy.argtypes = [POINTER(PpppRuleset)]
    Pppp_Ruleset_Destroy.restype = None
    break

# include/pppp/capi.h: 293
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_Create", "cdecl"):
        continue
    Pppp_Mods_Create = _lib.get("Pppp_Mods_Create", "cdecl")
    Pppp_Mods_Create.argtypes = [POINTER(PpppNativeMods)]
    Pppp_Mods_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 294
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_Parse", "cdecl"):
        continue
    Pppp_Mods_Parse = _lib.get("Pppp_Mods_Parse", "cdecl")
    Pppp_Mods_Parse.argtypes = [POINTER(PpppMods), String]
    Pppp_Mods_Parse.restype = PpppErrorCode
    break

# include/pppp/capi.h: 297
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_FromLegacy", "cdecl"):
        continue
    Pppp_Mods_FromLegacy = _lib.get("Pppp_Mods_FromLegacy", "cdecl")
    Pppp_Mods_FromLegacy.argtypes = [POINTER(PpppMods), PpppUint32, PpppInt32]
    Pppp_Mods_FromLegacy.restype = PpppErrorCode
    break

# include/pppp/capi.h: 298
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_AddClassic", "cdecl"):
        continue
    Pppp_Mods_AddClassic = _lib.get("Pppp_Mods_AddClassic", "cdecl")
    Pppp_Mods_AddClassic.argtypes = [POINTER(PpppMods)]
    Pppp_Mods_AddClassic.restype = PpppErrorCode
    break

# include/pppp/capi.h: 299
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_Contains", "cdecl"):
        continue
    Pppp_Mods_Contains = _lib.get("Pppp_Mods_Contains", "cdecl")
    Pppp_Mods_Contains.argtypes = [POINTER(PpppMods), String, POINTER(PpppBool)]
    Pppp_Mods_Contains.restype = PpppErrorCode
    break

# include/pppp/capi.h: 300
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_GetCount", "cdecl"):
        continue
    Pppp_Mods_GetCount = _lib.get("Pppp_Mods_GetCount", "cdecl")
    Pppp_Mods_GetCount.argtypes = [POINTER(PpppMods), POINTER(PpppInt32)]
    Pppp_Mods_GetCount.restype = PpppErrorCode
    break

# include/pppp/capi.h: 301
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_GetAcronym", "cdecl"):
        continue
    Pppp_Mods_GetAcronym = _lib.get("Pppp_Mods_GetAcronym", "cdecl")
    Pppp_Mods_GetAcronym.argtypes = [POINTER(PpppMods), PpppInt32, String, POINTER(PpppInt32)]
    Pppp_Mods_GetAcronym.restype = PpppErrorCode
    break

# include/pppp/capi.h: 302
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_GetClockRate", "cdecl"):
        continue
    Pppp_Mods_GetClockRate = _lib.get("Pppp_Mods_GetClockRate", "cdecl")
    Pppp_Mods_GetClockRate.argtypes = [POINTER(PpppMods), POINTER(c_double)]
    Pppp_Mods_GetClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 305
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_GetLegacyBits", "cdecl"):
        continue
    Pppp_Mods_GetLegacyBits = _lib.get("Pppp_Mods_GetLegacyBits", "cdecl")
    Pppp_Mods_GetLegacyBits.argtypes = [POINTER(PpppMods), PpppInt32, POINTER(PpppUint32)]
    Pppp_Mods_GetLegacyBits.restype = PpppErrorCode
    break

# include/pppp/capi.h: 306
for _lib in _libs.values():
    if not _lib.has("Pppp_Mods_Destroy", "cdecl"):
        continue
    Pppp_Mods_Destroy = _lib.get("Pppp_Mods_Destroy", "cdecl")
    Pppp_Mods_Destroy.argtypes = [POINTER(PpppMods)]
    Pppp_Mods_Destroy.restype = None
    break

# include/pppp/capi.h: 311
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_Create", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_Create = _lib.get("Pppp_OsuDifficultyCalculator_Create", "cdecl")
    Pppp_OsuDifficultyCalculator_Create.argtypes = [POINTER(PpppRuleset), POINTER(PpppBeatmap), POINTER(PpppNativeOsuDifficultyCalculator)]
    Pppp_OsuDifficultyCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 313
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_Calculate", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_Calculate = _lib.get("Pppp_OsuDifficultyCalculator_Calculate", "cdecl")
    Pppp_OsuDifficultyCalculator_Calculate.argtypes = [POINTER(PpppOsuDifficultyCalculator), POINTER(PpppMods), POINTER(PpppOsuDifficultyAttributes)]
    Pppp_OsuDifficultyCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 316
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_CalculateTimed", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_CalculateTimed = _lib.get("Pppp_OsuDifficultyCalculator_CalculateTimed", "cdecl")
    Pppp_OsuDifficultyCalculator_CalculateTimed.argtypes = [POINTER(PpppOsuDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppTimedResult))]
    Pppp_OsuDifficultyCalculator_CalculateTimed.restype = PpppErrorCode
    break

# include/pppp/capi.h: 318
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_CalculateStrains", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_CalculateStrains = _lib.get("Pppp_OsuDifficultyCalculator_CalculateStrains", "cdecl")
    Pppp_OsuDifficultyCalculator_CalculateStrains.argtypes = [POINTER(PpppOsuDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppStrains))]
    Pppp_OsuDifficultyCalculator_CalculateStrains.restype = PpppErrorCode
    break

# include/pppp/capi.h: 320
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_SetClockRate", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_SetClockRate = _lib.get("Pppp_OsuDifficultyCalculator_SetClockRate", "cdecl")
    Pppp_OsuDifficultyCalculator_SetClockRate.argtypes = [POINTER(PpppOsuDifficultyCalculator), c_double]
    Pppp_OsuDifficultyCalculator_SetClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 322
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_ClearClockRate", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_ClearClockRate = _lib.get("Pppp_OsuDifficultyCalculator_ClearClockRate", "cdecl")
    Pppp_OsuDifficultyCalculator_ClearClockRate.argtypes = [POINTER(PpppOsuDifficultyCalculator)]
    Pppp_OsuDifficultyCalculator_ClearClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 323
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuDifficultyCalculator_Destroy", "cdecl"):
        continue
    Pppp_OsuDifficultyCalculator_Destroy = _lib.get("Pppp_OsuDifficultyCalculator_Destroy", "cdecl")
    Pppp_OsuDifficultyCalculator_Destroy.argtypes = [POINTER(PpppOsuDifficultyCalculator)]
    Pppp_OsuDifficultyCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 325
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_Create", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_Create = _lib.get("Pppp_TaikoDifficultyCalculator_Create", "cdecl")
    Pppp_TaikoDifficultyCalculator_Create.argtypes = [POINTER(PpppRuleset), POINTER(PpppBeatmap), POINTER(PpppNativeTaikoDifficultyCalculator)]
    Pppp_TaikoDifficultyCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 327
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_Calculate", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_Calculate = _lib.get("Pppp_TaikoDifficultyCalculator_Calculate", "cdecl")
    Pppp_TaikoDifficultyCalculator_Calculate.argtypes = [POINTER(PpppTaikoDifficultyCalculator), POINTER(PpppMods), POINTER(PpppTaikoDifficultyAttributes)]
    Pppp_TaikoDifficultyCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 330
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_CalculateTimed", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_CalculateTimed = _lib.get("Pppp_TaikoDifficultyCalculator_CalculateTimed", "cdecl")
    Pppp_TaikoDifficultyCalculator_CalculateTimed.argtypes = [POINTER(PpppTaikoDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppTimedResult))]
    Pppp_TaikoDifficultyCalculator_CalculateTimed.restype = PpppErrorCode
    break

# include/pppp/capi.h: 332
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_CalculateStrains", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_CalculateStrains = _lib.get("Pppp_TaikoDifficultyCalculator_CalculateStrains", "cdecl")
    Pppp_TaikoDifficultyCalculator_CalculateStrains.argtypes = [POINTER(PpppTaikoDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppStrains))]
    Pppp_TaikoDifficultyCalculator_CalculateStrains.restype = PpppErrorCode
    break

# include/pppp/capi.h: 335
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_SetClockRate", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_SetClockRate = _lib.get("Pppp_TaikoDifficultyCalculator_SetClockRate", "cdecl")
    Pppp_TaikoDifficultyCalculator_SetClockRate.argtypes = [POINTER(PpppTaikoDifficultyCalculator), c_double]
    Pppp_TaikoDifficultyCalculator_SetClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 337
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_ClearClockRate", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_ClearClockRate = _lib.get("Pppp_TaikoDifficultyCalculator_ClearClockRate", "cdecl")
    Pppp_TaikoDifficultyCalculator_ClearClockRate.argtypes = [POINTER(PpppTaikoDifficultyCalculator)]
    Pppp_TaikoDifficultyCalculator_ClearClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 338
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoDifficultyCalculator_Destroy", "cdecl"):
        continue
    Pppp_TaikoDifficultyCalculator_Destroy = _lib.get("Pppp_TaikoDifficultyCalculator_Destroy", "cdecl")
    Pppp_TaikoDifficultyCalculator_Destroy.argtypes = [POINTER(PpppTaikoDifficultyCalculator)]
    Pppp_TaikoDifficultyCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 340
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_Create", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_Create = _lib.get("Pppp_CatchDifficultyCalculator_Create", "cdecl")
    Pppp_CatchDifficultyCalculator_Create.argtypes = [POINTER(PpppRuleset), POINTER(PpppBeatmap), POINTER(PpppNativeCatchDifficultyCalculator)]
    Pppp_CatchDifficultyCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 342
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_Calculate", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_Calculate = _lib.get("Pppp_CatchDifficultyCalculator_Calculate", "cdecl")
    Pppp_CatchDifficultyCalculator_Calculate.argtypes = [POINTER(PpppCatchDifficultyCalculator), POINTER(PpppMods), POINTER(PpppCatchDifficultyAttributes)]
    Pppp_CatchDifficultyCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 345
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_CalculateTimed", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_CalculateTimed = _lib.get("Pppp_CatchDifficultyCalculator_CalculateTimed", "cdecl")
    Pppp_CatchDifficultyCalculator_CalculateTimed.argtypes = [POINTER(PpppCatchDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppTimedResult))]
    Pppp_CatchDifficultyCalculator_CalculateTimed.restype = PpppErrorCode
    break

# include/pppp/capi.h: 347
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_CalculateStrains", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_CalculateStrains = _lib.get("Pppp_CatchDifficultyCalculator_CalculateStrains", "cdecl")
    Pppp_CatchDifficultyCalculator_CalculateStrains.argtypes = [POINTER(PpppCatchDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppStrains))]
    Pppp_CatchDifficultyCalculator_CalculateStrains.restype = PpppErrorCode
    break

# include/pppp/capi.h: 350
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_SetClockRate", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_SetClockRate = _lib.get("Pppp_CatchDifficultyCalculator_SetClockRate", "cdecl")
    Pppp_CatchDifficultyCalculator_SetClockRate.argtypes = [POINTER(PpppCatchDifficultyCalculator), c_double]
    Pppp_CatchDifficultyCalculator_SetClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 352
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_ClearClockRate", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_ClearClockRate = _lib.get("Pppp_CatchDifficultyCalculator_ClearClockRate", "cdecl")
    Pppp_CatchDifficultyCalculator_ClearClockRate.argtypes = [POINTER(PpppCatchDifficultyCalculator)]
    Pppp_CatchDifficultyCalculator_ClearClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 353
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchDifficultyCalculator_Destroy", "cdecl"):
        continue
    Pppp_CatchDifficultyCalculator_Destroy = _lib.get("Pppp_CatchDifficultyCalculator_Destroy", "cdecl")
    Pppp_CatchDifficultyCalculator_Destroy.argtypes = [POINTER(PpppCatchDifficultyCalculator)]
    Pppp_CatchDifficultyCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 355
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_Create", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_Create = _lib.get("Pppp_ManiaDifficultyCalculator_Create", "cdecl")
    Pppp_ManiaDifficultyCalculator_Create.argtypes = [POINTER(PpppRuleset), POINTER(PpppBeatmap), POINTER(PpppNativeManiaDifficultyCalculator)]
    Pppp_ManiaDifficultyCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 357
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_Calculate", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_Calculate = _lib.get("Pppp_ManiaDifficultyCalculator_Calculate", "cdecl")
    Pppp_ManiaDifficultyCalculator_Calculate.argtypes = [POINTER(PpppManiaDifficultyCalculator), POINTER(PpppMods), POINTER(PpppManiaDifficultyAttributes)]
    Pppp_ManiaDifficultyCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 360
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_CalculateTimed", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_CalculateTimed = _lib.get("Pppp_ManiaDifficultyCalculator_CalculateTimed", "cdecl")
    Pppp_ManiaDifficultyCalculator_CalculateTimed.argtypes = [POINTER(PpppManiaDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppTimedResult))]
    Pppp_ManiaDifficultyCalculator_CalculateTimed.restype = PpppErrorCode
    break

# include/pppp/capi.h: 362
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_CalculateStrains", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_CalculateStrains = _lib.get("Pppp_ManiaDifficultyCalculator_CalculateStrains", "cdecl")
    Pppp_ManiaDifficultyCalculator_CalculateStrains.argtypes = [POINTER(PpppManiaDifficultyCalculator), POINTER(PpppMods), POINTER(POINTER(PpppStrains))]
    Pppp_ManiaDifficultyCalculator_CalculateStrains.restype = PpppErrorCode
    break

# include/pppp/capi.h: 365
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_SetClockRate", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_SetClockRate = _lib.get("Pppp_ManiaDifficultyCalculator_SetClockRate", "cdecl")
    Pppp_ManiaDifficultyCalculator_SetClockRate.argtypes = [POINTER(PpppManiaDifficultyCalculator), c_double]
    Pppp_ManiaDifficultyCalculator_SetClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 367
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_ClearClockRate", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_ClearClockRate = _lib.get("Pppp_ManiaDifficultyCalculator_ClearClockRate", "cdecl")
    Pppp_ManiaDifficultyCalculator_ClearClockRate.argtypes = [POINTER(PpppManiaDifficultyCalculator)]
    Pppp_ManiaDifficultyCalculator_ClearClockRate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 368
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaDifficultyCalculator_Destroy", "cdecl"):
        continue
    Pppp_ManiaDifficultyCalculator_Destroy = _lib.get("Pppp_ManiaDifficultyCalculator_Destroy", "cdecl")
    Pppp_ManiaDifficultyCalculator_Destroy.argtypes = [POINTER(PpppManiaDifficultyCalculator)]
    Pppp_ManiaDifficultyCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 370
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuPerformanceCalculator_Create", "cdecl"):
        continue
    Pppp_OsuPerformanceCalculator_Create = _lib.get("Pppp_OsuPerformanceCalculator_Create", "cdecl")
    Pppp_OsuPerformanceCalculator_Create.argtypes = [POINTER(PpppNativeOsuPerformanceCalculator)]
    Pppp_OsuPerformanceCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 371
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuPerformanceCalculator_Calculate", "cdecl"):
        continue
    Pppp_OsuPerformanceCalculator_Calculate = _lib.get("Pppp_OsuPerformanceCalculator_Calculate", "cdecl")
    Pppp_OsuPerformanceCalculator_Calculate.argtypes = [POINTER(PpppOsuPerformanceCalculator), POINTER(PpppNativeScoreInfo), POINTER(PpppOsuDifficultyAttributes), POINTER(PpppOsuPerformanceAttributes)]
    Pppp_OsuPerformanceCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 374
for _lib in _libs.values():
    if not _lib.has("Pppp_OsuPerformanceCalculator_Destroy", "cdecl"):
        continue
    Pppp_OsuPerformanceCalculator_Destroy = _lib.get("Pppp_OsuPerformanceCalculator_Destroy", "cdecl")
    Pppp_OsuPerformanceCalculator_Destroy.argtypes = [POINTER(PpppOsuPerformanceCalculator)]
    Pppp_OsuPerformanceCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 376
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoPerformanceCalculator_Create", "cdecl"):
        continue
    Pppp_TaikoPerformanceCalculator_Create = _lib.get("Pppp_TaikoPerformanceCalculator_Create", "cdecl")
    Pppp_TaikoPerformanceCalculator_Create.argtypes = [POINTER(PpppNativeTaikoPerformanceCalculator)]
    Pppp_TaikoPerformanceCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 377
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoPerformanceCalculator_Calculate", "cdecl"):
        continue
    Pppp_TaikoPerformanceCalculator_Calculate = _lib.get("Pppp_TaikoPerformanceCalculator_Calculate", "cdecl")
    Pppp_TaikoPerformanceCalculator_Calculate.argtypes = [POINTER(PpppTaikoPerformanceCalculator), POINTER(PpppNativeScoreInfo), POINTER(PpppTaikoDifficultyAttributes), POINTER(PpppTaikoPerformanceAttributes)]
    Pppp_TaikoPerformanceCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 380
for _lib in _libs.values():
    if not _lib.has("Pppp_TaikoPerformanceCalculator_Destroy", "cdecl"):
        continue
    Pppp_TaikoPerformanceCalculator_Destroy = _lib.get("Pppp_TaikoPerformanceCalculator_Destroy", "cdecl")
    Pppp_TaikoPerformanceCalculator_Destroy.argtypes = [POINTER(PpppTaikoPerformanceCalculator)]
    Pppp_TaikoPerformanceCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 382
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchPerformanceCalculator_Create", "cdecl"):
        continue
    Pppp_CatchPerformanceCalculator_Create = _lib.get("Pppp_CatchPerformanceCalculator_Create", "cdecl")
    Pppp_CatchPerformanceCalculator_Create.argtypes = [POINTER(PpppNativeCatchPerformanceCalculator)]
    Pppp_CatchPerformanceCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 383
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchPerformanceCalculator_Calculate", "cdecl"):
        continue
    Pppp_CatchPerformanceCalculator_Calculate = _lib.get("Pppp_CatchPerformanceCalculator_Calculate", "cdecl")
    Pppp_CatchPerformanceCalculator_Calculate.argtypes = [POINTER(PpppCatchPerformanceCalculator), POINTER(PpppNativeScoreInfo), POINTER(PpppCatchDifficultyAttributes), POINTER(PpppCatchPerformanceAttributes)]
    Pppp_CatchPerformanceCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 386
for _lib in _libs.values():
    if not _lib.has("Pppp_CatchPerformanceCalculator_Destroy", "cdecl"):
        continue
    Pppp_CatchPerformanceCalculator_Destroy = _lib.get("Pppp_CatchPerformanceCalculator_Destroy", "cdecl")
    Pppp_CatchPerformanceCalculator_Destroy.argtypes = [POINTER(PpppCatchPerformanceCalculator)]
    Pppp_CatchPerformanceCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 388
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaPerformanceCalculator_Create", "cdecl"):
        continue
    Pppp_ManiaPerformanceCalculator_Create = _lib.get("Pppp_ManiaPerformanceCalculator_Create", "cdecl")
    Pppp_ManiaPerformanceCalculator_Create.argtypes = [POINTER(PpppNativeManiaPerformanceCalculator)]
    Pppp_ManiaPerformanceCalculator_Create.restype = PpppErrorCode
    break

# include/pppp/capi.h: 389
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaPerformanceCalculator_Calculate", "cdecl"):
        continue
    Pppp_ManiaPerformanceCalculator_Calculate = _lib.get("Pppp_ManiaPerformanceCalculator_Calculate", "cdecl")
    Pppp_ManiaPerformanceCalculator_Calculate.argtypes = [POINTER(PpppManiaPerformanceCalculator), POINTER(PpppNativeScoreInfo), POINTER(PpppManiaDifficultyAttributes), POINTER(PpppManiaPerformanceAttributes)]
    Pppp_ManiaPerformanceCalculator_Calculate.restype = PpppErrorCode
    break

# include/pppp/capi.h: 392
for _lib in _libs.values():
    if not _lib.has("Pppp_ManiaPerformanceCalculator_Destroy", "cdecl"):
        continue
    Pppp_ManiaPerformanceCalculator_Destroy = _lib.get("Pppp_ManiaPerformanceCalculator_Destroy", "cdecl")
    Pppp_ManiaPerformanceCalculator_Destroy.argtypes = [POINTER(PpppManiaPerformanceCalculator)]
    Pppp_ManiaPerformanceCalculator_Destroy.restype = None
    break

# include/pppp/capi.h: 394
for _lib in _libs.values():
    if not _lib.has("Pppp_TimedResult_GetCount", "cdecl"):
        continue
    Pppp_TimedResult_GetCount = _lib.get("Pppp_TimedResult_GetCount", "cdecl")
    Pppp_TimedResult_GetCount.argtypes = [POINTER(PpppTimedResult), POINTER(PpppInt32)]
    Pppp_TimedResult_GetCount.restype = PpppErrorCode
    break

# include/pppp/capi.h: 395
for _lib in _libs.values():
    if not _lib.has("Pppp_TimedResult_GetOsu", "cdecl"):
        continue
    Pppp_TimedResult_GetOsu = _lib.get("Pppp_TimedResult_GetOsu", "cdecl")
    Pppp_TimedResult_GetOsu.argtypes = [POINTER(PpppTimedResult), POINTER(PpppTimedOsuDifficultyAttributes), POINTER(PpppInt32)]
    Pppp_TimedResult_GetOsu.restype = PpppErrorCode
    break

# include/pppp/capi.h: 397
for _lib in _libs.values():
    if not _lib.has("Pppp_TimedResult_GetTaiko", "cdecl"):
        continue
    Pppp_TimedResult_GetTaiko = _lib.get("Pppp_TimedResult_GetTaiko", "cdecl")
    Pppp_TimedResult_GetTaiko.argtypes = [POINTER(PpppTimedResult), POINTER(PpppTimedTaikoDifficultyAttributes), POINTER(PpppInt32)]
    Pppp_TimedResult_GetTaiko.restype = PpppErrorCode
    break

# include/pppp/capi.h: 400
for _lib in _libs.values():
    if not _lib.has("Pppp_TimedResult_GetCatch", "cdecl"):
        continue
    Pppp_TimedResult_GetCatch = _lib.get("Pppp_TimedResult_GetCatch", "cdecl")
    Pppp_TimedResult_GetCatch.argtypes = [POINTER(PpppTimedResult), POINTER(PpppTimedCatchDifficultyAttributes), POINTER(PpppInt32)]
    Pppp_TimedResult_GetCatch.restype = PpppErrorCode
    break

# include/pppp/capi.h: 403
for _lib in _libs.values():
    if not _lib.has("Pppp_TimedResult_GetMania", "cdecl"):
        continue
    Pppp_TimedResult_GetMania = _lib.get("Pppp_TimedResult_GetMania", "cdecl")
    Pppp_TimedResult_GetMania.argtypes = [POINTER(PpppTimedResult), POINTER(PpppTimedManiaDifficultyAttributes), POINTER(PpppInt32)]
    Pppp_TimedResult_GetMania.restype = PpppErrorCode
    break

# include/pppp/capi.h: 406
for _lib in _libs.values():
    if not _lib.has("Pppp_TimedResult_Destroy", "cdecl"):
        continue
    Pppp_TimedResult_Destroy = _lib.get("Pppp_TimedResult_Destroy", "cdecl")
    Pppp_TimedResult_Destroy.argtypes = [POINTER(PpppTimedResult)]
    Pppp_TimedResult_Destroy.restype = None
    break

# include/pppp/capi.h: 408
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetRuleset", "cdecl"):
        continue
    Pppp_Strains_GetRuleset = _lib.get("Pppp_Strains_GetRuleset", "cdecl")
    Pppp_Strains_GetRuleset.argtypes = [POINTER(PpppStrains), POINTER(PpppInt32)]
    Pppp_Strains_GetRuleset.restype = PpppErrorCode
    break

# include/pppp/capi.h: 409
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetStartTime", "cdecl"):
        continue
    Pppp_Strains_GetStartTime = _lib.get("Pppp_Strains_GetStartTime", "cdecl")
    Pppp_Strains_GetStartTime.argtypes = [POINTER(PpppStrains), POINTER(c_double)]
    Pppp_Strains_GetStartTime.restype = PpppErrorCode
    break

# include/pppp/capi.h: 410
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetSectionLength", "cdecl"):
        continue
    Pppp_Strains_GetSectionLength = _lib.get("Pppp_Strains_GetSectionLength", "cdecl")
    Pppp_Strains_GetSectionLength.argtypes = [POINTER(PpppStrains), POINTER(c_double)]
    Pppp_Strains_GetSectionLength.restype = PpppErrorCode
    break

# include/pppp/capi.h: 412
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetOsuAim", "cdecl"):
        continue
    Pppp_Strains_GetOsuAim = _lib.get("Pppp_Strains_GetOsuAim", "cdecl")
    Pppp_Strains_GetOsuAim.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetOsuAim.restype = PpppErrorCode
    break

# include/pppp/capi.h: 413
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetOsuAimNoSliders", "cdecl"):
        continue
    Pppp_Strains_GetOsuAimNoSliders = _lib.get("Pppp_Strains_GetOsuAimNoSliders", "cdecl")
    Pppp_Strains_GetOsuAimNoSliders.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetOsuAimNoSliders.restype = PpppErrorCode
    break

# include/pppp/capi.h: 415
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetOsuSpeed", "cdecl"):
        continue
    Pppp_Strains_GetOsuSpeed = _lib.get("Pppp_Strains_GetOsuSpeed", "cdecl")
    Pppp_Strains_GetOsuSpeed.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetOsuSpeed.restype = PpppErrorCode
    break

# include/pppp/capi.h: 416
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetOsuReading", "cdecl"):
        continue
    Pppp_Strains_GetOsuReading = _lib.get("Pppp_Strains_GetOsuReading", "cdecl")
    Pppp_Strains_GetOsuReading.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetOsuReading.restype = PpppErrorCode
    break

# include/pppp/capi.h: 417
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetOsuFlashlight", "cdecl"):
        continue
    Pppp_Strains_GetOsuFlashlight = _lib.get("Pppp_Strains_GetOsuFlashlight", "cdecl")
    Pppp_Strains_GetOsuFlashlight.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetOsuFlashlight.restype = PpppErrorCode
    break

# include/pppp/capi.h: 419
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetTaikoColour", "cdecl"):
        continue
    Pppp_Strains_GetTaikoColour = _lib.get("Pppp_Strains_GetTaikoColour", "cdecl")
    Pppp_Strains_GetTaikoColour.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetTaikoColour.restype = PpppErrorCode
    break

# include/pppp/capi.h: 420
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetTaikoReading", "cdecl"):
        continue
    Pppp_Strains_GetTaikoReading = _lib.get("Pppp_Strains_GetTaikoReading", "cdecl")
    Pppp_Strains_GetTaikoReading.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetTaikoReading.restype = PpppErrorCode
    break

# include/pppp/capi.h: 421
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetTaikoRhythm", "cdecl"):
        continue
    Pppp_Strains_GetTaikoRhythm = _lib.get("Pppp_Strains_GetTaikoRhythm", "cdecl")
    Pppp_Strains_GetTaikoRhythm.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetTaikoRhythm.restype = PpppErrorCode
    break

# include/pppp/capi.h: 422
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetTaikoStamina", "cdecl"):
        continue
    Pppp_Strains_GetTaikoStamina = _lib.get("Pppp_Strains_GetTaikoStamina", "cdecl")
    Pppp_Strains_GetTaikoStamina.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetTaikoStamina.restype = PpppErrorCode
    break

# include/pppp/capi.h: 423
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetTaikoSingleColourStamina", "cdecl"):
        continue
    Pppp_Strains_GetTaikoSingleColourStamina = _lib.get("Pppp_Strains_GetTaikoSingleColourStamina", "cdecl")
    Pppp_Strains_GetTaikoSingleColourStamina.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetTaikoSingleColourStamina.restype = PpppErrorCode
    break

# include/pppp/capi.h: 426
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetCatchMovement", "cdecl"):
        continue
    Pppp_Strains_GetCatchMovement = _lib.get("Pppp_Strains_GetCatchMovement", "cdecl")
    Pppp_Strains_GetCatchMovement.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetCatchMovement.restype = PpppErrorCode
    break

# include/pppp/capi.h: 427
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_GetManiaStrain", "cdecl"):
        continue
    Pppp_Strains_GetManiaStrain = _lib.get("Pppp_Strains_GetManiaStrain", "cdecl")
    Pppp_Strains_GetManiaStrain.argtypes = [POINTER(PpppStrains), POINTER(c_double), POINTER(PpppInt32)]
    Pppp_Strains_GetManiaStrain.restype = PpppErrorCode
    break

# include/pppp/capi.h: 429
for _lib in _libs.values():
    if not _lib.has("Pppp_Strains_Destroy", "cdecl"):
        continue
    Pppp_Strains_Destroy = _lib.get("Pppp_Strains_Destroy", "cdecl")
    Pppp_Strains_Destroy.argtypes = [POINTER(PpppStrains)]
    Pppp_Strains_Destroy.restype = None
    break

PpppBeatmap = struct_PpppBeatmap# include/pppp/capi.h: 59

PpppRuleset = struct_PpppRuleset# include/pppp/capi.h: 60

PpppMods = struct_PpppMods# include/pppp/capi.h: 61

PpppOsuDifficultyCalculator = struct_PpppOsuDifficultyCalculator# include/pppp/capi.h: 62

PpppTaikoDifficultyCalculator = struct_PpppTaikoDifficultyCalculator# include/pppp/capi.h: 63

PpppCatchDifficultyCalculator = struct_PpppCatchDifficultyCalculator# include/pppp/capi.h: 64

PpppManiaDifficultyCalculator = struct_PpppManiaDifficultyCalculator# include/pppp/capi.h: 65

PpppTimedResult = struct_PpppTimedResult# include/pppp/capi.h: 66

PpppStrains = struct_PpppStrains# include/pppp/capi.h: 67

PpppOsuPerformanceCalculator = struct_PpppOsuPerformanceCalculator# include/pppp/capi.h: 68

PpppTaikoPerformanceCalculator = struct_PpppTaikoPerformanceCalculator# include/pppp/capi.h: 69

PpppCatchPerformanceCalculator = struct_PpppCatchPerformanceCalculator# include/pppp/capi.h: 70

PpppManiaPerformanceCalculator = struct_PpppManiaPerformanceCalculator# include/pppp/capi.h: 71

PpppNativeBeatmap = struct_PpppNativeBeatmap# include/pppp/capi.h: 85

PpppNativeRuleset = struct_PpppNativeRuleset# include/pppp/capi.h: 90

PpppNativeMods = struct_PpppNativeMods# include/pppp/capi.h: 94

PpppOptionalInt64 = struct_PpppOptionalInt64# include/pppp/capi.h: 101

PpppOptionalDouble = struct_PpppOptionalDouble# include/pppp/capi.h: 106

PpppNativeScoreInfo = struct_PpppNativeScoreInfo# include/pppp/capi.h: 127

PpppOsuDifficultyAttributes = struct_PpppOsuDifficultyAttributes# include/pppp/capi.h: 151

PpppTaikoDifficultyAttributes = struct_PpppTaikoDifficultyAttributes# include/pppp/capi.h: 164

PpppCatchDifficultyAttributes = struct_PpppCatchDifficultyAttributes# include/pppp/capi.h: 169

PpppManiaDifficultyAttributes = struct_PpppManiaDifficultyAttributes# include/pppp/capi.h: 174

PpppOsuPerformanceAttributes = struct_PpppOsuPerformanceAttributes# include/pppp/capi.h: 189

PpppTaikoPerformanceAttributes = struct_PpppTaikoPerformanceAttributes# include/pppp/capi.h: 196

PpppCatchPerformanceAttributes = struct_PpppCatchPerformanceAttributes# include/pppp/capi.h: 200

PpppManiaPerformanceAttributes = struct_PpppManiaPerformanceAttributes# include/pppp/capi.h: 205

PpppTimedOsuDifficultyAttributes = struct_PpppTimedOsuDifficultyAttributes# include/pppp/capi.h: 210

PpppTimedTaikoDifficultyAttributes = struct_PpppTimedTaikoDifficultyAttributes# include/pppp/capi.h: 215

PpppTimedCatchDifficultyAttributes = struct_PpppTimedCatchDifficultyAttributes# include/pppp/capi.h: 220

PpppTimedManiaDifficultyAttributes = struct_PpppTimedManiaDifficultyAttributes# include/pppp/capi.h: 225

PpppNativeOsuDifficultyCalculator = struct_PpppNativeOsuDifficultyCalculator# include/pppp/capi.h: 229

PpppNativeTaikoDifficultyCalculator = struct_PpppNativeTaikoDifficultyCalculator# include/pppp/capi.h: 233

PpppNativeCatchDifficultyCalculator = struct_PpppNativeCatchDifficultyCalculator# include/pppp/capi.h: 237

PpppNativeManiaDifficultyCalculator = struct_PpppNativeManiaDifficultyCalculator# include/pppp/capi.h: 241

PpppNativeOsuPerformanceCalculator = struct_PpppNativeOsuPerformanceCalculator# include/pppp/capi.h: 245

PpppNativeTaikoPerformanceCalculator = struct_PpppNativeTaikoPerformanceCalculator# include/pppp/capi.h: 249

PpppNativeCatchPerformanceCalculator = struct_PpppNativeCatchPerformanceCalculator# include/pppp/capi.h: 253

PpppNativeManiaPerformanceCalculator = struct_PpppNativeManiaPerformanceCalculator# include/pppp/capi.h: 257

# No inserted files

# No prefix-stripping
