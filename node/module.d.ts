/** Which ruleset a tagged attribute set belongs to. The values are the beatmap's `Mode`. */
export declare const Ruleset: {
  readonly Osu: 0;
  readonly Taiko: 1;
  readonly Catch: 2;
  readonly Mania: 3;
};
export type Ruleset = (typeof Ruleset)[keyof typeof Ruleset];

export declare const version: string;

export interface Vector2 {
  x: number;
  y: number;
}

export interface BeatmapDifficulty {
  drainRate: number;
  circleSize: number;
  overallDifficulty: number;
  approachRate: number;
  sliderMultiplier: number;
  sliderTickRate: number;
}

export interface SliderEvent {
  type: number;
  time: number;
  spanIndex: number;
  spanStartTime: number;
  pathProgress: number;
  position: Vector2;
}

export interface Slider {
  slides: number;
  expectedLength: number;
  nodeSounds: number[];
  controlPoints: Vector2[];
  path: Vector2[];
  cumulativeLengths: number[];
  undecimatedPath: Vector2[];
  undecimatedCumulativeLengths: number[];
  events: SliderEvent[];
  catchEvents: SliderEvent[];
}

export interface HitObject {
  position: Vector2;
  type: number;
  hitsound: number;
  startTime: number;
  endTime: number;
  newCombo: boolean;
  comboOffset: number;
  slider: number;
}

export interface TimingPoint {
  time: number;
  beatLength: number;
  meter: number;
  uninherited: boolean;
  effects: number;
}

export interface BreakPeriod {
  startTime: number;
  endTime: number;
}

/** A loaded beatmap. */
export declare class Beatmap {
  private constructor();

  /** Parse a `Beatmap` by providing the content of a `.osu` file as a slice of bytes. */
  static fromBytes(data: Uint8Array): Beatmap;

  /** Parse a `Beatmap` by providing a path to a `.osu` file. */
  static fromFileSync(path: string | URL | Uint8Array): Beatmap;

  /** Parse a `Beatmap` by providing a path to a `.osu` file. */
  static fromFile(path: string | URL | Uint8Array): Promise<Beatmap>;

  readonly formatVersion: number;
  readonly mode: number;
  readonly stackLeniency: number;
  readonly difficulty: BeatmapDifficulty;
  readonly hitObjects: HitObject[];
  readonly sliders: Slider[];
  readonly timingPoints: TimingPoint[];
  readonly breaks: BreakPeriod[];
}

export interface OsuDifficultyAttributes {
  ruleset: typeof Ruleset.Osu;
  starRating: number;
  maxCombo: number;
  aimDifficulty: number;
  speedDifficulty: number;
  readingDifficulty: number;
  flashlightDifficulty: number;
  sliderFactor: number;
  aimDifficultStrainCount: number;
  speedDifficultStrainCount: number;
  readingDifficultNoteCount: number;
  aimDifficultSliderCount: number;
  aimTopWeightedSliderFactor: number;
  speedTopWeightedSliderFactor: number;
  speedNoteCount: number;
  hitCircleCount: number;
  sliderCount: number;
  largeTickCount: number;
  spinnerCount: number;
  nestedScorePerObject: number;
  legacyScoreBaseMultiplier: number;
  maximumLegacyComboScore: number;
}

export interface TaikoDifficultyAttributes {
  ruleset: typeof Ruleset.Taiko;
  starRating: number;
  maxCombo: number;
  mechanicalDifficulty: number;
  rhythmDifficulty: number;
  readingDifficulty: number;
  colourDifficulty: number;
  staminaDifficulty: number;
  monoStaminaFactor: number;
  consistencyFactor: number;
  staminaTopStrains: number;
}

export interface CatchDifficultyAttributes {
  ruleset: typeof Ruleset.Catch;
  starRating: number;
  maxCombo: number;
}

export interface ManiaDifficultyAttributes {
  ruleset: typeof Ruleset.Mania;
  starRating: number;
  maxCombo: number;
}

export type DifficultyAttributes =
  | OsuDifficultyAttributes
  | TaikoDifficultyAttributes
  | CatchDifficultyAttributes
  | ManiaDifficultyAttributes;

/**
 * Wraps a DifficultyAttributes object and adds a time value for which the attribute is valid.
 * Output by `Difficulty.calculateTimed`.
 */
export interface TimedDifficultyAttributes {
  /** The non-clock-adjusted time value at which the attributes take effect. */
  time: number;
  /** The attributes. */
  attributes: DifficultyAttributes;
}

/**
 * The result of calculating the strains on a osu! map.
 *
 * Suitable to plot the difficulty of a map over time.
 */
export interface OsuStrains {
  ruleset: typeof Ruleset.Osu;
  startTime: number;
  sectionLength: number;
  aim: number[];
  aimNoSliders: number[];
  speed: number[];
  reading: number[];
  flashlight: number[];
}

