
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

pppp_int64 = c_longlong# include/pppp/capi.h: 38

pppp_uint64 = c_ulonglong# include/pppp/capi.h: 39

pppp_int32 = c_int# include/pppp/capi.h: 42

pppp_uint32 = c_uint# include/pppp/capi.h: 43

enum_pppp_result = c_int# include/pppp/capi.h: 54

PPPP_OK = 0# include/pppp/capi.h: 54

PPPP_INVALID_ARGUMENT = (PPPP_OK + 1)# include/pppp/capi.h: 54

PPPP_ALLOCATION = (PPPP_INVALID_ARGUMENT + 1)# include/pppp/capi.h: 54

PPPP_PARSE = (PPPP_ALLOCATION + 1)# include/pppp/capi.h: 54

PPPP_NO_SLIDER_PATH_BACKEND = (PPPP_PARSE + 1)# include/pppp/capi.h: 54

PPPP_SLIDER_PATH = (PPPP_NO_SLIDER_PATH_BACKEND + 1)# include/pppp/capi.h: 54

PPPP_INTERNAL = (PPPP_SLIDER_PATH + 1)# include/pppp/capi.h: 54

PPPP_NOT_PREPARED = (PPPP_INTERNAL + 1)# include/pppp/capi.h: 54

pppp_result = enum_pppp_result# include/pppp/capi.h: 54

enum_pppp_ruleset = c_int# include/pppp/capi.h: 62

PPPP_RULESET_OSU = 0# include/pppp/capi.h: 62

PPPP_RULESET_TAIKO = (PPPP_RULESET_OSU + 1)# include/pppp/capi.h: 62

PPPP_RULESET_CATCH = (PPPP_RULESET_TAIKO + 1)# include/pppp/capi.h: 62

PPPP_RULESET_MANIA = (PPPP_RULESET_CATCH + 1)# include/pppp/capi.h: 62

pppp_ruleset = enum_pppp_ruleset# include/pppp/capi.h: 62

enum_pppp_slider_event_type = c_int# include/pppp/capi.h: 73

PPPP_SLIDER_EVENT_TICK = 0# include/pppp/capi.h: 73

PPPP_SLIDER_EVENT_LEGACY_LAST_TICK = 1# include/pppp/capi.h: 73

PPPP_SLIDER_EVENT_HEAD = 2# include/pppp/capi.h: 73

PPPP_SLIDER_EVENT_TAIL = 3# include/pppp/capi.h: 73

PPPP_SLIDER_EVENT_REPEAT = 4# include/pppp/capi.h: 73

pppp_slider_event_type = enum_pppp_slider_event_type# include/pppp/capi.h: 73

# include/pppp/capi.h: 84
class struct_pppp_vector2(Structure):
    pass

struct_pppp_vector2.__slots__ = [
    'x',
    'y',
]
struct_pppp_vector2._fields_ = [
    ('x', c_float),
    ('y', c_float),
]

pppp_vector2 = struct_pppp_vector2# include/pppp/capi.h: 84

# include/pppp/capi.h: 100
class struct_pppp_beatmap_difficulty(Structure):
    pass

struct_pppp_beatmap_difficulty.__slots__ = [
    'drain_rate',
    'circle_size',
    'overall_difficulty',
    'approach_rate',
    'slider_multiplier',
    'slider_tick_rate',
]
struct_pppp_beatmap_difficulty._fields_ = [
    ('drain_rate', c_double),
    ('circle_size', c_double),
    ('overall_difficulty', c_double),
    ('approach_rate', c_double),
    ('slider_multiplier', c_double),
    ('slider_tick_rate', c_double),
]

pppp_beatmap_difficulty = struct_pppp_beatmap_difficulty# include/pppp/capi.h: 100

# include/pppp/capi.h: 116
class struct_pppp_slider_event_descriptor(Structure):
    pass

struct_pppp_slider_event_descriptor.__slots__ = [
    'type',
    'time',
    'span_index',
    'span_start_time',
    'path_progress',
    'position',
]
struct_pppp_slider_event_descriptor._fields_ = [
    ('type', pppp_int32),
    ('time', c_double),
    ('span_index', pppp_int32),
    ('span_start_time', c_double),
    ('path_progress', c_double),
    ('position', pppp_vector2),
]

pppp_slider_event_descriptor = struct_pppp_slider_event_descriptor# include/pppp/capi.h: 116

# include/pppp/capi.h: 136
class struct_pppp_hit_object(Structure):
    pass

