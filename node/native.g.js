import * as koffi from "koffi";

const types = {};

types.pppp_vector2 = koffi.struct('pppp_vector2', {
  x: 'float',
  y: 'float',
});

types.pppp_beatmap_difficulty = koffi.struct('pppp_beatmap_difficulty', {
  drain_rate: 'double',
  circle_size: 'double',
  overall_difficulty: 'double',
  approach_rate: 'double',
  slider_multiplier: 'double',
  slider_tick_rate: 'double',
});

types.pppp_slider_event_descriptor = koffi.struct('pppp_slider_event_descriptor', {
  type: 'int32',
  time: 'double',
  span_index: 'int32',
  span_start_time: 'double',
  path_progress: 'double',
  position: types.pppp_vector2,
});

types.pppp_hit_object = koffi.struct('pppp_hit_object', {
  position: types.pppp_vector2,
  type: 'uint32',
  hitsound: 'uint32',
  start_time: 'double',
  end_time: 'double',
  new_combo: 'int32',
  combo_offset: 'int32',
  slider: 'int32',
});

types.pppp_timing_point = koffi.struct('pppp_timing_point', {
  time: 'double',
  beat_length: 'double',
  meter: 'int32',
  uninherited: 'int32',
  effects: 'uint32',
});

types.pppp_break_period = koffi.struct('pppp_break_period', {
  start_time: 'double',
  end_time: 'double',
});

types.pppp_slider = koffi.struct('pppp_slider', {
  slides: 'int32',
  expected_length: 'double',
  node_sounds: koffi.pointer('uint32'),
  node_sound_count: 'size_t',
  control_points: koffi.pointer(types.pppp_vector2),
  control_point_count: 'size_t',
  path: koffi.pointer(types.pppp_vector2),
  path_count: 'size_t',
  cumulative_lengths: koffi.pointer('double'),
  cumulative_length_count: 'size_t',
  undecimated_path: koffi.pointer(types.pppp_vector2),
  undecimated_path_count: 'size_t',
  undecimated_cumulative_lengths: koffi.pointer('double'),
  undecimated_cumulative_length_count: 'size_t',
  events: koffi.pointer(types.pppp_slider_event_descriptor),
  event_count: 'size_t',
  catch_events: koffi.pointer(types.pppp_slider_event_descriptor),
  catch_event_count: 'size_t',
});

types.pppp_score_info = koffi.struct('pppp_score_info', {
  statistics: koffi.array('int32', 18),
  maximum_statistics: koffi.array('int32', 18),
  max_combo: 'int32',
  accuracy: 'double',
  has_legacy_total_score: 'int32',
  legacy_total_score: 'int64',
});

types.pppp_osu_difficulty_attributes = koffi.struct('pppp_osu_difficulty_attributes', {
  aim_difficulty: 'double',
  speed_difficulty: 'double',
  reading_difficulty: 'double',
  flashlight_difficulty: 'double',
  slider_factor: 'double',
  aim_difficult_strain_count: 'double',
  speed_difficult_strain_count: 'double',
  reading_difficult_note_count: 'double',
  aim_difficult_slider_count: 'double',
  aim_top_weighted_slider_factor: 'double',
  speed_top_weighted_slider_factor: 'double',
  speed_note_count: 'double',
  hit_circle_count: 'int32',
  slider_count: 'int32',
  large_tick_count: 'int32',
  spinner_count: 'int32',
  nested_score_per_object: 'double',
  legacy_score_base_multiplier: 'double',
  maximum_legacy_combo_score: 'double',
});

types.pppp_taiko_difficulty_attributes = koffi.struct('pppp_taiko_difficulty_attributes', {
  mechanical_difficulty: 'double',
  rhythm_difficulty: 'double',
  reading_difficulty: 'double',
  colour_difficulty: 'double',
  stamina_difficulty: 'double',
  mono_stamina_factor: 'double',
  consistency_factor: 'double',
  stamina_top_strains: 'double',
});

types.pppp_osu_performance_attributes = koffi.struct('pppp_osu_performance_attributes', {
  aim: 'double',
  speed: 'double',
  accuracy: 'double',
  flashlight: 'double',
  reading: 'double',
  effective_miss_count: 'double',
  combo_based_estimated_miss_count: 'double',
  score_based_estimated_miss_count: 'double',
  has_score_based_estimated_miss_count: 'int32',
  aim_estimated_slider_breaks: 'double',
  speed_estimated_slider_breaks: 'double',
  speed_deviation: 'double',
  has_speed_deviation: 'int32',
});

