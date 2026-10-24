/** Which ruleset an attribute set belongs to. The values are the beatmap's `Mode`. */
export declare const Ruleset: {
  readonly Osu: 0;
  readonly Taiko: 1;
  readonly Catch: 2;
  readonly Mania: 3;
};
export type Ruleset = (typeof Ruleset)[keyof typeof Ruleset];

/** The resolved path of the native library this module loaded. */
export declare const libraryPath: string;

export declare const version: string;
export declare const backend: string;

/** A two-dimensional point. */
export interface Vector2 {
  x: number;
  y: number;
}

/** The top-level difficulty settings of a beatmap. */
export interface BeatmapDifficulty {
  drainRate: number;
  circleSize: number;
  overallDifficulty: number;
  approachRate: number;
  sliderMultiplier: number;
  sliderTickRate: number;
}

/** A point in time on a slider with a special meaning. */
export interface SliderEvent {
  type: number;
  time: number;
  spanIndex: number;
  spanStartTime: number;
  pathProgress: number;
  position: Vector2;
}

/** An object in a beatmap. */
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

/** A slider and the arrays that describe it. */
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

/** A timing point of a beatmap. */
export interface TimingPoint {
  time: number;
  beatLength: number;
  meter: number;
  uninherited: boolean;
  effects: number;
}

/** A break in a beatmap. */
export interface BreakPeriod {
  startTime: number;
  endTime: number;
}

/** The difficulty fields every ruleset shares. */
export interface DifficultyAttributesBase {
  starRating: number;
  maxCombo: number;
}

/** The performance fields every ruleset shares. */
export interface PerformanceAttributesBase {
  total: number;
}

/** Difficulty attributes for osu!standard mode. */
export interface OsuDifficultyAttributes extends DifficultyAttributesBase {
  ruleset: typeof Ruleset.Osu;
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

/** Difficulty attributes for osu!taiko mode. */
export interface TaikoDifficultyAttributes extends DifficultyAttributesBase {
  ruleset: typeof Ruleset.Taiko;
  mechanicalDifficulty: number;
  rhythmDifficulty: number;
  readingDifficulty: number;
  colourDifficulty: number;
  staminaDifficulty: number;
  monoStaminaFactor: number;
  consistencyFactor: number;
  staminaTopStrains: number;
}

/** Difficulty attributes for osu!catch mode. */
export interface CatchDifficultyAttributes extends DifficultyAttributesBase {
  ruleset: typeof Ruleset.Catch;}

/** Difficulty attributes for osu!mania mode. */
export interface ManiaDifficultyAttributes extends DifficultyAttributesBase {
  ruleset: typeof Ruleset.Mania;}

export type DifficultyAttributes =
  | OsuDifficultyAttributes
  | TaikoDifficultyAttributes
  | CatchDifficultyAttributes
  | ManiaDifficultyAttributes;

/** Difficulty attributes together with the time they apply to. */
export interface TimedDifficultyAttributes {
  time: number;
  attributes: DifficultyAttributes;
}

/** Performance attributes for osu!standard mode. */
export interface OsuPerformanceAttributes extends PerformanceAttributesBase {
  ruleset: typeof Ruleset.Osu;
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

/** Performance attributes for osu!taiko mode. */
export interface TaikoPerformanceAttributes extends PerformanceAttributesBase {
  ruleset: typeof Ruleset.Taiko;
  difficulty: number;
  accuracy: number;
  estimatedUnstableRate: number | null;
}

/** Performance attributes for osu!catch mode. */
export interface CatchPerformanceAttributes extends PerformanceAttributesBase {
  ruleset: typeof Ruleset.Catch;}

/** Performance attributes for osu!mania mode. */
export interface ManiaPerformanceAttributes extends PerformanceAttributesBase {
  ruleset: typeof Ruleset.Mania;
  difficulty: number;
}

export type PerformanceAttributes =
  | OsuPerformanceAttributes
  | TaikoPerformanceAttributes
  | CatchPerformanceAttributes
  | ManiaPerformanceAttributes;

/** The fields every ruleset's strain series shares. */
export interface StrainsBase {
  startTime: number;
  sectionLength: number;
}

/** The series of an osu! strain graph. */
export interface OsuStrains extends StrainsBase {
  ruleset: typeof Ruleset.Osu;
  aim: number[];
  aimNoSliders: number[];
  speed: number[];
  reading: number[];
  flashlight: number[];
}

/** The strain series of an osu!taiko map. */
export interface TaikoStrains extends StrainsBase {
  ruleset: typeof Ruleset.Taiko;
  colour: number[];
  reading: number[];
  rhythm: number[];
  stamina: number[];
  singleColourStamina: number[];
}

/** The strain series of an osu!catch map. */
export interface CatchStrains extends StrainsBase {
  ruleset: typeof Ruleset.Catch;
  movement: number[];
}

/** The strain series of an osu!mania map. */
export interface ManiaStrains extends StrainsBase {
  ruleset: typeof Ruleset.Mania;
  strain: number[];
}

export type Strains = OsuStrains | TaikoStrains | CatchStrains | ManiaStrains;


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

  /** Free the native handle. Safe to call more than once. */
  close(): void;
}

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
  classic?: boolean;
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
  calculateStrains(beatmap: Beatmap): Strains;
  calculateStrainsAsync(beatmap: Beatmap): Promise<Strains>;
}

export interface PerformanceOptions {
  mods?: string | number;
  maxCombo?: number | null;
  accuracy?: number | null;
  misses?: number | null;
  statistics?: Statistics | null;
  legacyTotalScore?: number | null;
  classic?: boolean;
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
