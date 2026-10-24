/** Locating and loading `pppp_c`, and the ABI version check. */

import { existsSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { load } from "koffi";

import { PPPP_C_ABI_VERSION } from "./constants.js";
import { declare } from "./native.g.js";

const HERE = dirname(fileURLToPath(import.meta.url));

const LIBRARY_NAMES = { win32: "pppp_c.dll", darwin: "pppp_c.dylib" };

function libraryName() {
  return LIBRARY_NAMES[process.platform] || "pppp_c.so";
}

/** The path of `pppp_c`, honouring `PPPP_LIBRARY`, the package directory and the build trees. */
function libraryPath() {
  const name = libraryName();
  const candidates = [];

  if (process.env.PPPP_LIBRARY) {
    candidates.push(resolve(process.env.PPPP_LIBRARY));
  }

  candidates.push(join(HERE, name));
  candidates.push(join(HERE, "native", name));
  candidates.push(join(HERE, "..", "build", name));
  candidates.push(join(HERE, "..", "build", "release", name));

  for (const candidate of candidates) {
    if (existsSync(candidate)) {
      return candidate;
    }
  }

  throw new Error(`cannot find ${name}; build pppp or set PPPP_LIBRARY to the shared library`);
}

const library = load(libraryPath());
const fn = declare(library);

if (fn.pppp_abi_version() !== PPPP_C_ABI_VERSION) {
  throw new Error("pppp_c has an incompatible C ABI");
}

export { fn, libraryName, libraryPath };