types.pppp_taiko_performance_attributes = koffi.struct('pppp_taiko_performance_attributes', {
  difficulty: 'double',
  accuracy: 'double',
  estimated_unstable_rate: 'double',
  has_estimated_unstable_rate: 'int32',
});

types.pppp_mania_performance_attributes = koffi.struct('pppp_mania_performance_attributes', {
  difficulty: 'double',
});

types.pppp_difficulty_attributes_union = koffi.union('pppp_difficulty_attributes_union', {
  osu: types.pppp_osu_difficulty_attributes,
  taiko: types.pppp_taiko_difficulty_attributes,
});

types.pppp_difficulty_attributes = koffi.struct('pppp_difficulty_attributes', {
  ruleset: 'int32',
  star_rating: 'double',
  max_combo: 'int32',
  union: types.pppp_difficulty_attributes_union,
});

types.pppp_performance_attributes_union = koffi.union('pppp_performance_attributes_union', {
  osu: types.pppp_osu_performance_attributes,
  taiko: types.pppp_taiko_performance_attributes,
  mania: types.pppp_mania_performance_attributes,
});

types.pppp_performance_attributes = koffi.struct('pppp_performance_attributes', {
  ruleset: 'int32',
  total: 'double',
  union: types.pppp_performance_attributes_union,
});

types.pppp_timed_difficulty_attributes = koffi.struct('pppp_timed_difficulty_attributes', {
  time: 'double',
  attributes: types.pppp_difficulty_attributes,
});

types.pppp_osu_strains = koffi.struct('pppp_osu_strains', {
  aim: koffi.pointer('double'),
  aim_count: 'size_t',
  aim_no_sliders: koffi.pointer('double'),
  aim_no_sliders_count: 'size_t',
  speed: koffi.pointer('double'),
  speed_count: 'size_t',
  reading: koffi.pointer('double'),
  reading_count: 'size_t',
  flashlight: koffi.pointer('double'),
  flashlight_count: 'size_t',
});

types.pppp_taiko_strains = koffi.struct('pppp_taiko_strains', {
  colour: koffi.pointer('double'),
  colour_count: 'size_t',
  reading: koffi.pointer('double'),
  reading_count: 'size_t',
  rhythm: koffi.pointer('double'),
  rhythm_count: 'size_t',
  stamina: koffi.pointer('double'),
  stamina_count: 'size_t',
  single_colour_stamina: koffi.pointer('double'),
  single_colour_stamina_count: 'size_t',
});

types.pppp_catch_strains = koffi.struct('pppp_catch_strains', {
  movement: koffi.pointer('double'),
  movement_count: 'size_t',
});

types.pppp_mania_strains = koffi.struct('pppp_mania_strains', {
  strain: koffi.pointer('double'),
  strain_count: 'size_t',
});

types.pppp_difficulty_options = koffi.struct('pppp_difficulty_options', {
  version: 'uint32',
  mods: 'void *',
  ruleset: 'int32',
  has_ruleset: 'int32',
  clock_rate: 'double',
  has_clock_rate: 'int32',
});

types.pppp_performance_options = koffi.struct('pppp_performance_options', {
  version: 'uint32',
  mods: 'void *',
  score: types.pppp_score_info,
  has_score: 'int32',
  combo: 'int32',
  has_combo: 'int32',
  accuracy: 'double',
  has_accuracy: 'int32',
  misses: 'int32',
  has_misses: 'int32',
  difficulty: koffi.pointer(types.pppp_difficulty_attributes),
  has_difficulty: 'int32',
});

