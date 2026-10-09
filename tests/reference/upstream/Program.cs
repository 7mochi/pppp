using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Threading;
using Newtonsoft.Json;
using osu.Game.Beatmaps;
using osu.Game.Beatmaps.Formats;
using osu.Game.Database;
using osu.Game.Extensions;
using osu.Game.IO;
using osu.Game.Online.API;
using osu.Game.Rulesets;
using osu.Game.Rulesets.Catch;
using osu.Game.Rulesets.Difficulty;
using osu.Game.Rulesets.Mania;
using osu.Game.Rulesets.Mods;
using osu.Game.Rulesets.Osu;
using osu.Game.Rulesets.Osu.Difficulty;
using osu.Game.Rulesets.Scoring;
using osu.Game.Rulesets.Taiko;
using osu.Game.Scoring;
using osu.Game.Scoring.Legacy;
using Decoder = osu.Game.Beatmaps.Formats.Decoder;

CultureInfo.CurrentCulture = CultureInfo.InvariantCulture;

_ = new OsuRuleset();
_ = new TaikoRuleset();
_ = new CatchRuleset();
_ = new ManiaRuleset();
var rulesets = new AssemblyRulesetStore();
Decoder.RegisterDependencies(rulesets);

string rowsPath = args[0];
string resourcesRoot = args.Length > 1 ? args[1] : "";

string output = args.Length > 2 ? args[2] : null;
JsonArray rows = JsonNode.Parse(File.ReadAllText(rowsPath))!.AsArray();

foreach (JsonNode node in rows)
{
    JsonObject row = node.AsObject();
    int mode = row["ruleset"]!.GetValue<int>();
    Ruleset ruleset = rulesetFor(mode, rulesets);

    using var stream = File.OpenRead(Path.Combine(resourcesRoot, row["map"]!.GetValue<string>()));
    using var reader = new LineBufferedReader(stream);
    Beatmap map = Decoder.GetDecoder<Beatmap>(reader).Decode(reader);
    var working = new FlatWorkingBeatmap(map);

    row["decode"] = decodeFields(map);

    Mod[] mods = parseMods(row["mods"], ruleset);

    using var cts = new CancellationTokenSource();
    DifficultyAttributes difficulty = ruleset.CreateDifficultyCalculator(working).Calculate(mods, cts.Token);
    row["difficulty"] = attributes(difficulty);

    if (row["score"] is JsonObject score)
    {
        ScoreInfo info = buildScore(score, ruleset, working, mods, difficulty, mode);
        row["performance"] = attributes(ruleset.CreatePerformanceCalculator()!.Calculate(info, difficulty));
    }
}

string json = rows.ToJsonString(new JsonSerializerOptions { WriteIndented = true }) + "\n";
if (output is null)
    Console.Out.Write(json);
else
    File.WriteAllText(output, json);

static Ruleset rulesetFor(int mode, AssemblyRulesetStore rulesets)
{
    RulesetInfo info = rulesets.GetRuleset(mode);
    if (info is null || !info.Available)
        throw new ArgumentException("no ruleset for mode " + mode);

    return info.CreateInstance();
}

static Mod[] parseMods(JsonNode modsNode, Ruleset ruleset)
{
    var parsed = new List<Mod>();
    foreach (JsonNode element in modsNode.AsArray())
    {
        string acronym = element is JsonObject asObject ? asObject["acronym"]!.GetValue<string>()
                                                        : element.GetValue<string>();
        if (string.IsNullOrWhiteSpace(acronym) || acronym == "NM")
            continue;

        APIMod apiMod = element is JsonObject
            ? JsonConvert.DeserializeObject<APIMod>(element.ToJsonString())!
            : new APIMod { Acronym = acronym.Trim() };

        Mod mod = apiMod.ToMod(ruleset);
        if (mod is UnknownMod)
            throw new ArgumentException("unknown mod " + apiMod.Acronym);

        parsed.Add(mod);
    }

    return parsed.ToArray();
}

static JsonObject decodeFields(Beatmap map)
{
    return new JsonObject
    {
        ["format"] = map.BeatmapVersion,
        ["mode"] = map.BeatmapInfo.Ruleset.OnlineID,
        ["ar"] = (double)map.Difficulty.ApproachRate,
        ["od"] = (double)map.Difficulty.OverallDifficulty,
        ["cs"] = (double)map.Difficulty.CircleSize,
        ["hp"] = (double)map.Difficulty.DrainRate,
        ["slider_multiplier"] = map.Difficulty.SliderMultiplier,
        ["slider_tick_rate"] = map.Difficulty.SliderTickRate,
        ["hit_objects"] = map.HitObjects.Count,
        ["uninherited_points"] = map.ControlPointInfo.TimingPoints.Count,
        ["breaks"] = map.Breaks.Count,
        ["stack_leniency"] = (double)map.StackLeniency,
    };
}

static JsonObject attributes(object value)
{
    var result = new JsonObject();
    foreach (PropertyInfo property in value.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance))
    {
        object field = property.GetValue(value);
        string name = property.Name.ToSnakeCase();

        if (field is double asDouble)
            result[name] = asDouble;
        else if (field is int asInt)
            result[name] = asInt;
        else if (field is long asLong)
            result[name] = asLong;
        else if (property.PropertyType == typeof(double?) && field is null)
            result[name] = null;
    }

    return result;
}

static ScoreInfo buildScore(JsonObject input, Ruleset ruleset, FlatWorkingBeatmap working, Mod[] mods,
                            DifficultyAttributes difficulty, int mode)
{
    int count(string name) => input[name]?.GetValue<int>() ?? 0;

    var score = new ScoreInfo((BeatmapInfo)working.BeatmapInfo, ruleset.RulesetInfo)
    {
        Mods = mods,
        MaxCombo = input["max_combo"] is JsonNode combo ? combo.GetValue<int>()
                                                        : (mode == 3 ? 0 : difficulty.MaxCombo),
        Statistics =
        {
            [HitResult.Miss] = count("count_miss"),
            [HitResult.Meh] = count("count_meh"),
            [HitResult.Ok] = count("count_ok"),
            [HitResult.Good] = count("count_good"),
            [HitResult.Great] = count("count_great"),
            [HitResult.Perfect] = count("count_perfect"),
            [HitResult.SmallTickMiss] = count("count_small_tick_miss"),
            [HitResult.SmallTickHit] = count("count_small_tick_hit"),
            [HitResult.LargeTickMiss] = count("count_large_tick_miss"),
            [HitResult.LargeTickHit] = count("count_large_tick_hit"),
        },
    };

    if (input["accuracy"] is JsonNode accuracy)
        score.Accuracy = accuracy.GetValue<double>();
    else
    {
        LegacyScoreDecoder.PopulateMaximumStatistics(score, working);
        score.Accuracy = StandardisedScoreMigrationTools.ComputeAccuracy(score, ruleset.CreateScoreProcessor());
    }

    score.Statistics[HitResult.SliderTailHit] = input["count_slider_tail_hit"] is JsonNode tail
        ? tail.GetValue<int>()
        : (mode == 0 && difficulty is OsuDifficultyAttributes osu ? osu.SliderCount : 0);

    if (input["legacy_total_score"] is JsonNode legacy)
        score.LegacyTotalScore = legacy.GetValue<long>();

    return score;
}

