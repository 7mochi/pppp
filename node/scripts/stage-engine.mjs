import { copyFileSync, mkdirSync, readdirSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);

const arch = process.argv[2];
if (!arch) {
  console.error("usage: node scripts/stage-engine.mjs <arch>");
  process.exit(1);
}

const nodeDir = join(dirname(fileURLToPath(import.meta.url)), "..");
const { name } = require(join(nodeDir, "binding-options.js"));
const from = join(nodeDir, "build", "Release");
const to = join(nodeDir, "prebuilds", `${name}-${process.platform}-${arch}`);

mkdirSync(to, { recursive: true });

let copied = 0;
for (const file of readdirSync(from)) {
  if (!file.includes("fosu_engine_")) continue;
  copyFileSync(join(from, file), join(to, file));
  copied++;
}
if (copied === 0) {
  console.log(`no engine for ${process.platform}-${arch}; the binding will use scalar`);
}
