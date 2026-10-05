const { execFileSync } = require("child_process");
const fs = require("fs");
const path = require("path");
const { pathToFileURL } = require("url");
const pppp = require("../index.js");

const RESOURCES = path.join(__dirname, "..", "..", "tests", "resources");

function beatmapOf(name) {
  return pppp.Beatmap.fromFileSync(path.join(RESOURCES, name));
}

test("the version is a version", () => {
  expect(pppp.version).toMatch(/^\d+\.\d+\.\d+$/);
});

test("fromFileSync reads the model", () => {
  const beatmap = beatmapOf("osu/diffcalc-test.osu");
  expect(beatmap).toBeInstanceOf(pppp.Beatmap);
  expect(beatmap.formatVersion).toBe(14);
  expect(beatmap.mode).toBe(0);
  expect(beatmap.stackLeniency).toBeCloseTo(0.3);
  expect(beatmap.difficulty.drainRate).toBe(5.0);
  expect(beatmap.difficulty.circleSize).toBe(4.0);
  expect(beatmap.difficulty.overallDifficulty).toBe(7.0);
  expect(beatmap.difficulty.approachRate).toBeCloseTo(8.3);
  expect(beatmap.difficulty.sliderMultiplier).toBe(1.6);
  expect(beatmap.difficulty.sliderTickRate).toBe(1.0);
  expect(beatmap.hitObjects).toHaveLength(124);
  expect(beatmap.sliders).toHaveLength(33);
  expect(beatmap.timingPoints).toHaveLength(3);
  expect(beatmap.breaks).toHaveLength(0);
});

test("fromFile resolves to the same beatmap", async () => {
  const file = path.join(RESOURCES, "osu/diffcalc-test.osu");
  const beatmap = await pppp.Beatmap.fromFile(file);
  expect(beatmap).toBeInstanceOf(pppp.Beatmap);
  expect(new pppp.Difficulty().calculate(beatmap)).toEqual(
    new pppp.Difficulty().calculate(pppp.Beatmap.fromFileSync(file))
  );
});

test("fromBytes matches the file", () => {
  const file = path.join(RESOURCES, "osu/diffcalc-test.osu");
  const bytes = fs.readFileSync(file);
  expect(new pppp.Difficulty().calculate(pppp.Beatmap.fromBytes(new Uint8Array(bytes)))).toEqual(
    new pppp.Difficulty().calculate(pppp.Beatmap.fromFileSync(file))
  );
});

test("a missing file is the fs error", async () => {
  const missing = path.join(RESOURCES, "osu", "does-not-exist.osu");
  expect(() => pppp.Beatmap.fromFileSync(missing)).toThrow(expect.objectContaining({ code: "ENOENT" }));
  await expect(pppp.Beatmap.fromFile(missing)).rejects.toMatchObject({ code: "ENOENT" });
});

test("fromBytes takes bytes only", () => {
  expect(() => pppp.Beatmap.fromBytes("osu file format v14")).toThrow(
    expect.objectContaining({ code: "ERR_INVALID_ARG_TYPE" })
  );
});

test("a beatmap is not constructed directly", () => {
  expect(() => new pppp.Beatmap()).toThrow(TypeError);
});

test("hit objects point at their sliders", () => {
  const beatmap = beatmapOf("osu/diffcalc-test.osu");
  const objectSliders = beatmap.hitObjects.filter((object) => object.slider >= 0);
  expect(objectSliders).toHaveLength(beatmap.sliders.length);
  for (const object of objectSliders) {
    expect(object.slider).toBeGreaterThanOrEqual(0);
    expect(object.slider).toBeLessThan(beatmap.sliders.length);
  }
});

test.each([
  ["osu/2785319.osu", 6.004027372552197, 909],
  ["taiko/1028484.osu", 2.9144215641617333, 289],
  ["fruits/2118524.osu", 3.2340182503279706, 730],
  ["mania/1638954.osu", 3.358304846842773, 956],
])("the difficulty of %s matches the pinned stars", (name, stars, combo) => {
  const attributes = new pppp.Difficulty().calculate(beatmapOf(name));
  expect(attributes.starRating).toBeCloseTo(stars, 9);
  expect(attributes.maxCombo).toBe(combo);
});

test("mods change the calculation", () => {
  const attributes = new pppp.Difficulty({ mods: "HR" }).calculate(beatmapOf("fruits/2118524.osu"));
  expect(attributes.starRating).toBeCloseTo(4.308291009137178, 9);
});

test("stable mod bits match their acronyms", () => {
  const beatmap = beatmapOf("fruits/2118524.osu");
  expect(new pppp.Difficulty({ mods: 16 }).calculate(beatmap)).toEqual(
    new pppp.Difficulty({ mods: "HR" }).calculate(beatmap)
  );
});

test("the ruleset can be forced", () => {
  const attributes = new pppp.Difficulty({ ruleset: pppp.Ruleset.Taiko }).calculate(
    beatmapOf("osu/2785319.osu")
  );
  expect(attributes.ruleset).toBe(pppp.Ruleset.Taiko);
  expect(attributes.starRating).toBeCloseTo(4.752572620626138, 9);
  expect(attributes.mechanicalDifficulty).toBeGreaterThan(0.0);
});

