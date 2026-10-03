/** The ruleset a tagged attribute set belongs to; the values are the beatmap's mode. */
export declare const Ruleset: {
  readonly OSU: 0;
  readonly TAIKO: 1;
  readonly CATCH: 2;
  readonly MANIA: 3;
};

export declare const version: string;

export interface Vector2 {
  x: number;
  y: number;
}

export interface BeatmapDifficulty {
  drain_rate: number;
  circle_size: number;
  overall_difficulty: number;
  approach_rate: number;
  slider_multiplier: number;
  slider_tick_rate: number;
}

export interface SliderEvent {
  type: number;
  time: number;
  span_index: number;
  span_start_time: number;
  path_progress: number;
  position: Vector2;
}

export interface Slider {
  slides: number;
  expected_length: number;
  node_sounds: number[];
  control_points: Vector2[];
  path: Vector2[];
  cumulative_lengths: number[];
  undecimated_path: Vector2[];
  undecimated_cumulative_lengths: number[];
  events: SliderEvent[];
  catch_events: SliderEvent[];
}

export interface HitObject {
  position: Vector2;
  type: number;
  hitsound: number;
  start_time: number;
  end_time: number;
  new_combo: boolean;
  combo_offset: number;
  slider: number;
}

export interface TimingPoint {
  time: number;
  beat_length: number;
  meter: number;
  uninherited: boolean;
  effects: number;
}

export interface BreakPeriod {
  start_time: number;
  end_time: number;
}

export interface Beatmap {
  format_version: number;
  mode: number;
  stack_leniency: number;
  difficulty: BeatmapDifficulty;
  hit_objects: HitObject[];
  sliders: Slider[];
  timing_points: TimingPoint[];
  breaks: BreakPeriod[];
}

export interface ScoreInfo {
  statistics: number[];
  maximum_statistics: number[];
  max_combo: number;
  accuracy: number;
  legacy_total_score: number | null;
}

export interface OsuDifficultyAttributes {
  star_rating: number;
  max_combo: number;
  aim_difficulty: number;
  speed_difficulty: number;
  reading_difficulty: number;
  flashlight_difficulty: number;
  slider_factor: number;
  aim_difficult_strain_count: number;
  speed_difficult_strain_count: number;
  reading_difficult_note_count: number;
  aim_difficult_slider_count: number;
  aim_top_weighted_slider_factor: number;
  speed_top_weighted_slider_factor: number;
  speed_note_count: number;
  hit_circle_count: number;
  slider_count: number;
  large_tick_count: number;
  spinner_count: number;
  nested_score_per_object: number;
  legacy_score_base_multiplier: number;
  maximum_legacy_combo_score: number;
}

export interface TaikoDifficultyAttributes {
  star_rating: number;
  max_combo: number;
  mechanical_difficulty: number;
  rhythm_difficulty: number;
  reading_difficulty: number;
  colour_difficulty: number;
  stamina_difficulty: number;
  mono_stamina_factor: number;
  consistency_factor: number;
  stamina_top_strains: number;
}

export interface CatchDifficultyAttributes {
  star_rating: number;
  max_combo: number;
}

export interface ManiaDifficultyAttributes {
  star_rating: number;
  max_combo: number;
}

export interface DifficultyAttributes {
  ruleset: number;
  star_rating: number;
  max_combo: number;
  osu: OsuDifficultyAttributes;
  taiko: TaikoDifficultyAttributes;
  fruits: CatchDifficultyAttributes;
  mania: ManiaDifficultyAttributes;
}

export interface OsuPerformanceAttributes {
  total: number;
  aim: number;
  speed: number;
  accuracy: number;
  flashlight: number;
  reading: number;
  effective_miss_count: number;
  combo_based_estimated_miss_count: number;
  score_based_estimated_miss_count: number | null;
  aim_estimated_slider_breaks: number;
  speed_estimated_slider_breaks: number;
  speed_deviation: number | null;
}

export interface TaikoPerformanceAttributes {
  total: number;
  difficulty: number;
  accuracy: number;
  estimated_unstable_rate: number | null;
}

export interface CatchPerformanceAttributes {
  total: number;
}

export interface ManiaPerformanceAttributes {
  total: number;
  difficulty: number;
}

export interface PerformanceAttributes {
  ruleset: number;
  total: number;
  osu: OsuPerformanceAttributes;
  taiko: TaikoPerformanceAttributes;
  fruits: CatchPerformanceAttributes;
  mania: ManiaPerformanceAttributes;
}

export declare function fromFile(path: string): Beatmap;

export interface DifficultyOptions {
  mods?: string;
  ruleset?: number;
  clockRate?: number;
}

/** Difficulty calculator on maps of any mode. */
export declare class Difficulty {
  constructor(beatmap: Beatmap, options?: DifficultyOptions);

  /** Specify mods, as osu!'s own specification list. */
  mods(spec: string): Difficulty;

  /** Calculate for this ruleset instead of the beatmap's own mode. */
  ruleset(ruleset: number): Difficulty;

  /** Adjust the clock rate used in the calculation. */
  clockRate(rate: number): Difficulty;

  /** Perform the difficulty calculation. */
  calculate(): DifficultyAttributes;
}

export interface PerformanceOptions {
  mods?: string;
  state?: ScoreInfo;
  combo?: number;
  accuracy?: number;
  misses?: number;
  attributes?: DifficultyAttributes;
}

/** Performance calculator on maps of any mode. */
export declare class Performance {
  constructor(beatmap: Beatmap, options?: PerformanceOptions);

  /** Specify mods, as osu!'s own specification list. */
  mods(spec: string): Performance;

  /** Provide the score state through a `ScoreInfo`. */
  state(state: ScoreInfo): Performance;

  /** Specify the max combo of the play. */
  combo(combo: number): Performance;

  /** Set the accuracy between 0.0 and 1.0. */
  accuracy(accuracy: number): Performance;

  /** Use the given already-calculated attributes, skipping the difficulty calculation. */
  attributes(attributes: DifficultyAttributes): Performance;

  /** Specify the amount of misses of the play. */
  misses(misses: number): Performance;

  /** Perform the performance calculation. */
  calculate(): PerformanceAttributes;
}
