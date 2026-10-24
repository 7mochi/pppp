/** The strain series a difficulty calculation returns, per ruleset. */

import { decode } from "koffi";

import { check } from "./errors.js";
import { fn } from "./library.js";
import { camelCase } from "./names.js";
import { Ruleset } from "./ruleset.js";

/** The series in each ruleset's strain struct, under the C ABI's names. */
const STRAIN_FIELDS = {
  [Ruleset.Osu]: ["aim", "aim_no_sliders", "speed", "reading", "flashlight"],
  [Ruleset.Taiko]: ["colour", "reading", "rhythm", "stamina", "single_colour_stamina"],
  [Ruleset.Catch]: ["movement"],
  [Ruleset.Mania]: ["strain"],
};

/** Read the strain series out of an open `pppp_strains` handle. */
function strains(handle) {
  const nativeRuleset = [0];
  const startTime = [0];
  const sectionLength = [0];
  check(fn.pppp_strains_info(handle, nativeRuleset, startTime, sectionLength));
  const base = {
    ruleset: nativeRuleset[0],
    startTime: startTime[0],
    sectionLength: sectionLength[0],
  };
  const readers = {
    [Ruleset.Osu]: "pppp_strains_osu",
    [Ruleset.Taiko]: "pppp_strains_taiko",
    [Ruleset.Catch]: "pppp_strains_catch",
    [Ruleset.Mania]: "pppp_strains_mania",
  };
  const reader = readers[base.ruleset];
  if (reader === undefined) {
    throw new Error(`unknown ruleset ${base.ruleset}`);
  }
  const out = [null];
  check(fn[reader](handle, out));
  const series = out[0];
  const result = Object.assign({}, base);
  // Each series is a pointer and a count. Koffi returns the pointer, so the values are decoded here.
  for (const field of STRAIN_FIELDS[base.ruleset] || []) {
    const count = Number(series[`${field}_count`]);
    const values = series[field];
    result[camelCase(field)] =
      values === null || count === 0 ? [] : decode(values, "double", count);
  }
  return result;
}

export { strains };