struct_pppp_hit_object.__slots__ = [
    'position',
    'type',
    'hitsound',
    'start_time',
    'end_time',
    'new_combo',
    'combo_offset',
    'slider',
]
struct_pppp_hit_object._fields_ = [
    ('position', pppp_vector2),
    ('type', pppp_uint32),
    ('hitsound', pppp_uint32),
    ('start_time', c_double),
    ('end_time', c_double),
    ('new_combo', pppp_int32),
    ('combo_offset', pppp_int32),
    ('slider', pppp_int32),
]

pppp_hit_object = struct_pppp_hit_object# include/pppp/capi.h: 136

# include/pppp/capi.h: 150
class struct_pppp_timing_point(Structure):
    pass

struct_pppp_timing_point.__slots__ = [
    'time',
    'beat_length',
    'meter',
    'uninherited',
    'effects',
]
struct_pppp_timing_point._fields_ = [
    ('time', c_double),
    ('beat_length', c_double),
    ('meter', pppp_int32),
    ('uninherited', pppp_int32),
    ('effects', pppp_uint32),
]

pppp_timing_point = struct_pppp_timing_point# include/pppp/capi.h: 150

# include/pppp/capi.h: 158
class struct_pppp_break_period(Structure):
    pass

struct_pppp_break_period.__slots__ = [
    'start_time',
    'end_time',
]
struct_pppp_break_period._fields_ = [
    ('start_time', c_double),
    ('end_time', c_double),
]

pppp_break_period = struct_pppp_break_period# include/pppp/capi.h: 158

# include/pppp/capi.h: 199
class struct_pppp_slider(Structure):
    pass

struct_pppp_slider.__slots__ = [
    'slides',
    'expected_length',
    'node_sounds',
    'node_sound_count',
    'control_points',
    'control_point_count',
    'path',
    'path_count',
    'cumulative_lengths',
    'cumulative_length_count',
    'undecimated_path',
    'undecimated_path_count',
    'undecimated_cumulative_lengths',
    'undecimated_cumulative_length_count',
    'events',
    'event_count',
    'catch_events',
    'catch_event_count',
]
struct_pppp_slider._fields_ = [
    ('slides', pppp_int32),
    ('expected_length', c_double),
    ('node_sounds', POINTER(pppp_uint32)),
    ('node_sound_count', c_size_t),
    ('control_points', POINTER(pppp_vector2)),
    ('control_point_count', c_size_t),
    ('path', POINTER(pppp_vector2)),
    ('path_count', c_size_t),
    ('cumulative_lengths', POINTER(c_double)),
    ('cumulative_length_count', c_size_t),
    ('undecimated_path', POINTER(pppp_vector2)),
    ('undecimated_path_count', c_size_t),
    ('undecimated_cumulative_lengths', POINTER(c_double)),
    ('undecimated_cumulative_length_count', c_size_t),
    ('events', POINTER(pppp_slider_event_descriptor)),
    ('event_count', c_size_t),
    ('catch_events', POINTER(pppp_slider_event_descriptor)),
    ('catch_event_count', c_size_t),
]

pppp_slider = struct_pppp_slider# include/pppp/capi.h: 199

# include/pppp/capi.h: 209
class struct_pppp_score_info(Structure):
    pass

struct_pppp_score_info.__slots__ = [
    'statistics',
    'maximum_statistics',
    'max_combo',
    'accuracy',
    'has_legacy_total_score',
    'legacy_total_score',
]
struct_pppp_score_info._fields_ = [
    ('statistics', pppp_int32 * int(18)),
    ('maximum_statistics', pppp_int32 * int(18)),
    ('max_combo', pppp_int32),
    ('accuracy', c_double),
    ('has_legacy_total_score', pppp_int32),
    ('legacy_total_score', pppp_int64),
]

pppp_score_info = struct_pppp_score_info# include/pppp/capi.h: 209

# include/pppp/capi.h: 252
class struct_pppp_osu_difficulty_attributes(Structure):
    pass

