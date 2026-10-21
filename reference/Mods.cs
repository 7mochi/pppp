using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text.Json;
using System.Text.Json.Nodes;
using osu.Game.Configuration;
using osu.Game.Extensions;
using osu.Game.Rulesets;
using osu.Game.Rulesets.Catch;
using osu.Game.Rulesets.Mania;
using osu.Game.Rulesets.Mods;
using osu.Game.Rulesets.Osu;
using osu.Game.Rulesets.Taiko;
using osu.Framework.Bindables;

internal static class Mods
{
    internal static void Run()
    {
        Ruleset[] rulesets = { new OsuRuleset(), new TaikoRuleset(), new CatchRuleset(), new ManiaRuleset() };
        string[] rulesetNames = { "osu", "taiko", "catch", "mania" };

        var instances = new List<Mod>[rulesets.Length];
        for (int i = 0; i < rulesets.Length; i++)
            instances[i] = flatten(rulesets[i].AllMods.Select(m => m.CreateInstance())).ToList();

        var acronyms = new SortedSet<string>(StringComparer.Ordinal);
        foreach (List<Mod> list in instances)
            foreach (Mod mod in list)
                acronyms.Add(mod.Acronym);

        var result = new JsonArray();

        foreach (string acronym in acronyms)
        {
            var settingsByKey = new SortedDictionary<string, JsonObject>(StringComparer.Ordinal);
            var membership = new List<string>();
            var incompatible = new SortedSet<string>(StringComparer.Ordinal);
            var representative = new Mod[rulesets.Length];

            for (int i = 0; i < rulesets.Length; i++)
            {
                List<Mod> here = instances[i].Where(m => m.Acronym == acronym).ToList();
                if (here.Count == 0)
                    continue;

                membership.Add(rulesetNames[i]);
                representative[i] = here[0];

                foreach (Mod mod in here)
                {
                    foreach ((SettingSourceAttribute _, PropertyInfo property) in mod.GetSettingsSourceProperties())
                    {
                        object value = property.GetValue(mod);
                        JsonObject row = settingRow(property.Name.ToSnakeCase(), value);
                        if (row is not null)
                            settingsByKey[property.Name.ToSnakeCase()] = row;
                    }

                    foreach (Mod other in instances[i])
                        if (other.Acronym != mod.Acronym && mod.IncompatibleMods.Any(t => t.IsInstanceOfType(other)))
                            incompatible.Add(other.Acronym);
                }
            }

            var legacy = new JsonArray();
            for (int i = 0; i < rulesets.Length; i++)
            {
                int bits = representative[i] is null
                    ? 0
                    : (int)rulesets[i].ConvertToLegacyMods(new Mod[] { representative[i] });
                legacy.Add(bits);
            }

            Mod first = representative.Where(m => m is not null).First();

            var settings = new JsonArray();
            foreach (JsonObject row in settingsByKey.Values)
                settings.Add(row);

            var membershipArray = new JsonArray();
            foreach (string name in membership)
                membershipArray.Add(name);

            var incompatibleArray = new JsonArray();
            foreach (string name in incompatible)
                incompatibleArray.Add(name);

            result.Add(new JsonObject
            {
                ["acronym"] = acronym,
                ["name"] = first.Name,
                ["type"] = first.Type.ToString(),
                ["rulesets"] = membershipArray,
                ["incompatible"] = incompatibleArray,
                ["settings"] = settings,
                ["legacy"] = legacy,
            });
        }

        Console.Out.Write(result.ToJsonString(new JsonSerializerOptions { WriteIndented = true }) + "\n");
    }

    private static JsonObject settingRow(string key, object value)
    {
        if (value is DifficultyBindable difficulty)
            return numberRow(key, difficulty.MinValue, difficulty.MaxValue, 0.0, difficulty.Value ?? 0.0);

        switch (value)
        {
            case IBindableNumber<double> n:
                return numberRow(key, n.MinValue, n.MaxValue, n.Precision, n.Value);

            case IBindableNumber<float> n:
                return numberRow(key, n.MinValue, n.MaxValue, n.Precision, n.Value);

            case IBindableNumber<int> n:
                return numberRow(key, n.MinValue, n.MaxValue, n.Precision, n.Value);

            case IBindable<bool>:
                return new JsonObject { ["key"] = key, ["type"] = "boolean" };

            case IBindable<string>:
                return new JsonObject { ["key"] = key, ["type"] = "string" };

            case IBindable<int?>:
            case IBindable<long?>:
                return new JsonObject { ["key"] = key, ["type"] = "integer" };

            case IBindable<float?>:
            case IBindable<double?>:
                return new JsonObject { ["key"] = key, ["type"] = "number" };

            case IBindable:
                return new JsonObject { ["key"] = key, ["type"] = "unsupported" };

            default:
                return null;
        }
    }

    private static JsonObject numberRow(string key, double min, double max, double precision, double value)
    {
        return new JsonObject
        {
            ["key"] = key,
            ["type"] = "number",
            ["min"] = min,
            ["max"] = max,
            ["precision"] = precision,
            ["default"] = value,
        };
    }

    private static IEnumerable<Mod> flatten(IEnumerable<Mod> mods)
    {
        foreach (Mod mod in mods)
        {
            if (mod is MultiMod multi)
            {
                foreach (Mod inner in flatten(multi.Mods))
                    yield return inner;
            }
            else
            {
                yield return mod;
            }
        }
    }
}
