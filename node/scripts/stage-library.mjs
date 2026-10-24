import { copyFileSync, mkdirSync, readdirSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

import { libraryName } from "../library.js";

const nodeDir = join(dirname(fileURLToPath(import.meta.url)), "..");

// The package loads `pppp_c` through Koffi, so the binary ships beside `index.js` under the name
// the loader looks for on this platform. The parser's engine goes with it, because `pppp_c` loads
// it from its own directory.
const from = process.argv[2];
if (!from) {
  console.error("usage: node scripts/stage-library.mjs <path to the built pppp_c>");
  process.exit(1);
}

mkdirSync(nodeDir, { recursive: true });

const to = join(nodeDir, libraryName());
copyFileSync(from, to);
console.log(`staged ${to}`);

const built = dirname(from);
for (const name of readdirSync(built)) {
  if (!name.includes("fosu_engine_")) {
    continue;
  }
  copyFileSync(join(built, name), join(nodeDir, name));
  console.log(`staged ${join(nodeDir, name)}`);
}
