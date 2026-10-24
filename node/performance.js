/** The performance calculator and the conversion of its output. */

import { asyncCall } from "./async.js";
import { fromNative, toNative } from "./attributes.js";
import { Beatmap } from "./beatmap.js";
import { PPPP_HIT_RESULT_COUNT, PPPP_PERFORMANCE_OPTIONS_VERSION } from "./constants.js";
import { check } from "./errors.js";
import { fn } from "./library.js";
import { parse, requireRuleset, withMods } from "./mods.js";

/** The statistics keys of a score, in the order the ABI indexes them. */
const HIT_RESULT_KEYS = [
  "none",
  "miss",
  "meh",
  "ok",
  "good",
  "great",
  "perfect",
  "smallTickMiss",
  "smallTickHit",
  "largeTickMiss",
  "largeTickHit",
  "smallBonus",
  "largeBonus",
  "ignoreMiss",
  "ignoreHit",
  "comboBreak",
  "sliderTailHit",
  "legacyComboIncrease",
];

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

  /** The `pppp_score_info` for the fields the caller set, plus whether any was set. */
  _score() {
    const counts = new Array(PPPP_HIT_RESULT_COUNT).fill(0);
    let present = false;

    if (this.statistics !== null && this.statistics !== undefined) {
      if (typeof this.statistics !== "object") {
        const error = new TypeError("statistics must be an object of counts");
        error.code = "ERR_INVALID_ARG_TYPE";
        throw error;
      }

      for (const [key, value] of Object.entries(this.statistics)) {
        const index = HIT_RESULT_KEYS.indexOf(key);
        if (index < 0) {
          const error = new RangeError("statistics keys must be hit results");
          error.code = "ERR_OUT_OF_RANGE";
          throw error;
        }

        counts[index] = value;
      }

      present = true;
    }

    const score = {
      statistics: counts,
      maximum_statistics: new Array(PPPP_HIT_RESULT_COUNT).fill(0),
      max_combo: 0,
      accuracy: 0,
      has_legacy_total_score: 0,
      legacy_total_score: 0,
    };

    if (this.legacyTotalScore !== null && this.legacyTotalScore !== undefined) {
      score.legacy_total_score = this.legacyTotalScore;
      score.has_legacy_total_score = 1;
      present = true;
    }

    return { score, present };
  }

  /** The `pppp_performance_options` for a call, validating the score fields. */
  _options(mods, provided) {
    const { score, present } = this._score();
    const hasMaxCombo = this.maxCombo !== null && this.maxCombo !== undefined;
    const hasAccuracy = this.accuracy !== null && this.accuracy !== undefined;
    const hasMisses = this.misses !== null && this.misses !== undefined;

    return {
      version: PPPP_PERFORMANCE_OPTIONS_VERSION,
      mods,
      score,
      has_score: present ? 1 : 0,
      combo: hasMaxCombo ? this.maxCombo : 0,
      has_combo: hasMaxCombo ? 1 : 0,
      accuracy: hasAccuracy ? this.accuracy : 0,
      has_accuracy: hasAccuracy ? 1 : 0,
      misses: hasMisses ? this.misses : 0,
      has_misses: hasMisses ? 1 : 0,
      difficulty: provided,
      has_difficulty: provided === null ? 0 : 1,
    };
  }

  /** The ruleset the calculation will use, from the attributes when they are given. */
  _ruleset(beatmap, provided) {
    return provided === null ? beatmap.mode : requireRuleset(provided.ruleset);
  }

  /** Perform the performance calculation for the map's or the attributes' mode. */
  calculate(beatmap, provided = null) {
    const handle = Beatmap.require(beatmap);
    const source = provided === null || provided === undefined ? null : provided;
    const native = source === null ? null : toNative(source);
    const ruleset = this._ruleset(beatmap, native);

    return withMods(this.mods, ruleset, this.classic, (mods) => {
      const out = [null];
      check(fn.pppp_calculate_performance(handle, this._options(mods, native), out));
      return fromNative(out[0]);
    });
  }

  /** The asynchronous form of `calculate`. */
  calculateAsync(beatmap, provided = null) {
    const handle = Beatmap.require(beatmap);
    const source = provided === null || provided === undefined ? null : provided;
    const native = source === null ? null : toNative(source);
    const ruleset = this._ruleset(beatmap, native);
    const mods = parse(this.mods, ruleset, this.classic);
    const options = this._options(mods, native);

    return asyncCall(fn.pppp_calculate_performance, handle, options, [null], mods, fromNative);
  }
}

export { Performance };