test("the attributes are the live ruleset only", () => {
  const attributes = new pppp.Difficulty().calculate(beatmapOf("taiko/1028484.osu"));
  expect(attributes.ruleset).toBe(pppp.Ruleset.Taiko);
  expect(attributes).not.toHaveProperty("osu");
  expect(attributes).not.toHaveProperty("aimDifficulty");
  expect(JSON.parse(JSON.stringify(attributes))).toEqual(attributes);
});

test("an unknown mod is an error with a code", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  expect(() => new pppp.Difficulty({ mods: "XX" }).calculate(beatmap)).toThrow(
    expect.objectContaining({ code: "ERR_PPPP_MODS", message: "invalid mod specification" })
  );
});

test("a beatmap is required", () => {
  expect(() => new pppp.Difficulty().calculate("not a beatmap")).toThrow(
    expect.objectContaining({ code: "ERR_INVALID_ARG_TYPE", message: "expected a Beatmap" })
  );
});

test("a ruleset outside the four is an error", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  expect(() => new pppp.Difficulty({ ruleset: 9 }).calculate(beatmap)).toThrow(
    expect.objectContaining({ code: "ERR_OUT_OF_RANGE", message: "ruleset must be one of 0, 1, 2, 3" })
  );
});

test("the performance of osu matches the pinned pp", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const difficulty = new pppp.Difficulty().calculate(beatmap);
  // The score carries the slider tails, as the library's own test does.
  const play = new pppp.Performance({
    maxCombo: 909,
    accuracy: 1.0,
    statistics: { great: 601, sliderTailHit: difficulty.sliderCount },
  }).calculate(beatmap);
  expect(play.ruleset).toBe(pppp.Ruleset.Osu);
  expect(play.total).toBeCloseTo(316.5901855625614, 9);
  expect(play.aim).toBeCloseTo(148.75278891878943, 9);
  expect(play.speed).toBeCloseTo(61.34653468094172, 9);
  expect(play.accuracy).toBeCloseTo(98.99847982709288, 9);
  expect(play.reading).toBeCloseTo(2.2291238201795176, 9);
  expect(play.scoreBasedEstimatedMissCount).toBeNull();
});

test("the performance of taiko matches the pinned pp", () => {
  const play = new pppp.Performance({ maxCombo: 289, accuracy: 1.0, statistics: { great: 289 } }).calculate(
    beatmapOf("taiko/1028484.osu")
  );
  expect(play.ruleset).toBe(pppp.Ruleset.Taiko);
  expect(play.total).toBeCloseTo(130.26636361095524, 9);
  expect(play.difficulty).toBeCloseTo(33.48488833057447, 9);
  expect(play.accuracy).toBeCloseTo(96.78147528038076, 9);
  expect(play.estimatedUnstableRate).toBeCloseTo(146.3238357972284, 9);
});

test("the performance of catch matches the pinned pp", () => {
  const beatmap = beatmapOf("fruits/2118524.osu");
  const difficulty = new pppp.Difficulty().calculate(beatmap);
  const play = new pppp.Performance({
    maxCombo: difficulty.maxCombo,
    accuracy: 1.0,
    statistics: { great: 728, largeTickHit: 2, smallTickHit: 263 },
  }).calculate(beatmap);
  expect(play.ruleset).toBe(pppp.Ruleset.Catch);
  expect(play.total).toBeCloseTo(112.72215339177879, 9);
});

test("the performance of mania matches the pinned pp", () => {
  const beatmap = beatmapOf("mania/1638954.osu");
  const difficulty = new pppp.Difficulty().calculate(beatmap);
  const play = new pppp.Performance({
    maxCombo: difficulty.maxCombo,
    accuracy: 1.0,
    statistics: { perfect: 715 },
  }).calculate(beatmap);
  expect(play.ruleset).toBe(pppp.Ruleset.Mania);
  expect(play.total).toBeCloseTo(108.92297471705167, 9);
});

test("the performance takes the combo and misses it is given", () => {
  const play = new pppp.Performance({ maxCombo: 909, accuracy: 1.0, misses: 0 }).calculate(
    beatmapOf("osu/2785319.osu")
  );
  expect(play.total).toBeGreaterThan(0.0);
});

test("a statistics key outside the hit results is an error", () => {
  expect(() =>
    new pppp.Performance({ statistics: { great: 1, notAResult: 2 } }).calculate(beatmapOf("osu/2785319.osu"))
  ).toThrow(expect.objectContaining({ code: "ERR_OUT_OF_RANGE" }));
});

test("the performance from attributes matches the map", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const difficulty = new pppp.Difficulty().calculate(beatmap);
  const performance = new pppp.Performance({
    maxCombo: 909,
    accuracy: 1.0,
    statistics: { great: 601, sliderTailHit: difficulty.sliderCount },
  });
  expect(performance.calculate(beatmap, difficulty)).toEqual(performance.calculate(beatmap));
});

