import { Difficulty, Performance, ScoreInfo, fromFile } from "../module";

const beatmap = fromFile("map.osu");
const difficulty = new Difficulty(beatmap).mods("HD,DT").calculate();

const score: ScoreInfo = {
  statistics: [0, 0, 0, 0, 0, 601],
  maximum_statistics: [0, 0, 0, 0, 0, 601],
  max_combo: 909,
  accuracy: 1.0,
  legacy_total_score: null,
};

const byConstructor = new Performance(beatmap, { state: score, attributes: difficulty }).calculate();
const bySetter = new Performance(beatmap).attributes(difficulty).calculate();

console.log(byConstructor.total, bySetter.osu.aim);