/**
 * The result of calculating the strains on a osu!taiko map.
 *
 * Suitable to plot the difficulty of a map over time.
 */
export interface TaikoStrains {
  ruleset: typeof Ruleset.Taiko;
  startTime: number;
  sectionLength: number;
  colour: number[];
  reading: number[];
  rhythm: number[];
  stamina: number[];
  singleColourStamina: number[];
}

/**
 * The result of calculating the strains on a osu!catch map.
 *
 * Suitable to plot the difficulty of a map over time.
 */
export interface CatchStrains {
  ruleset: typeof Ruleset.Catch;
  startTime: number;
  sectionLength: number;
  movement: number[];
}

/**
 * The result of calculating the strains on a osu!mania map.
 *
 * Suitable to plot the difficulty of a map over time.
 */
export interface ManiaStrains {
  ruleset: typeof Ruleset.Mania;
  startTime: number;
  sectionLength: number;
  strain: number[];
}

/**
 * The result of calculating the strains on a map.
 *
 * Suitable to plot the difficulty of a map over time.
 */
export type Strains = OsuStrains | TaikoStrains | CatchStrains | ManiaStrains;

export interface OsuPerformanceAttributes {
  ruleset: typeof Ruleset.Osu;
  total: number;
  aim: number;
  speed: number;
  accuracy: number;
  flashlight: number;
  reading: number;
  effectiveMissCount: number;
  comboBasedEstimatedMissCount: number;
  scoreBasedEstimatedMissCount: number | null;
  aimEstimatedSliderBreaks: number;
  speedEstimatedSliderBreaks: number;
  speedDeviation: number | null;
}

export interface TaikoPerformanceAttributes {
  ruleset: typeof Ruleset.Taiko;
  total: number;
  difficulty: number;
  accuracy: number;
  estimatedUnstableRate: number | null;
}

export interface CatchPerformanceAttributes {
  ruleset: typeof Ruleset.Catch;
  total: number;
}

export interface ManiaPerformanceAttributes {
  ruleset: typeof Ruleset.Mania;
  total: number;
  difficulty: number;
}

export type PerformanceAttributes =
  | OsuPerformanceAttributes
  | TaikoPerformanceAttributes
  | CatchPerformanceAttributes
  | ManiaPerformanceAttributes;

export interface Statistics {
  none?: number;
  miss?: number;
  meh?: number;
  ok?: number;
  good?: number;
  great?: number;
  perfect?: number;
  smallTickMiss?: number;
  smallTickHit?: number;
  largeTickMiss?: number;
  largeTickHit?: number;
  smallBonus?: number;
  largeBonus?: number;
  ignoreMiss?: number;
  ignoreHit?: number;
  comboBreak?: number;
  sliderTailHit?: number;
  legacyComboIncrease?: number;
}

export interface DifficultyOptions {
  mods?: string | number;
  ruleset?: Ruleset | null;
  clockRate?: number | null;
}

/** Difficulty calculator on maps of any mode. */
export declare class Difficulty {
  constructor(options?: DifficultyOptions);

  mods: string | number;
  ruleset: Ruleset | null;
  clockRate: number | null;

  /** Perform the difficulty calculation. */
  calculate(beatmap: Beatmap): DifficultyAttributes;
  calculateAsync(beatmap: Beatmap): Promise<DifficultyAttributes>;

  /**
   * Calculates the difficulty of the beatmap using a specific mod combination and returns a set of
   * TimedDifficultyAttributes representing the difficulty at every relevant time value in the beatmap.
   */
  calculateTimed(beatmap: Beatmap): TimedDifficultyAttributes[];
  calculateTimedAsync(beatmap: Beatmap): Promise<TimedDifficultyAttributes[]>;

  /**
   * Perform the difficulty calculation but instead of evaluating the skill strains, return them as
   * is.
   *
   * Suitable to plot the difficulty of a map over time.
   */
  strains(beatmap: Beatmap): Strains;
  strainsAsync(beatmap: Beatmap): Promise<Strains>;
}

export interface PerformanceOptions {
  mods?: string | number;
  maxCombo?: number | null;
  accuracy?: number | null;
  misses?: number | null;
  statistics?: Statistics | null;
  legacyTotalScore?: number | null;
}

/** Performance calculator on maps of any mode. */
export declare class Performance {
  constructor(options?: PerformanceOptions);

  mods: string | number;
  maxCombo: number | null;
  accuracy: number | null;
  misses: number | null;
  statistics: Statistics | null;
  legacyTotalScore: number | null;

  /** Perform the performance calculation for the map's or the attributes' mode. */
  calculate(beatmap: Beatmap, attributes?: DifficultyAttributes | null): PerformanceAttributes;
  calculateAsync(beatmap: Beatmap, attributes?: DifficultyAttributes | null): Promise<PerformanceAttributes>;
}
