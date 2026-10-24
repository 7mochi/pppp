/** The frozen attribute sets a calculation returns, and their converters to and from the ABI structs. */

import { requireRuleset } from "./mods.js";
import { camelCase } from "./names.js";
import { Ruleset } from "./ruleset.js";

/**
 * The union member that holds each ruleset's attributes. The two ABI unions differ: difficulty has
 * osu and taiko, performance adds mania.
 */
const DIFFICULTY_MEMBER = {
  [Ruleset.Osu]: "osu",
  [Ruleset.Taiko]: "taiko",
};

const PERFORMANCE_MEMBER = {
  ...DIFFICULTY_MEMBER,
  [Ruleset.Mania]: "mania",
};

/** The fields of each ruleset's performance attributes, under the C ABI's names. */
const PERFORMANCE_FIELDS = {
  [Ruleset.Osu]: [
    "aim", "speed", "accuracy", "flashlight", "reading", "effective_miss_count",
    "combo_based_estimated_miss_count", "score_based_estimated_miss_count",
    "aim_estimated_slider_breaks", "speed_estimated_slider_breaks", "speed_deviation",
  ],
  [Ruleset.Taiko]: ["difficulty", "accuracy", "estimated_unstable_rate"],
  [Ruleset.Mania]: ["difficulty"],
};

/** The fields of each ruleset's difficulty attributes, under the C ABI's names. */
const DIFFICULTY_FIELDS = {
  [Ruleset.Osu]: [
    "aim_difficulty", "speed_difficulty", "reading_difficulty", "flashlight_difficulty",
    "slider_factor", "aim_difficult_strain_count", "speed_difficult_strain_count",
    "reading_difficult_note_count", "aim_difficult_slider_count", "aim_top_weighted_slider_factor",
    "speed_top_weighted_slider_factor", "speed_note_count", "hit_circle_count", "slider_count",
    "large_tick_count", "spinner_count", "nested_score_per_object", "legacy_score_base_multiplier",
    "maximum_legacy_combo_score",
  ],
  [Ruleset.Taiko]: [
    "mechanical_difficulty", "rhythm_difficulty", "reading_difficulty", "colour_difficulty",
    "stamina_difficulty", "mono_stamina_factor", "consistency_factor", "stamina_top_strains",
  ],
};

/**
 * The public form of an attribute struct: the shared fields, then the ones of the union member the
 * ruleset selects. A field whose `has_` flag is clear becomes `null`.
 */
function fromNative(native) {
  const out = { ruleset: native.ruleset };
  const isPerformance = "total" in native;

  if (isPerformance) {
    out.total = native.total;
  } else {
    out.starRating = native.star_rating;
    out.maxCombo = native.max_combo;
  }

  const member = (isPerformance ? PERFORMANCE_MEMBER : DIFFICULTY_MEMBER)[native.ruleset];
  if (member === undefined) {
    requireRuleset(native.ruleset);
    return out;
  }

  const fields = native.union === undefined ? undefined : native.union[member];
  if (fields === undefined) {
    return out;
  }

  // Koffi exposes a struct's fields as non-enumerable properties, so this reads them by the C ABI's
  // names instead of iterating the object.
  const wanted = isPerformance ? PERFORMANCE_FIELDS : DIFFICULTY_FIELDS;
  for (const field of wanted[native.ruleset] || []) {
    const flag = `has_${field}`;
    out[camelCase(field)] = flag in fields && !fields[flag] ? null : fields[field];
  }

  return out;
}

/**
 * The `pppp_difficulty_attributes` for the public attribute object `Performance` receives: the
 * shared fields and the union member the ruleset selects, in the C ABI's names.
 */
function toNative(attributes) {
  requireRuleset(attributes.ruleset);
  const member = DIFFICULTY_MEMBER[attributes.ruleset];
  const native = {
    ruleset: attributes.ruleset,
    star_rating: attributes.starRating,
    max_combo: attributes.maxCombo,
  };

  // Koffi takes a union as an object holding exactly one member, so a ruleset that has none
  // (catch, mania) leaves the union out entirely.
  if (member !== undefined) {
    native.union = { [member]: {} };
  }

  for (const field of DIFFICULTY_FIELDS[attributes.ruleset] || []) {
    const key = camelCase(field);
    if (!(key in attributes)) {
      continue;
    }

    const value = attributes[key];
    const target = native.union === undefined ? native : native.union[member];
    target[field] = value === null ? 0 : value;

    if (value === null) {
      target[`has_${field}`] = 0;
    }
  }

  return native;
}

export { fromNative, toNative };