function declare(lib) {
  const fn = {};
  fn.pppp_version = lib.func('pppp_version', 'str', []);
  fn.pppp_abi_version = lib.func('pppp_abi_version', 'int32', []);
  fn.pppp_backend = lib.func('pppp_backend', 'str', []);
  fn.pppp_result_message = lib.func('pppp_result_message', 'str', ['int32']);
  fn.pppp_mods_parse = lib.func('pppp_mods_parse', 'int32', ['str', koffi.out(koffi.pointer('void *'))]);
  fn.pppp_mods_from_legacy = lib.func('pppp_mods_from_legacy', 'int32', ['uint32', 'int32', koffi.out(koffi.pointer('void *'))]);
  fn.pppp_mods_add_classic = lib.func('pppp_mods_add_classic', 'int32', [koffi.out(koffi.pointer('void *'))]);
  fn.pppp_mods_free = lib.func('pppp_mods_free', 'void', [koffi.out(koffi.pointer('void *'))]);
  fn.pppp_beatmap_from_file = lib.func('pppp_beatmap_from_file', 'int32', ['str', koffi.out(koffi.pointer('void *'))]);
  fn.pppp_beatmap_from_bytes = lib.func('pppp_beatmap_from_bytes', 'int32', ['void *', 'size_t', koffi.out(koffi.pointer('void *'))]);
  fn.pppp_beatmap_free = lib.func('pppp_beatmap_free', 'void', [koffi.out(koffi.pointer('void *'))]);
  fn.pppp_beatmap_prepare_data = lib.func('pppp_beatmap_prepare_data', 'int32', [koffi.out(koffi.pointer('void *'))]);
  fn.pppp_beatmap_format_version = lib.func('pppp_beatmap_format_version', 'int32', ['void *']);
  fn.pppp_beatmap_mode = lib.func('pppp_beatmap_mode', 'int32', ['void *']);
  fn.pppp_beatmap_stack_leniency = lib.func('pppp_beatmap_stack_leniency', 'double', ['void *']);
  fn.pppp_beatmap_get_difficulty = lib.func('pppp_beatmap_get_difficulty', 'int32', ['void *', koffi.out(koffi.pointer(types.pppp_beatmap_difficulty))]);
  fn.pppp_beatmap_hit_objects = lib.func('pppp_beatmap_hit_objects', 'int32', ['void *', koffi.out(koffi.pointer('void *')), koffi.out(koffi.pointer('size_t'))]);
  fn.pppp_beatmap_sliders = lib.func('pppp_beatmap_sliders', 'int32', ['void *', koffi.out(koffi.pointer('void *')), koffi.out(koffi.pointer('size_t'))]);
  fn.pppp_beatmap_timing_points = lib.func('pppp_beatmap_timing_points', 'int32', ['void *', koffi.out(koffi.pointer('void *')), koffi.out(koffi.pointer('size_t'))]);
  fn.pppp_beatmap_breaks = lib.func('pppp_beatmap_breaks', 'int32', ['void *', koffi.out(koffi.pointer('void *')), koffi.out(koffi.pointer('size_t'))]);
  fn.pppp_calculate_difficulty = lib.func('pppp_calculate_difficulty', 'int32', ['void *', koffi.pointer(types.pppp_difficulty_options), koffi.out(koffi.pointer(types.pppp_difficulty_attributes))]);
  fn.pppp_calculate_performance = lib.func('pppp_calculate_performance', 'int32', ['void *', koffi.pointer(types.pppp_performance_options), koffi.out(koffi.pointer(types.pppp_performance_attributes))]);
  fn.pppp_calculate_timed_difficulty = lib.func('pppp_calculate_timed_difficulty', 'int32', ['void *', koffi.pointer(types.pppp_difficulty_options), koffi.out(koffi.pointer('void *'))]);
  fn.pppp_timed_difficulty_entries = lib.func('pppp_timed_difficulty_entries', 'int32', ['void *', koffi.out(koffi.pointer('void *')), koffi.out(koffi.pointer('size_t'))]);
  fn.pppp_timed_difficulty_free = lib.func('pppp_timed_difficulty_free', 'void', [koffi.out(koffi.pointer('void *'))]);
  fn.pppp_calculate_strains = lib.func('pppp_calculate_strains', 'int32', ['void *', koffi.pointer(types.pppp_difficulty_options), koffi.out(koffi.pointer('void *'))]);
  fn.pppp_strains_info = lib.func('pppp_strains_info', 'int32', ['void *', koffi.out(koffi.pointer('int32')), koffi.out(koffi.pointer('double')), koffi.out(koffi.pointer('double'))]);
  fn.pppp_strains_osu = lib.func('pppp_strains_osu', 'int32', ['void *', koffi.out(koffi.pointer(types.pppp_osu_strains))]);
  fn.pppp_strains_taiko = lib.func('pppp_strains_taiko', 'int32', ['void *', koffi.out(koffi.pointer(types.pppp_taiko_strains))]);
  fn.pppp_strains_catch = lib.func('pppp_strains_catch', 'int32', ['void *', koffi.out(koffi.pointer(types.pppp_catch_strains))]);
  fn.pppp_strains_mania = lib.func('pppp_strains_mania', 'int32', ['void *', koffi.out(koffi.pointer(types.pppp_mania_strains))]);
  fn.pppp_strains_free = lib.func('pppp_strains_free', 'void', [koffi.out(koffi.pointer('void *'))]);
  return fn;
}

export { declare, types };
