import {
  Beatmap,
  type BeatmapDifficulty,
  type BreakPeriod,
  Difficulty,
  type DifficultyAttributes,
  type DifficultyOptions,
  type HitObject,
  Performance,
  type PerformanceAttributes,
  type PerformanceOptions,
  Ruleset,
  type Slider,
  type Statistics,
  type Strains,
  type TimedDifficultyAttributes,
  type TimingPoint,
  type Vector2,
} from "../module";

async function main(): Promise<void> {
  const beatmap = await Beatmap.fromFile("map.osu");
  const sync: Beatmap = Beatmap.fromFileSync("map.osu");
  const fromBytes: Beatmap = Beatmap.fromBytes(new Uint8Array(0));

  const settings: BeatmapDifficulty = beatmap.difficulty;
  console.log(settings.drainRate, settings.sliderTickRate);

  const object: HitObject = beatmap.hitObjects[0];
  const position: Vector2 = object.position;
  console.log(position.x, position.y, object.startTime, object.newCombo);

  const slider: Slider = beatmap.sliders[0];
  console.log(slider.expectedLength, slider.path.length, slider.cumulativeLengths.length);
  console.log(slider.events[0].spanIndex, slider.events[0].position.x);

  const point: TimingPoint = beatmap.timingPoints[0];
  const pause: BreakPeriod = beatmap.breaks[0];
  console.log(point.beatLength, point.uninherited, pause.endTime);

  const options: DifficultyOptions = { mods: "HD,DT", ruleset: Ruleset.Osu, clockRate: 1.5 };
  const difficulty: DifficultyAttributes = new Difficulty(options).calculate(beatmap);
  if (difficulty.ruleset === Ruleset.Osu) {
    console.log(difficulty.aimDifficulty, difficulty.sliderCount, difficulty.starRating);
  }

  const statistics: Statistics = { great: 601, miss: 2 };
  const playOptions: PerformanceOptions = { mods: 72, maxCombo: 909, statistics };
  const play: PerformanceAttributes = new Performance(playOptions).calculate(sync, difficulty);
  console.log(play.total);

  console.log(beatmap.formatVersion, beatmap.mode, beatmap.stackLeniency);
  console.log(beatmap.timingPoints.length, beatmap.hitObjects.length + beatmap.breaks.length, fromBytes.hitObjects.length);
  for (const sl of beatmap.sliders) {
    console.log(sl.undecimatedPath.length);
  }
  beatmap.close();

  const timed: TimedDifficultyAttributes[] = new Difficulty({ mods: 64 }).calculateTimed(beatmap);
  console.log(timed[0].time, timed[0].attributes.starRating);

  const later: DifficultyAttributes = await new Difficulty({ mods: "DT" }).calculateAsync(beatmap);
  const pp: number = (await new Performance().calculateAsync(beatmap, later)).total;
  const timedLater: TimedDifficultyAttributes[] = await new Difficulty().calculateTimedAsync(beatmap);
  console.log(pp, timedLater.length);

  const strains: Strains = new Difficulty().calculateStrains(beatmap);
  const strainsLater: Strains = await new Difficulty().calculateStrainsAsync(beatmap);
  for (const graph of [strains, strainsLater]) {
    if (graph.ruleset === Ruleset.Taiko) {
      console.log(graph.sectionLength, graph.singleColourStamina.length);
    }
  }
}

void main();
