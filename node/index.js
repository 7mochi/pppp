/** pppp: osu! difficulty and performance points, in native code. */

import { fn, libraryPath as resolveLibraryPath } from "./library.js";

export { Beatmap } from "./beatmap.js";
export { Difficulty } from "./difficulty.js";
export { Performance } from "./performance.js";
export { Ruleset } from "./ruleset.js";

/** The path of the `pppp_c` the module loaded. */
export const libraryPath = resolveLibraryPath();
export const version = fn.pppp_version();
export const backend = fn.pppp_backend();