struct_pppp_osu_difficulty_attributes.__slots__ = [
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
struct_pppp_osu_difficulty_attributes._fields_ = [
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
    ('hit_circle_count', pppp_int32),
    ('slider_count', pppp_int32),
    ('large_tick_count', pppp_int32),
    ('spinner_count', pppp_int32),
    ('nested_score_per_object', c_double),
    ('legacy_score_base_multiplier', c_double),
    ('maximum_legacy_combo_score', c_double),
]

pppp_osu_difficulty_attributes = struct_pppp_osu_difficulty_attributes# include/pppp/capi.h: 252

# include/pppp/capi.h: 272
class struct_pppp_taiko_difficulty_attributes(Structure):
    pass

struct_pppp_taiko_difficulty_attributes.__slots__ = [
    'mechanical_difficulty',
    'rhythm_difficulty',
    'reading_difficulty',
    'colour_difficulty',
    'stamina_difficulty',
    'mono_stamina_factor',
    'consistency_factor',
    'stamina_top_strains',
]
struct_pppp_taiko_difficulty_attributes._fields_ = [
    ('mechanical_difficulty', c_double),
    ('rhythm_difficulty', c_double),
    ('reading_difficulty', c_double),
    ('colour_difficulty', c_double),
    ('stamina_difficulty', c_double),
    ('mono_stamina_factor', c_double),
    ('consistency_factor', c_double),
    ('stamina_top_strains', c_double),
]

pppp_taiko_difficulty_attributes = struct_pppp_taiko_difficulty_attributes# include/pppp/capi.h: 272

# include/pppp/capi.h: 300
class struct_pppp_osu_performance_attributes(Structure):
    pass

struct_pppp_osu_performance_attributes.__slots__ = [
    'aim',
    'speed',
    'accuracy',
    'flashlight',
    'reading',
    'effective_miss_count',
    'combo_based_estimated_miss_count',
    'score_based_estimated_miss_count',
    'has_score_based_estimated_miss_count',
    'aim_estimated_slider_breaks',
    'speed_estimated_slider_breaks',
    'speed_deviation',
    'has_speed_deviation',
]
struct_pppp_osu_performance_attributes._fields_ = [
    ('aim', c_double),
    ('speed', c_double),
    ('accuracy', c_double),
    ('flashlight', c_double),
    ('reading', c_double),
    ('effective_miss_count', c_double),
    ('combo_based_estimated_miss_count', c_double),
    ('score_based_estimated_miss_count', c_double),
    ('has_score_based_estimated_miss_count', pppp_int32),
    ('aim_estimated_slider_breaks', c_double),
    ('speed_estimated_slider_breaks', c_double),
    ('speed_deviation', c_double),
    ('has_speed_deviation', pppp_int32),
]

pppp_osu_performance_attributes = struct_pppp_osu_performance_attributes# include/pppp/capi.h: 300

# include/pppp/capi.h: 311
class struct_pppp_taiko_performance_attributes(Structure):
    pass

struct_pppp_taiko_performance_attributes.__slots__ = [
    'difficulty',
    'accuracy',
    'estimated_unstable_rate',
    'has_estimated_unstable_rate',
]
struct_pppp_taiko_performance_attributes._fields_ = [
    ('difficulty', c_double),
    ('accuracy', c_double),
    ('estimated_unstable_rate', c_double),
    ('has_estimated_unstable_rate', pppp_int32),
]

pppp_taiko_performance_attributes = struct_pppp_taiko_performance_attributes# include/pppp/capi.h: 311

# include/pppp/capi.h: 317
class struct_pppp_mania_performance_attributes(Structure):
    pass

struct_pppp_mania_performance_attributes.__slots__ = [
    'difficulty',
]
struct_pppp_mania_performance_attributes._fields_ = [
    ('difficulty', c_double),
]

pppp_mania_performance_attributes = struct_pppp_mania_performance_attributes# include/pppp/capi.h: 317

# include/pppp/capi.h: 327
class union_anon_2(Union):
    pass

union_anon_2.__slots__ = [
    'osu',
    'taiko',
]
union_anon_2._fields_ = [
    ('osu', pppp_osu_difficulty_attributes),
    ('taiko', pppp_taiko_difficulty_attributes),
]

# include/pppp/capi.h: 331
class struct_pppp_difficulty_attributes(Structure):
    pass

struct_pppp_difficulty_attributes.__slots__ = [
    'ruleset',
    'star_rating',
    'max_combo',
    'unnamed_1',
]
struct_pppp_difficulty_attributes._anonymous_ = [
    'unnamed_1',
]
struct_pppp_difficulty_attributes._fields_ = [
    ('ruleset', pppp_int32),
    ('star_rating', c_double),
    ('max_combo', pppp_int32),
    ('unnamed_1', union_anon_2),
]

pppp_difficulty_attributes = struct_pppp_difficulty_attributes# include/pppp/capi.h: 331

# include/pppp/capi.h: 339
class union_anon_3(Union):
    pass

union_anon_3.__slots__ = [
    'osu',
    'taiko',
    'mania',
]
union_anon_3._fields_ = [
    ('osu', pppp_osu_performance_attributes),
    ('taiko', pppp_taiko_performance_attributes),
    ('mania', pppp_mania_performance_attributes),
]

# include/pppp/capi.h: 344
class struct_pppp_performance_attributes(Structure):
    pass

struct_pppp_performance_attributes.__slots__ = [
    'ruleset',
    'total',
    'unnamed_1',
]
struct_pppp_performance_attributes._anonymous_ = [
    'unnamed_1',
]
struct_pppp_performance_attributes._fields_ = [
    ('ruleset', pppp_int32),
    ('total', c_double),
    ('unnamed_1', union_anon_3),
]

pppp_performance_attributes = struct_pppp_performance_attributes# include/pppp/capi.h: 344

# include/pppp/capi.h: 354
class struct_pppp_timed_difficulty_attributes(Structure):
    pass

struct_pppp_timed_difficulty_attributes.__slots__ = [
    'time',
    'attributes',
]
struct_pppp_timed_difficulty_attributes._fields_ = [
    ('time', c_double),
    ('attributes', pppp_difficulty_attributes),
]

pppp_timed_difficulty_attributes = struct_pppp_timed_difficulty_attributes# include/pppp/capi.h: 354

# include/pppp/capi.h: 357
class struct_pppp_timed_difficulty(Structure):
    pass

pppp_timed_difficulty = struct_pppp_timed_difficulty# include/pppp/capi.h: 357

# include/pppp/capi.h: 360
class struct_pppp_strains(Structure):
    pass

pppp_strains = struct_pppp_strains# include/pppp/capi.h: 360

# include/pppp/capi.h: 380
class struct_pppp_osu_strains(Structure):
    pass

struct_pppp_osu_strains.__slots__ = [
    'aim',
    'aim_count',
    'aim_no_sliders',
    'aim_no_sliders_count',
    'speed',
    'speed_count',
    'reading',
    'reading_count',
    'flashlight',
    'flashlight_count',
]
struct_pppp_osu_strains._fields_ = [
    ('aim', POINTER(c_double)),
    ('aim_count', c_size_t),
    ('aim_no_sliders', POINTER(c_double)),
    ('aim_no_sliders_count', c_size_t),
    ('speed', POINTER(c_double)),
    ('speed_count', c_size_t),
    ('reading', POINTER(c_double)),
    ('reading_count', c_size_t),
    ('flashlight', POINTER(c_double)),
    ('flashlight_count', c_size_t),
]

pppp_osu_strains = struct_pppp_osu_strains# include/pppp/capi.h: 380

# include/pppp/capi.h: 399
class struct_pppp_taiko_strains(Structure):
    pass

struct_pppp_taiko_strains.__slots__ = [
    'colour',
    'colour_count',
    'reading',
    'reading_count',
    'rhythm',
    'rhythm_count',
    'stamina',
    'stamina_count',
    'single_colour_stamina',
    'single_colour_stamina_count',
]
struct_pppp_taiko_strains._fields_ = [
    ('colour', POINTER(c_double)),
    ('colour_count', c_size_t),
    ('reading', POINTER(c_double)),
    ('reading_count', c_size_t),
    ('rhythm', POINTER(c_double)),
    ('rhythm_count', c_size_t),
    ('stamina', POINTER(c_double)),
    ('stamina_count', c_size_t),
    ('single_colour_stamina', POINTER(c_double)),
    ('single_colour_stamina_count', c_size_t),
]

pppp_taiko_strains = struct_pppp_taiko_strains# include/pppp/capi.h: 399

# include/pppp/capi.h: 406
class struct_pppp_catch_strains(Structure):
    pass

struct_pppp_catch_strains.__slots__ = [
    'movement',
    'movement_count',
]
struct_pppp_catch_strains._fields_ = [
    ('movement', POINTER(c_double)),
    ('movement_count', c_size_t),
]

pppp_catch_strains = struct_pppp_catch_strains# include/pppp/capi.h: 406

# include/pppp/capi.h: 413
class struct_pppp_mania_strains(Structure):
    pass

struct_pppp_mania_strains.__slots__ = [
    'strain',
    'strain_count',
]
struct_pppp_mania_strains._fields_ = [
    ('strain', POINTER(c_double)),
    ('strain_count', c_size_t),
]

pppp_mania_strains = struct_pppp_mania_strains# include/pppp/capi.h: 413

# include/pppp/capi.h: 417
class struct_pppp_mods(Structure):
    pass

pppp_mods = struct_pppp_mods# include/pppp/capi.h: 417

# include/pppp/capi.h: 429
class struct_pppp_difficulty_options(Structure):
    pass

struct_pppp_difficulty_options.__slots__ = [
    'version',
    'mods',
    'ruleset',
    'has_ruleset',
    'clock_rate',
    'has_clock_rate',
]
struct_pppp_difficulty_options._fields_ = [
    ('version', pppp_uint32),
    ('mods', POINTER(pppp_mods)),
    ('ruleset', pppp_int32),
    ('has_ruleset', pppp_int32),
    ('clock_rate', c_double),
    ('has_clock_rate', pppp_int32),
]

pppp_difficulty_options = struct_pppp_difficulty_options# include/pppp/capi.h: 429

# include/pppp/capi.h: 451
class struct_pppp_performance_options(Structure):
    pass

struct_pppp_performance_options.__slots__ = [
    'version',
    'mods',
    'score',
    'has_score',
    'combo',
    'has_combo',
    'accuracy',
    'has_accuracy',
    'misses',
    'has_misses',
    'difficulty',
    'has_difficulty',
]
struct_pppp_performance_options._fields_ = [
    ('version', pppp_uint32),
    ('mods', POINTER(pppp_mods)),
    ('score', pppp_score_info),
    ('has_score', pppp_int32),
    ('combo', pppp_int32),
    ('has_combo', pppp_int32),
    ('accuracy', c_double),
    ('has_accuracy', pppp_int32),
    ('misses', pppp_int32),
    ('has_misses', pppp_int32),
    ('difficulty', POINTER(pppp_difficulty_attributes)),
    ('has_difficulty', pppp_int32),
]

pppp_performance_options = struct_pppp_performance_options# include/pppp/capi.h: 451

# include/pppp/capi.h: 462
class struct_pppp_beatmap(Structure):
    pass

pppp_beatmap = struct_pppp_beatmap# include/pppp/capi.h: 462

# include/pppp/capi.h: 469
for _lib in _libs.values():
    if not _lib.has("pppp_version", "cdecl"):
        continue
    pppp_version = _lib.get("pppp_version", "cdecl")
    pppp_version.argtypes = []
    pppp_version.restype = c_char_p
    break

# include/pppp/capi.h: 472
for _lib in _libs.values():
    if not _lib.has("pppp_abi_version", "cdecl"):
        continue
    pppp_abi_version = _lib.get("pppp_abi_version", "cdecl")
    pppp_abi_version.argtypes = []
    pppp_abi_version.restype = pppp_int32
    break

# include/pppp/capi.h: 476
for _lib in _libs.values():
    if not _lib.has("pppp_backend", "cdecl"):
        continue
    pppp_backend = _lib.get("pppp_backend", "cdecl")
    pppp_backend.argtypes = []
    pppp_backend.restype = c_char_p
    break

# include/pppp/capi.h: 479
for _lib in _libs.values():
    if not _lib.has("pppp_result_message", "cdecl"):
        continue
    pppp_result_message = _lib.get("pppp_result_message", "cdecl")
    pppp_result_message.argtypes = [pppp_result]
    pppp_result_message.restype = c_char_p
    break

# include/pppp/capi.h: 484
for _lib in _libs.values():
    if not _lib.has("pppp_mods_parse", "cdecl"):
        continue
    pppp_mods_parse = _lib.get("pppp_mods_parse", "cdecl")
    pppp_mods_parse.argtypes = [String, POINTER(POINTER(pppp_mods))]
    pppp_mods_parse.restype = pppp_result
    break

# include/pppp/capi.h: 487
for _lib in _libs.values():
    if not _lib.has("pppp_mods_from_legacy", "cdecl"):
        continue
    pppp_mods_from_legacy = _lib.get("pppp_mods_from_legacy", "cdecl")
    pppp_mods_from_legacy.argtypes = [pppp_uint32, pppp_int32, POINTER(POINTER(pppp_mods))]
    pppp_mods_from_legacy.restype = pppp_result
    break

# include/pppp/capi.h: 492
for _lib in _libs.values():
    if not _lib.has("pppp_mods_add_classic", "cdecl"):
        continue
    pppp_mods_add_classic = _lib.get("pppp_mods_add_classic", "cdecl")
    pppp_mods_add_classic.argtypes = [POINTER(pppp_mods)]
    pppp_mods_add_classic.restype = pppp_result
    break

# include/pppp/capi.h: 495
for _lib in _libs.values():
    if not _lib.has("pppp_mods_free", "cdecl"):
        continue
    pppp_mods_free = _lib.get("pppp_mods_free", "cdecl")
    pppp_mods_free.argtypes = [POINTER(pppp_mods)]
    pppp_mods_free.restype = None
    break

# include/pppp/capi.h: 501
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_from_file", "cdecl"):
        continue
    pppp_beatmap_from_file = _lib.get("pppp_beatmap_from_file", "cdecl")
    pppp_beatmap_from_file.argtypes = [String, POINTER(POINTER(pppp_beatmap))]
    pppp_beatmap_from_file.restype = pppp_result
    break

# include/pppp/capi.h: 506
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_from_bytes", "cdecl"):
        continue
    pppp_beatmap_from_bytes = _lib.get("pppp_beatmap_from_bytes", "cdecl")
    pppp_beatmap_from_bytes.argtypes = [POINTER(None), c_size_t, POINTER(POINTER(pppp_beatmap))]
    pppp_beatmap_from_bytes.restype = pppp_result
    break

# include/pppp/capi.h: 509
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_free", "cdecl"):
        continue
    pppp_beatmap_free = _lib.get("pppp_beatmap_free", "cdecl")
    pppp_beatmap_free.argtypes = [POINTER(pppp_beatmap)]
    pppp_beatmap_free.restype = None
    break

# include/pppp/capi.h: 513
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_prepare_data", "cdecl"):
        continue
    pppp_beatmap_prepare_data = _lib.get("pppp_beatmap_prepare_data", "cdecl")
    pppp_beatmap_prepare_data.argtypes = [POINTER(pppp_beatmap)]
    pppp_beatmap_prepare_data.restype = pppp_result
    break

# include/pppp/capi.h: 515
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_format_version", "cdecl"):
        continue
    pppp_beatmap_format_version = _lib.get("pppp_beatmap_format_version", "cdecl")
    pppp_beatmap_format_version.argtypes = [POINTER(pppp_beatmap)]
    pppp_beatmap_format_version.restype = pppp_int32
    break

# include/pppp/capi.h: 516
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_mode", "cdecl"):
        continue
    pppp_beatmap_mode = _lib.get("pppp_beatmap_mode", "cdecl")
    pppp_beatmap_mode.argtypes = [POINTER(pppp_beatmap)]
    pppp_beatmap_mode.restype = pppp_int32
    break

# include/pppp/capi.h: 517
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_stack_leniency", "cdecl"):
        continue
    pppp_beatmap_stack_leniency = _lib.get("pppp_beatmap_stack_leniency", "cdecl")
    pppp_beatmap_stack_leniency.argtypes = [POINTER(pppp_beatmap)]
    pppp_beatmap_stack_leniency.restype = c_double
    break

# include/pppp/capi.h: 518
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_get_difficulty", "cdecl"):
        continue
    pppp_beatmap_get_difficulty = _lib.get("pppp_beatmap_get_difficulty", "cdecl")
    pppp_beatmap_get_difficulty.argtypes = [POINTER(pppp_beatmap), POINTER(pppp_beatmap_difficulty)]
    pppp_beatmap_get_difficulty.restype = pppp_result
    break

# include/pppp/capi.h: 524
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_hit_objects", "cdecl"):
        continue
    pppp_beatmap_hit_objects = _lib.get("pppp_beatmap_hit_objects", "cdecl")
    pppp_beatmap_hit_objects.argtypes = [POINTER(pppp_beatmap), POINTER(POINTER(pppp_hit_object)), POINTER(c_size_t)]
    pppp_beatmap_hit_objects.restype = pppp_result
    break

# include/pppp/capi.h: 526
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_sliders", "cdecl"):
        continue
    pppp_beatmap_sliders = _lib.get("pppp_beatmap_sliders", "cdecl")
    pppp_beatmap_sliders.argtypes = [POINTER(pppp_beatmap), POINTER(POINTER(pppp_slider)), POINTER(c_size_t)]
    pppp_beatmap_sliders.restype = pppp_result
    break

# include/pppp/capi.h: 527
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_timing_points", "cdecl"):
        continue
    pppp_beatmap_timing_points = _lib.get("pppp_beatmap_timing_points", "cdecl")
    pppp_beatmap_timing_points.argtypes = [POINTER(pppp_beatmap), POINTER(POINTER(pppp_timing_point)), POINTER(c_size_t)]
    pppp_beatmap_timing_points.restype = pppp_result
    break

# include/pppp/capi.h: 529
for _lib in _libs.values():
    if not _lib.has("pppp_beatmap_breaks", "cdecl"):
        continue
    pppp_beatmap_breaks = _lib.get("pppp_beatmap_breaks", "cdecl")
    pppp_beatmap_breaks.argtypes = [POINTER(pppp_beatmap), POINTER(POINTER(pppp_break_period)), POINTER(c_size_t)]
    pppp_beatmap_breaks.restype = pppp_result
    break

# include/pppp/capi.h: 534
for _lib in _libs.values():
    if not _lib.has("pppp_calculate_difficulty", "cdecl"):
        continue
    pppp_calculate_difficulty = _lib.get("pppp_calculate_difficulty", "cdecl")
    pppp_calculate_difficulty.argtypes = [POINTER(pppp_beatmap), POINTER(pppp_difficulty_options), POINTER(pppp_difficulty_attributes)]
    pppp_calculate_difficulty.restype = pppp_result
    break

# include/pppp/capi.h: 539
for _lib in _libs.values():
    if not _lib.has("pppp_calculate_performance", "cdecl"):
        continue
    pppp_calculate_performance = _lib.get("pppp_calculate_performance", "cdecl")
    pppp_calculate_performance.argtypes = [POINTER(pppp_beatmap), POINTER(pppp_performance_options), POINTER(pppp_performance_attributes)]
    pppp_calculate_performance.restype = pppp_result
    break

# include/pppp/capi.h: 548
for _lib in _libs.values():
    if not _lib.has("pppp_calculate_timed_difficulty", "cdecl"):
        continue
    pppp_calculate_timed_difficulty = _lib.get("pppp_calculate_timed_difficulty", "cdecl")
    pppp_calculate_timed_difficulty.argtypes = [POINTER(pppp_beatmap), POINTER(pppp_difficulty_options), POINTER(POINTER(pppp_timed_difficulty))]
    pppp_calculate_timed_difficulty.restype = pppp_result
    break

# include/pppp/capi.h: 554
for _lib in _libs.values():
    if not _lib.has("pppp_timed_difficulty_entries", "cdecl"):
        continue
    pppp_timed_difficulty_entries = _lib.get("pppp_timed_difficulty_entries", "cdecl")
    pppp_timed_difficulty_entries.argtypes = [POINTER(pppp_timed_difficulty), POINTER(POINTER(pppp_timed_difficulty_attributes)), POINTER(c_size_t)]
    pppp_timed_difficulty_entries.restype = pppp_result
    break

# include/pppp/capi.h: 559
for _lib in _libs.values():
    if not _lib.has("pppp_timed_difficulty_free", "cdecl"):
        continue
    pppp_timed_difficulty_free = _lib.get("pppp_timed_difficulty_free", "cdecl")
    pppp_timed_difficulty_free.argtypes = [POINTER(pppp_timed_difficulty)]
    pppp_timed_difficulty_free.restype = None
    break

# include/pppp/capi.h: 564
for _lib in _libs.values():
    if not _lib.has("pppp_calculate_strains", "cdecl"):
        continue
    pppp_calculate_strains = _lib.get("pppp_calculate_strains", "cdecl")
    pppp_calculate_strains.argtypes = [POINTER(pppp_beatmap), POINTER(pppp_difficulty_options), POINTER(POINTER(pppp_strains))]
    pppp_calculate_strains.restype = pppp_result
    break

# include/pppp/capi.h: 569
for _lib in _libs.values():
    if not _lib.has("pppp_strains_info", "cdecl"):
        continue
    pppp_strains_info = _lib.get("pppp_strains_info", "cdecl")
    pppp_strains_info.argtypes = [POINTER(pppp_strains), POINTER(pppp_int32), POINTER(c_double), POINTER(c_double)]
    pppp_strains_info.restype = pppp_result
    break

# include/pppp/capi.h: 575
for _lib in _libs.values():
    if not _lib.has("pppp_strains_osu", "cdecl"):
        continue
    pppp_strains_osu = _lib.get("pppp_strains_osu", "cdecl")
    pppp_strains_osu.argtypes = [POINTER(pppp_strains), POINTER(pppp_osu_strains)]
    pppp_strains_osu.restype = pppp_result
    break

# include/pppp/capi.h: 576
for _lib in _libs.values():
    if not _lib.has("pppp_strains_taiko", "cdecl"):
        continue
    pppp_strains_taiko = _lib.get("pppp_strains_taiko", "cdecl")
    pppp_strains_taiko.argtypes = [POINTER(pppp_strains), POINTER(pppp_taiko_strains)]
    pppp_strains_taiko.restype = pppp_result
    break

# include/pppp/capi.h: 577
for _lib in _libs.values():
    if not _lib.has("pppp_strains_catch", "cdecl"):
        continue
    pppp_strains_catch = _lib.get("pppp_strains_catch", "cdecl")
    pppp_strains_catch.argtypes = [POINTER(pppp_strains), POINTER(pppp_catch_strains)]
    pppp_strains_catch.restype = pppp_result
    break

# include/pppp/capi.h: 578
for _lib in _libs.values():
    if not _lib.has("pppp_strains_mania", "cdecl"):
        continue
    pppp_strains_mania = _lib.get("pppp_strains_mania", "cdecl")
    pppp_strains_mania.argtypes = [POINTER(pppp_strains), POINTER(pppp_mania_strains)]
    pppp_strains_mania.restype = pppp_result
    break

# include/pppp/capi.h: 581
for _lib in _libs.values():
    if not _lib.has("pppp_strains_free", "cdecl"):
        continue
    pppp_strains_free = _lib.get("pppp_strains_free", "cdecl")
    pppp_strains_free.argtypes = [POINTER(pppp_strains)]
    pppp_strains_free.restype = None
    break

# include/pppp/capi.h: 26
try:
    PPPP_C_ABI_VERSION = 1
except:
    pass

# include/pppp/capi.h: 76
try:
    PPPP_HIT_RESULT_COUNT = 18
except:
    pass

# include/pppp/capi.h: 453
try:
    PPPP_DIFFICULTY_OPTIONS_VERSION = 1
except:
    pass

# include/pppp/capi.h: 454
try:
    PPPP_PERFORMANCE_OPTIONS_VERSION = 1
except:
    pass

pppp_vector2 = struct_pppp_vector2# include/pppp/capi.h: 84

pppp_beatmap_difficulty = struct_pppp_beatmap_difficulty# include/pppp/capi.h: 100

pppp_slider_event_descriptor = struct_pppp_slider_event_descriptor# include/pppp/capi.h: 116

pppp_hit_object = struct_pppp_hit_object# include/pppp/capi.h: 136

pppp_timing_point = struct_pppp_timing_point# include/pppp/capi.h: 150

pppp_break_period = struct_pppp_break_period# include/pppp/capi.h: 158

pppp_slider = struct_pppp_slider# include/pppp/capi.h: 199

pppp_score_info = struct_pppp_score_info# include/pppp/capi.h: 209

pppp_osu_difficulty_attributes = struct_pppp_osu_difficulty_attributes# include/pppp/capi.h: 252

pppp_taiko_difficulty_attributes = struct_pppp_taiko_difficulty_attributes# include/pppp/capi.h: 272

pppp_osu_performance_attributes = struct_pppp_osu_performance_attributes# include/pppp/capi.h: 300

pppp_taiko_performance_attributes = struct_pppp_taiko_performance_attributes# include/pppp/capi.h: 311

pppp_mania_performance_attributes = struct_pppp_mania_performance_attributes# include/pppp/capi.h: 317

pppp_difficulty_attributes = struct_pppp_difficulty_attributes# include/pppp/capi.h: 331

pppp_performance_attributes = struct_pppp_performance_attributes# include/pppp/capi.h: 344

pppp_timed_difficulty_attributes = struct_pppp_timed_difficulty_attributes# include/pppp/capi.h: 354

pppp_timed_difficulty = struct_pppp_timed_difficulty# include/pppp/capi.h: 357

pppp_strains = struct_pppp_strains# include/pppp/capi.h: 360

pppp_osu_strains = struct_pppp_osu_strains# include/pppp/capi.h: 380

pppp_taiko_strains = struct_pppp_taiko_strains# include/pppp/capi.h: 399

pppp_catch_strains = struct_pppp_catch_strains# include/pppp/capi.h: 406

pppp_mania_strains = struct_pppp_mania_strains# include/pppp/capi.h: 413

pppp_mods = struct_pppp_mods# include/pppp/capi.h: 417

pppp_difficulty_options = struct_pppp_difficulty_options# include/pppp/capi.h: 429

pppp_performance_options = struct_pppp_performance_options# include/pppp/capi.h: 451

pppp_beatmap = struct_pppp_beatmap# include/pppp/capi.h: 462

# No inserted files

# No prefix-stripping

