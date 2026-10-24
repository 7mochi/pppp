/** The difficulty calculator and the conversion of its output. */

import { decode } from "koffi";

import { asyncCall } from "./async.js";
import { fromNative } from "./attributes.js";
import { Beatmap } from "./beatmap.js";
import { PPPP_DIFFICULTY_OPTIONS_VERSION } from "./constants.js";
import { check } from "./errors.js";
import { fn } from "./library.js";
import { parse, requireRuleset, withMods } from "./mods.js";
import { types } from "./native.g.js";
import { strains } from "./strains.js";

/** The Koffi `pppp_difficulty_options` for a call. */
function difficultyOptions(mods, ruleset, clockRate) {
  const hasRuleset = ruleset !== null && ruleset !== undefined;
  const hasClockRate = clockRate !== null && clockRate !== undefined;

  return {
    version: PPPP_DIFFICULTY_OPTIONS_VERSION,
    mods,
    ruleset: hasRuleset ? ruleset : 0,
    has_ruleset: hasRuleset ? 1 : 0,
    clock_rate: hasClockRate ? clockRate : 0,
    has_clock_rate: hasClockRate ? 1 : 0,
  };
}

/** Difficulty calculator on maps of any mode. */
class Difficulty {
  constructor({ mods = "", ruleset = null, clockRate = null, classic = false } = {}) {
    this.mods = mods;
    this.ruleset = ruleset;
    this.clockRate = clockRate;
    this.classic = classic;
  }

  /** The ruleset the calculation will use, validating a caller-supplied one. */
  _ruleset(beatmap) {
    return this.ruleset === null || this.ruleset === undefined
      ? beatmap.mode
      : requireRuleset(this.ruleset);
  }

  /** Perform the difficulty calculation. */
  calculate(beatmap) {
    const handle = Beatmap.require(beatmap);
    const ruleset = this._ruleset(beatmap);
    return withMods(this.mods, ruleset, this.classic, (mods) => {
      const out = [null];
      check(fn.pppp_calculate_difficulty(handle, difficultyOptions(mods, this.ruleset, this.clockRate), out));
      return fromNative(out[0]);
    });
  }

  /**
   * Perform the difficulty calculation without blocking the event loop.
   *
   * The arguments are validated and the mods are parsed before the call is queued, so a bad
   * specification throws here rather than rejecting the promise.
   */
  calculateAsync(beatmap) {
    const handle = Beatmap.require(beatmap);
    const ruleset = this._ruleset(beatmap);
    const mods = parse(this.mods, ruleset, this.classic);
    const options = difficultyOptions(mods, this.ruleset, this.clockRate);

    return asyncCall(fn.pppp_calculate_difficulty, handle, options, [null], mods, fromNative);
  }

  /**
   * Calculates the difficulty of the beatmap using a specific mod combination and returns a set of
   * TimedDifficultyAttributes representing the difficulty at every relevant time value in the beatmap.
   */
  calculateTimed(beatmap) {
    const handle = Beatmap.require(beatmap);
    const ruleset = this._ruleset(beatmap);

    return withMods(this.mods, ruleset, this.classic, (mods) => {
      const timed = [null];
      check(fn.pppp_calculate_timed_difficulty(handle, difficultyOptions(mods, this.ruleset, this.clockRate), timed));
      try {
        return entries(timed[0]);
      } finally {
        fn.pppp_timed_difficulty_free(timed[0]);
      }
    });
  }

  /** The asynchronous form of `calculateTimed`. */
  calculateTimedAsync(beatmap) {
    const handle = Beatmap.require(beatmap);
    const ruleset = this._ruleset(beatmap);
    const mods = parse(this.mods, ruleset, this.classic);
    const options = difficultyOptions(mods, this.ruleset, this.clockRate);

    return asyncCall(
      fn.pppp_calculate_timed_difficulty,
      handle,
      options,
      [null],
      mods,
      entries,
      (slot) => fn.pppp_timed_difficulty_free(slot),
    );
  }

  /**
   * Perform the difficulty calculation but instead of evaluating the skill strains, return them as
   * is.
   *
   * Suitable to plot the difficulty of a map over time.
   */
  calculateStrains(beatmap) {
    const handle = Beatmap.require(beatmap);
    const ruleset = this._ruleset(beatmap);

    return withMods(this.mods, ruleset, this.classic, (mods) => {
      const out = [null];
      check(fn.pppp_calculate_strains(handle, difficultyOptions(mods, this.ruleset, this.clockRate), out));
      try {
        return strains(out[0]);
      } finally {
        fn.pppp_strains_free(out[0]);
      }
    });
  }

  /** The asynchronous form of `calculateStrains`. */
  calculateStrainsAsync(beatmap) {
    const handle = Beatmap.require(beatmap);
    const ruleset = this._ruleset(beatmap);
    const mods = parse(this.mods, ruleset, this.classic);
    const options = difficultyOptions(mods, this.ruleset, this.clockRate);

    return asyncCall(
      fn.pppp_calculate_strains,
      handle,
      options,
      [null],
      mods,
      strains,
      (slot) => fn.pppp_strains_free(slot),
    );
  }
}

/** Read the timed difficulty entries out of an open `pppp_timed_difficulty` handle. */
function entries(handle) {
  const points = [null];
  const count = [0];
  check(fn.pppp_timed_difficulty_entries(handle, points, count));

  if (points[0] === null || count[0] === 0) {
    return [];
  }

  return decode(points[0], types.pppp_timed_difficulty_attributes, Number(count[0]))
    .map((entry) => ({ time: entry.time, attributes: fromNative(entry.attributes) }));
}

export { Difficulty };
