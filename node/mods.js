/** Parsing a mod specification into the opaque `pppp_mods` handle the calculators take. */

import { PPPP_OK } from "./constants.js";
import { fn } from "./library.js";

function invalidMods() {
  const error = new Error("invalid mod specification");
  error.code = "ERR_PPPP_MODS";

  return error;
}

/** Reject a value that is not a ruleset id. */
function requireRuleset(ruleset) {
  if (!Number.isInteger(ruleset) || ruleset < 0 || ruleset > 3) {
    const error = new RangeError("ruleset must be one of 0, 1, 2, 3");
    error.code = "ERR_OUT_OF_RANGE";
    throw error;
  }

  return ruleset;
}

/** Parse a mod specification into a `pppp_mods`, or `null` for no mods. Throws on a bad input. */
function parse(spec, ruleset, classic) {
  let mods = null;
  if (spec !== null && spec !== undefined) {
    const slot = [null];

    if (typeof spec === "number") {
      if (!Number.isInteger(spec) || spec < 0 || spec > 0xffffffff) {
        throw invalidMods();
      }

      if (fn.pppp_mods_from_legacy(spec, ruleset, slot) !== PPPP_OK) {
        throw invalidMods();
      }
    } else if (typeof spec === "string") {
      if (fn.pppp_mods_parse(spec, slot) !== PPPP_OK) {
        throw invalidMods();
      }
    } else {
      throw new TypeError("mods must be a string or a number");
    }

    mods = slot[0];
  }

  if (classic) {
    if (mods === null) {
      const slot = [null];
      if (fn.pppp_mods_from_legacy(0, ruleset, slot) !== PPPP_OK) {
        throw invalidMods();
      }
      mods = slot[0];
    }
    if (fn.pppp_mods_add_classic(mods) !== PPPP_OK) {
      fn.pppp_mods_free(mods);
      throw invalidMods();
    }
  }
  return mods;
}

/** Run `body` with a `pppp_mods` and free it afterwards. */
function withMods(spec, ruleset, classic, body) {
  const mods = parse(spec, ruleset, classic);
  try {
    return body(mods);
  } finally {
    if (mods !== null) {
      fn.pppp_mods_free(mods);
    }
  }
}

export { parse, requireRuleset, withMods };
