import {
  Beatmap,
  Difficulty,
  type DifficultyAttributes,
  Performance,
  Ruleset,
  type Strains,
  type TimedDifficultyAttributes,
} from "../module";

async function main(): Promise<void> {
  const beatmap = await Beatmap.fromFile("map.osu");
  const sync: Beatmap = Beatmap.fromFileSync("map.osu");
  const fromBytes: Beatmap = Beatmap.fromBytes(new Uint8Array(0));

  const difficulty: DifficultyAttributes = new Difficulty({ mods: "HD,DT" }).calculate(beatmap);
  if (difficulty.ruleset === Ruleset.Osu) {
    console.log(difficulty.aimDifficulty, difficulty.sliderCount);
  }

  const play = new Performance({
    mods: 72,
    maxCombo: 909,
    statistics: { great: 601, miss: 2 },
  }).calculate(sync, difficulty);
  console.log(play.total, fromBytes.hitObjects.length, beatmap.stackLeniency);

  const timed: TimedDifficultyAttributes[] = new Difficulty({ mods: 64 }).calculateTimed(beatmap);
  console.log(timed[0].time, timed[0].attributes.starRating);

  const later: DifficultyAttributes = await new Difficulty({ mods: "DT" }).calculateAsync(beatmap);
  const pp: number = (await new Performance().calculateAsync(beatmap, later)).total;
  const timedLater: TimedDifficultyAttributes[] = await new Difficulty().calculateTimedAsync(beatmap);
  console.log(pp, timedLater.length);

  const strains: Strains = await new Difficulty().strainsAsync(beatmap);
  if (strains.ruleset === Ruleset.Taiko) {
    console.log(strains.sectionLength, strains.singleColourStamina.length);
  }
}

void main();
