const path = require("path");
const pppp = require("../index.js");

const RESOURCES = path.join(__dirname, "..", "..", "tests", "resources");

function beatmapOf(name) {
  return pppp.fromFile(path.join(RESOURCES, name));
}

// The hit-result indices are the library's: `GREAT = 5` and `SLIDER_TAIL_HIT = 16`.
function scoreOf(counts, combo, accuracy = 1.0) {
  const statistics = new Array(18).fill(0);
  for (const index of Object.keys(counts)) {
    statistics[index] = counts[index];
  }
  return {
    statistics,
    maximum_statistics: new Array(18).fill(0),
    max_combo: combo,
    accuracy,
    legacy_total_score: null,
  };
}

test("the version is a version", () => {
  expect(pppp.version).toMatch(/^\d+\.\d+\.\d+$/);
});

test("fromFile reads the model", () => {
  const beatmap = beatmapOf("osu/diffcalc-test.osu");
  expect(beatmap.format_version).toBe(14);
  expect(beatmap.mode).toBe(0);
  expect(beatmap.stack_leniency).toBeCloseTo(0.3);
  expect(beatmap.difficulty.drain_rate).toBe(5.0);
  expect(beatmap.difficulty.circle_size).toBe(4.0);
  expect(beatmap.difficulty.overall_difficulty).toBe(7.0);
  expect(beatmap.difficulty.approach_rate).toBeCloseTo(8.3);
  expect(beatmap.difficulty.slider_multiplier).toBe(1.6);
  expect(beatmap.difficulty.slider_tick_rate).toBe(1.0);
  expect(beatmap.hit_objects).toHaveLength(124);
  expect(beatmap.sliders).toHaveLength(33);
  expect(beatmap.timing_points).toHaveLength(3);
  expect(beatmap.breaks).toHaveLength(0);
});

test("a missing file is an error", () => {
  expect(() => pppp.fromFile(path.join(RESOURCES, "osu", "does-not-exist.osu"))).toThrow();
});

test("hit objects point at their sliders", () => {
  const beatmap = beatmapOf("osu/diffcalc-test.osu");
  const objectSliders = beatmap.hit_objects.filter((object) => object.slider >= 0);
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
  const attributes = new pppp.Difficulty(beatmapOf(name)).calculate();
  expect(attributes.star_rating).toBeCloseTo(stars, 9);
  expect(attributes.max_combo).toBe(combo);
});

test("mods change the calculation", () => {
  const beatmap = beatmapOf("fruits/2118524.osu");
  const attributes = new pppp.Difficulty(beatmap).mods("HR").calculate();
  expect(attributes.star_rating).toBeCloseTo(4.308291009137178, 9);
});

test("the ruleset can be forced", () => {
  const attributes = new pppp.Difficulty(beatmapOf("osu/2785319.osu"))
    .ruleset(pppp.Ruleset.TAIKO)
    .calculate();
  expect(attributes.ruleset).toBe(pppp.Ruleset.TAIKO);
  expect(attributes.taiko.star_rating).toBeCloseTo(4.752572620626138, 9);
  expect(attributes.taiko.mechanical_difficulty).toBeGreaterThan(0.0);
});

test("a taiko map fills the taiko attributes", () => {
  const attributes = new pppp.Difficulty(beatmapOf("taiko/1028484.osu")).calculate();
  expect(attributes.ruleset).toBe(pppp.Ruleset.TAIKO);
  expect(attributes.osu.star_rating).toBe(0.0);
  expect(attributes.taiko.mechanical_difficulty).toBeGreaterThan(0.0);
});

test("an unknown mod is an error", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  expect(() => new pppp.Difficulty(beatmap).mods("XX").calculate()).toThrow(
    "invalid mod specification"
  );
});

test("a beatmap is required", () => {
  expect(() => pppp.calculateDifficulty("not a beatmap")).toThrow("expected a Beatmap");
});

test("a ruleset outside the four is an error", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  expect(() => new pppp.Difficulty(beatmap).ruleset(9).calculate()).toThrow(
    "ruleset must be one of 0, 1, 2, 3"
  );
});

test("the performance of osu matches the pinned pp", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const difficulty = new pppp.Difficulty(beatmap).calculate();
  // The score carries the slider tails, as the library's own test does.
  const play = new pppp.Performance(beatmap, {
    state: scoreOf({ 5: 601, 16: difficulty.osu.slider_count }, 909),
  }).calculate();
  expect(play.total).toBeCloseTo(316.5901855625614, 9);
  expect(play.osu.aim).toBeCloseTo(148.75278891878943, 9);
  expect(play.osu.speed).toBeCloseTo(61.34653468094172, 9);
  expect(play.osu.accuracy).toBeCloseTo(98.99847982709288, 9);
  expect(play.osu.reading).toBeCloseTo(2.2291238201795176, 9);
  expect(play.osu.score_based_estimated_miss_count).toBeNull();
});

test("the performance of taiko matches the pinned pp", () => {
  const beatmap = beatmapOf("taiko/1028484.osu");
  const play = new pppp.Performance(beatmap, { state: scoreOf({ 5: 289 }, 289) }).calculate();
  expect(play.ruleset).toBe(pppp.Ruleset.TAIKO);
  expect(play.total).toBeCloseTo(130.26636361095524, 9);
  expect(play.taiko.difficulty).toBeCloseTo(33.48488833057447, 9);
  expect(play.taiko.accuracy).toBeCloseTo(96.78147528038076, 9);
  expect(play.taiko.estimated_unstable_rate).toBeCloseTo(146.3238357972284, 9);
});

test("the performance of catch matches the pinned pp", () => {
  const beatmap = beatmapOf("fruits/2118524.osu");
  const difficulty = new pppp.Difficulty(beatmap).calculate();
  const play = new pppp.Performance(beatmap, {
    state: scoreOf({ 5: 728, 10: 2, 8: 263 }, difficulty.max_combo),
  }).calculate();
  expect(play.ruleset).toBe(pppp.Ruleset.CATCH);
  expect(play.total).toBeCloseTo(112.72215339177879, 9);
});

test("the performance of mania matches the pinned pp", () => {
  const beatmap = beatmapOf("mania/1638954.osu");
  const difficulty = new pppp.Difficulty(beatmap).calculate();
  const play = new pppp.Performance(beatmap, {
    state: scoreOf({ 6: 715 }, difficulty.max_combo),
  }).calculate();
  expect(play.ruleset).toBe(pppp.Ruleset.MANIA);
  expect(play.total).toBeCloseTo(108.92297471705167, 9);
});

test("the performance takes the combo and misses it is given", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const play = new pppp.Performance(beatmap, { combo: 909, accuracy: 1.0, misses: 0 }).calculate();
  expect(play.total).toBeGreaterThan(0.0);
});

test("the performance from attributes matches the map", () => {
  const beatmap = beatmapOf("osu/2785319.osu");
  const difficulty = new pppp.Difficulty(beatmap).calculate();
  const score = scoreOf({ 5: 601, 16: difficulty.osu.slider_count }, 909);
  const fromMap = new pppp.Performance(beatmap, { state: score }).calculate();
  const fromAttributes = new pppp.Performance(beatmap, {
    state: score,
    attributes: difficulty,
  }).calculate();
  expect(fromAttributes.total).toBe(fromMap.total);
  expect(fromAttributes.osu.aim).toBe(fromMap.osu.aim);
  expect(fromAttributes).toEqual(fromMap);
});
