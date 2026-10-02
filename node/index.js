const binding = require("pkg-prebuilds")(
  __dirname,
  require("./binding-options")
);

// Copy exports so that we can customize them on the JS side without
// overwriting the binding itself.
Object.keys(binding).forEach(function (key) {
  module.exports[key] = binding[key];
});

const Ruleset = Object.freeze({
  /** The ruleset a tagged attribute set belongs to; the values are the beatmap's mode. */
  OSU: 0,
  TAIKO: 1,
  CATCH: 2,
  MANIA: 3,
});

/** Difficulty calculator on maps of any mode. */
class Difficulty {
  constructor(beatmap, options = {}) {
    this.beatmap = beatmap;
    this.modsSpec = options.mods ?? "";
    this.rulesetValue = options.ruleset ?? null;
    this.clockRateValue = options.clockRate ?? null;
  }

  /** Specify mods, as osu!'s own specification list. */
  mods(spec) {
    this.modsSpec = spec;
    return this;
  }

  /** Calculate for this ruleset instead of the beatmap's own mode. */
  ruleset(ruleset) {
    this.rulesetValue = ruleset;
    return this;
  }

  /** Adjust the clock rate used in the calculation. */
  clockRate(rate) {
    this.clockRateValue = rate;
    return this;
  }

  /** Perform the difficulty calculation. */
  calculate() {
    return binding.calculateDifficulty(
      this.beatmap,
      this.modsSpec,
      this.rulesetValue,
      this.clockRateValue
    );
  }
}

/** Performance calculator on maps of any mode. */
class Performance {
  constructor(beatmap, options = {}) {
    this.beatmap = beatmap;
    this.modsSpec = options.mods ?? "";
    this.scoreState = options.state ?? null;
    this.comboValue = options.combo ?? null;
    this.accuracyValue = options.accuracy ?? null;
    this.missesValue = options.misses ?? null;
  }

  /** Specify mods, as osu!'s own specification list. */
  mods(spec) {
    this.modsSpec = spec;
    return this;
  }

  /** Provide the score state through a `ScoreInfo`. */
  state(state) {
    this.scoreState = state;
    return this;
  }

  /** Specify the max combo of the play. */
  combo(combo) {
    this.comboValue = combo;
    return this;
  }

  /** Set the accuracy between 0.0 and 1.0. */
  accuracy(accuracy) {
    this.accuracyValue = accuracy;
    return this;
  }

  /** Specify the amount of misses of the play. */
  misses(misses) {
    this.missesValue = misses;
    return this;
  }

  /** Perform the performance calculation, difficulty included. */
  calculate() {
    return binding.calculatePerformance(
      this.beatmap,
      this.modsSpec,
      this.scoreState,
      this.comboValue,
      this.accuracyValue,
      this.missesValue
    );
  }
}

module.exports.Ruleset = Ruleset;
module.exports.Difficulty = Difficulty;
module.exports.Performance = Performance;
