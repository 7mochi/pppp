const fs = require("fs");

const binding = require("pkg-prebuilds")(
  __dirname,
  require("./binding-options")
);

/** Which ruleset a tagged attribute set belongs to. The values are the beatmap's `Mode`. */
const Ruleset = Object.freeze({
  Osu: 0,
  Taiko: 1,
  Catch: 2,
  Mania: 3,
});

/** A loaded beatmap. */
class Beatmap {
  constructor() {
    throw new TypeError("use Beatmap.fromFile, Beatmap.fromFileSync or Beatmap.fromBytes");
  }

  /** Parse a `Beatmap` by providing the content of a `.osu` file as a slice of bytes. */
  static fromBytes(data) {
    return Object.setPrototypeOf(binding.fromBytes(data), Beatmap.prototype);
  }

  /** Parse a `Beatmap` by providing a path to a `.osu` file. */
  static fromFileSync(path) {
    return Beatmap.fromBytes(fs.readFileSync(path));
  }

  /** Parse a `Beatmap` by providing a path to a `.osu` file. */
  static async fromFile(path) {
    return Beatmap.fromBytes(await fs.promises.readFile(path));
  }
}

/** Difficulty calculator on maps of any mode. */
class Difficulty {
  constructor({ mods = "", ruleset = null, clockRate = null, classic = false } = {}) {
    this.mods = mods;
    this.ruleset = ruleset;
    this.clockRate = clockRate;
    this.classic = classic;
  }

  /** Perform the difficulty calculation. */
  calculate(beatmap) {
    return binding.calculateDifficulty(beatmap, this.mods, this.ruleset, this.clockRate, this.classic);
  }

  calculateAsync(beatmap) {
    return binding.calculateDifficultyAsync(beatmap, this.mods, this.ruleset, this.clockRate, this.classic);
  }

  /**
   * Calculates the difficulty of the beatmap using a specific mod combination and returns a set of
   * TimedDifficultyAttributes representing the difficulty at every relevant time value in the beatmap.
   */
  calculateTimed(beatmap) {
    return binding.calculateTimedDifficulty(beatmap, this.mods, this.ruleset, this.clockRate, this.classic);
  }

  calculateTimedAsync(beatmap) {
    return binding.calculateTimedDifficultyAsync(beatmap, this.mods, this.ruleset, this.clockRate, this.classic);
  }

  /**
   * Perform the difficulty calculation but instead of evaluating the skill strains, return them as
   * is.
   *
   * Suitable to plot the difficulty of a map over time.
   */
  strains(beatmap) {
    return binding.calculateStrains(beatmap, this.mods, this.ruleset, this.clockRate, this.classic);
  }

  strainsAsync(beatmap) {
    return binding.calculateStrainsAsync(beatmap, this.mods, this.ruleset, this.clockRate, this.classic);
  }
}

/** Performance calculator on maps of any mode. */
class Performance {
  constructor({
    mods = "",
    maxCombo = null,
    accuracy = null,
    misses = null,
    statistics = null,
    legacyTotalScore = null,
    classic = false,
  } = {}) {
    this.mods = mods;
    this.maxCombo = maxCombo;
    this.accuracy = accuracy;
    this.misses = misses;
    this.statistics = statistics;
    this.legacyTotalScore = legacyTotalScore;
    this.classic = classic;
  }

  /** Perform the performance calculation for the map's or the attributes' mode. */
  calculate(beatmap, attributes = null) {
    return binding.calculatePerformance(
      beatmap,
      this.mods,
      this.maxCombo,
      this.accuracy,
      this.misses,
      this.statistics,
      this.legacyTotalScore,
      attributes,
      this.classic
    );
  }

  calculateAsync(beatmap, attributes = null) {
    return binding.calculatePerformanceAsync(
      beatmap,
      this.mods,
      this.maxCombo,
      this.accuracy,
      this.misses,
      this.statistics,
      this.legacyTotalScore,
      attributes,
      this.classic
    );
  }
}

module.exports.version = binding.version;
module.exports.Ruleset = Ruleset;
module.exports.Beatmap = Beatmap;
module.exports.Difficulty = Difficulty;
module.exports.Performance = Performance;