test("the package loads as an ES module", () => {
  const entry = JSON.stringify(pathToFileURL(path.join(__dirname, "..", "index.mjs")).href);
  const main = JSON.stringify(pathToFileURL(path.join(__dirname, "..", "index.js")).href);
  const script =
    `import { Beatmap, Difficulty } from ${entry};` +
    `import pppp from ${main};` +
    `process.stdout.write(String(Beatmap === pppp.Beatmap && typeof Difficulty === "function"));`;
  const output = execFileSync(process.execPath, ["--input-type=module", "-e", script]);
  expect(output.toString()).toBe("true");
});

test.each([
  ["osu/diffcalc-test.osu", "NM", 0, 124, [18000, 5.727266936277771, 63], [103000, 6.524323005451468, 239]],
  ["osu/2785319.osu", "HD,DT", 0, 601, [61309.235290751734, 8.272126401328721, 433], [115486.23529075173, 8.898179651893287, 909]],
  ["taiko/diffcalc-test.osu", "DT", 1, 238, [21624, 4.397186455763557, 118], [53000, 4.455142137225538, 200]],
  ["fruits/diffcalc-test.osu", "DT", 2, 93, [14500, 4.27412411307246, 47], [45250, 5.152717389780087, 127]],
  ["mania/diffcalc-test.osu", "NM", 3, 137, [16250, 1.8228476946125378, 69], [30500, 2.3493769750220914, 242]],
  ["osu/2785319.osu", "4K", 3, 838, [61662, 2.5718820820666117, 530], [115486, 2.653795415351293, 1072]],
])("the timed attributes of %s %s match upstream", (name, mods, ruleset, count, middle, last) => {
  const timed = new pppp.Difficulty({ mods, ruleset }).calculateTimed(beatmapOf(name));
  expect(timed).toHaveLength(count);
  for (const [entry, [time, stars, combo]] of [[timed[Math.floor(count / 2)], middle], [timed[count - 1], last]]) {
    expect(entry.time).toBe(time);
    expect(entry.attributes.starRating).toBeCloseTo(stars, 9);
    expect(entry.attributes.maxCombo).toBe(combo);
    expect(entry.attributes.ruleset).toBe(ruleset);
  }
});

test.each([
  ["osu/diffcalc-test.osu", "HD,FL", 0, 800, 400, "flashlight", 254, 18, 41.02690817661656],
  ["osu/2785319.osu", "DT", 0, 2400, 600, "aimNoSliders", 189, 66, 517.1805782727978],
  ["taiko/diffcalc-test.osu", "DT", 1, 0, 600, "singleColourStamina", 89, 6, 4.473586847131765],
  ["fruits/diffcalc-test.osu", "DT", 2, 0, 1125, "movement", 41, 11, 0.35301054964344125],
  ["mania/diffcalc-test.osu", "NM", 3, 400, 400, "strain", 76, 70, 16.8859847732691],
  ["osu/2785319.osu", "4K", 3, 2800, 400, "strain", 282, 195, 15.725755965531633],
])("the strain graph of %s %s matches upstream", (name, mods, ruleset, startTime, sectionLength, series, count, index, value) => {
  const strains = new pppp.Difficulty({ mods, ruleset }).strains(beatmapOf(name));
  expect(strains.ruleset).toBe(ruleset);
  expect(strains.startTime).toBe(startTime);
  expect(strains.sectionLength).toBe(sectionLength);
  expect(strains[series]).toHaveLength(count);
  expect(strains[series][index]).toBeCloseTo(value, 9);
});

test("each calculation has an async variant with the same result", async () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const difficulty = new pppp.Difficulty({ mods: "HD,DT" });
  const performance = new pppp.Performance({ maxCombo: 909, misses: 2 });
  const pending = difficulty.calculateAsync(beatmap);
  expect(typeof pending.then).toBe("function");
  const attributes = await pending;
  expect(attributes).toEqual(difficulty.calculate(beatmap));
  expect(await difficulty.calculateTimedAsync(beatmap)).toEqual(difficulty.calculateTimed(beatmap));
  expect(await difficulty.strainsAsync(beatmap)).toEqual(difficulty.strains(beatmap));
  expect(await performance.calculateAsync(beatmap, attributes)).toEqual(performance.calculate(beatmap, attributes));
});

test("the async variants check their arguments before they start", () => {
  expect(() => new pppp.Difficulty({ mods: "XX" }).calculateAsync(beatmapOf("osu/2785319.osu"))).toThrow(
    expect.objectContaining({ code: "ERR_PPPP_MODS" })
  );
  expect(() => new pppp.Difficulty().strainsAsync({})).toThrow(expect.objectContaining({ code: "ERR_INVALID_ARG_TYPE" }));
});

test("several async calculations on one beatmap run at once", async () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const mods = ["NM", "HD", "DT", "HR", "EZ", "HT"];
  const results = await Promise.all(mods.map((m) => new pppp.Difficulty({ mods: m }).calculateAsync(beatmap)));
  results.forEach((result, i) => expect(result).toEqual(new pppp.Difficulty({ mods: mods[i] }).calculate(beatmap)));
});
