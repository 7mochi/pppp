using System;
using System.IO;
using Pppp;

namespace Pppp.Tests {
    internal static class Program {
        private static int checks;
        private static int failures;
        private const double EPSILON = 1e-9;

        private static string resources;

        private static string Resources {
            get {
                if (resources == null) {
                    DirectoryInfo directory = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
                    while (directory != null) {
                        string candidate = Path.Combine(Path.Combine(directory.FullName, "tests"),
                                                        "resources");
                        if (Directory.Exists(candidate)) {
                            resources = candidate;
                            break;
                        }
                        directory = directory.Parent;
                    }
                    if (resources == null) {
                        throw new DirectoryNotFoundException("tests/resources");
                    }
                }
                return resources;
            }
        }

        private static string MapPath(string name) {
            return Path.Combine(Resources, name.Replace('/', Path.DirectorySeparatorChar));
        }

        private static void Check(bool condition, string what) {
            checks++;
            if (!condition) {
                failures++;
                Console.WriteLine("FAIL: " + what);
            }
        }

        private static void CheckClose(double actual, double expected, string what) {
            Check(Math.Abs(actual - expected) <= EPSILON, what + " (was " + actual + ")");
        }

        private static void CheckApprox(double actual, double expected, string what) {
            Check(Math.Abs(actual - expected) <= 1e-6 * Math.Abs(expected), what + " (was " + actual + ")");
        }

        private delegate void Throwing();

        private static void CheckThrows<T>(Throwing action, string what) where T : Exception {
            checks++;
            try {
                action();
            } catch (T) {
                return;
            } catch (Exception error) {
                failures++;
                Console.WriteLine("FAIL: " + what + " (threw " + error.GetType().Name + ")");
                return;
            }
            failures++;
            Console.WriteLine("FAIL: " + what + " (nothing was thrown)");
        }

        private static void TheVersionIsAVersion() {
            string version = Version.Current;
            Check(version != null && version.Split('.').Length == 3, "the version is a version");
        }

        private static void FromFileReadsTheModel() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/diffcalc-test.osu"))) {
                Check(beatmap.FormatVersion == 14, "FormatVersion");
                Check(beatmap.Mode == 0, "Mode");
                CheckApprox(beatmap.StackLeniency, 0.3, "StackLeniency");
                Check(beatmap.Difficulty.DrainRate == 5.0, "DrainRate");
                Check(beatmap.Difficulty.CircleSize == 4.0, "CircleSize");
                Check(beatmap.Difficulty.OverallDifficulty == 7.0, "OverallDifficulty");
                CheckApprox(beatmap.Difficulty.ApproachRate, 8.3, "ApproachRate");
                Check(beatmap.Difficulty.SliderMultiplier == 1.6, "SliderMultiplier");
                Check(beatmap.Difficulty.SliderTickRate == 1.0, "SliderTickRate");
                Check(beatmap.HitObjects.Length == 124, "HitObjects");
                Check(beatmap.Sliders.Length == 33, "Sliders");
                Check(beatmap.TimingPoints.Length == 3, "TimingPoints");
                Check(beatmap.Breaks.Length == 0, "Breaks");

                int onSliders = 0;
                for (int i = 0; i < beatmap.HitObjects.Length; i++) {
                    if (beatmap.HitObjects[i].Slider >= 0) {
                        onSliders++;
                        Check(beatmap.HitObjects[i].Slider < beatmap.Sliders.Length,
                              "a hit object's slider index is in range");
                    }
                }
                Check(onSliders == beatmap.Sliders.Length, "hit objects point at their sliders");

                Slider slider = beatmap.Sliders[0];
                Check(slider.Slides == 1, "the first slider's Slides");
                Check(slider.ExpectedLength == 160.0, "the first slider's ExpectedLength");
                Check(slider.ControlPoints.Length == 1, "the first slider's ControlPoints");
                Check(slider.Events.Length == 3, "the first slider's Events");
                Check(slider.Path.Length == slider.CumulativeLengths.Length,
                      "a slider's path matches its cumulative lengths");

                HitObject last = beatmap.HitObjects[beatmap.HitObjects.Length - 1];
                Check(last.Position.X == 256.0f && last.Position.Y == 192.0f, "a spinner is centred");
                Check(last.StartTime == 102125.0 && last.EndTime == 103000.0, "a spinner's times");
            }
        }

        private static void FromBytesMatchesTheFile() {
            string path = MapPath("osu/diffcalc-test.osu");
            using (Beatmap fromBytes = Beatmap.FromBytes(File.ReadAllBytes(path)))
            using (Beatmap fromFile = Beatmap.FromFile(path)) {
                CheckClose(new Difficulty().Calculate(fromBytes).StarRating,
                           new Difficulty().Calculate(fromFile).StarRating, "FromBytes matches FromFile");
            }
        }

        private static void AMissingFileIsTheFrameworkException() {
            CheckThrows<FileNotFoundException>(delegate { Beatmap.FromFile(MapPath("osu/does-not-exist.osu")); },
                                               "a missing file is a FileNotFoundException");
        }

        private static void DifficultyMatchesThePinnedStars() {
            string[] names = {"osu/2785319.osu", "taiko/1028484.osu", "fruits/2118524.osu",
                              "mania/1638954.osu"};
            double[] stars = {6.004027372552197, 2.9144215641617333, 3.2340182503279706,
                              3.358304846842773};
            int[] combos = {909, 289, 730, 956};

            for (int i = 0; i < names.Length; i++) {
                using (Beatmap beatmap = Beatmap.FromFile(MapPath(names[i]))) {
                    DifficultyAttributes attributes = new Difficulty().Calculate(beatmap);
                    CheckClose(attributes.StarRating, stars[i], names[i] + " star rating");
                    Check(attributes.MaxCombo == combos[i], names[i] + " max combo");
                }
            }
        }

        private static void ModsChangeTheCalculation() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes attributes = new Difficulty { Mods = "HR" }.Calculate(beatmap);
                CheckClose(attributes.StarRating, 4.308291009137178, "HR star rating");
            }
        }

        private static void StableModBitsMatchTheirAcronyms() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes legacy = new Difficulty { Mods = LegacyMods.HardRock }.Calculate(beatmap);
                CheckClose(legacy.StarRating, 4.308291009137178, "LegacyMods.HardRock star rating");
            }
        }

        private static void ClassicCanBeAddedToStableModBits() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                Mods stable = LegacyMods.None;
                PerformanceAttributes classic = new Performance {
                    Mods = stable.WithClassic(),
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                PerformanceAttributes twice = new Performance {
                    Mods = stable.WithClassic().WithClassic(),
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                PerformanceAttributes fromSpecification = new Performance {
                    Mods = "CL",
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                PerformanceAttributes plainBits = new Performance {
                    Mods = stable,
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);

                CheckClose(classic.Total, 298.5325815982227, "classic from stable bits");
                CheckClose(classic.Total, fromSpecification.Total, "classic from bits vs from the specification");
                CheckClose(twice.Total, classic.Total, "adding classic twice changes nothing");
                CheckClose(plainBits.Total, 316.5901855625614, "the same bits without classic stay on lazer");
            }
        }

        private static void TheRulesetCanBeForced() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                DifficultyAttributes attributes = new Difficulty { Ruleset = Ruleset.Taiko }.Calculate(beatmap);
                TaikoDifficultyAttributes taiko = attributes as TaikoDifficultyAttributes;
                Check(attributes.Ruleset == Ruleset.Taiko, "the forced ruleset");
                Check(taiko != null, "the forced ruleset's attribute class");
                CheckClose(attributes.StarRating, 4.752572620626138, "the taiko star rating");
                Check(taiko != null && taiko.MechanicalDifficulty > 0.0, "the taiko mechanical difficulty");
            }
        }

        private static void TheAttributesAreTheLiveRulesetOnly() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("taiko/1028484.osu"))) {
                DifficultyAttributes attributes = new Difficulty().Calculate(beatmap);
                Check(attributes is TaikoDifficultyAttributes, "a taiko map's attribute class");
                Check(!(attributes is OsuDifficultyAttributes), "no osu attributes on a taiko map");
            }
        }

        private static void AnUnknownModIsAnArgumentException() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                checks++;
                try {
                    new Difficulty { Mods = "XX" }.Calculate(beatmap);
                    failures++;
                    Console.WriteLine("FAIL: an unknown mod (nothing was thrown)");
                } catch (ArgumentException error) {
                    if (error.ParamName != "Mods" || !error.Message.StartsWith("invalid mod specification")) {
                        failures++;
                        Console.WriteLine("FAIL: an unknown mod (" + error.ParamName + ": " + error.Message + ")");
                    }
                }
            }
        }

        private static void ARulesetOutsideTheFourIsAnError() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                CheckThrows<ArgumentOutOfRangeException>(
                    delegate { new Difficulty { Ruleset = (Ruleset)9 }.Calculate(beatmap); },
                    "a ruleset outside the four");
            }
        }

        private static void ThePerformanceOfOsuMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                PerformanceAttributes play = new Performance {
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                OsuPerformanceAttributes osu = (OsuPerformanceAttributes)play;
                CheckClose(play.Total, 316.5901855625614, "the osu total");
                CheckClose(osu.Aim, 148.75278891878943, "the osu aim");
                CheckClose(osu.Speed, 61.34653468094172, "the osu speed");
                CheckClose(osu.Accuracy, 98.99847982709288, "the osu accuracy");
                CheckClose(osu.Reading, 2.2291238201795176, "the osu reading");
                Check(!osu.ScoreBasedEstimatedMissCount.HasValue, "the unset optional stays null");
            }
        }

        private static void ThePerformanceOfTaikoMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("taiko/1028484.osu"))) {
                PerformanceAttributes play = new Performance {
                    MaxCombo = 289,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 289}},
                }.Calculate(beatmap);
                TaikoPerformanceAttributes taiko = (TaikoPerformanceAttributes)play;
                Check(play.Ruleset == Ruleset.Taiko, "the taiko ruleset");
                CheckClose(play.Total, 130.26636361095524, "the taiko total");
                CheckClose(taiko.Difficulty, 33.48488833057447, "the taiko difficulty");
                CheckClose(taiko.Accuracy, 96.78147528038076, "the taiko accuracy");
                Check(taiko.EstimatedUnstableRate.HasValue, "the set optional has a value");
                CheckClose(taiko.EstimatedUnstableRate.GetValueOrDefault(), 146.3238357972284,
                           "the taiko unstable rate");
            }
        }

        private static void ThePerformanceOfCatchMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes difficulty = new Difficulty().Calculate(beatmap);
                PerformanceAttributes play = new Performance {
                    MaxCombo = difficulty.MaxCombo,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 728}, {HitResult.LargeTickHit, 2}, {HitResult.SmallTickHit, 263}},
                }.Calculate(beatmap);
                Check(play.Ruleset == Ruleset.Catch, "the catch ruleset");
                CheckClose(play.Total, 112.72215339177879, "the catch total");
            }
        }

        private static void ThePerformanceOfManiaMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("mania/1638954.osu"))) {
                DifficultyAttributes difficulty = new Difficulty().Calculate(beatmap);
                PerformanceAttributes play = new Performance {
                    MaxCombo = difficulty.MaxCombo,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Perfect, 715}},
                }.Calculate(beatmap);
                Check(play.Ruleset == Ruleset.Mania, "the mania ruleset");
                CheckClose(play.Total, 108.92297471705167, "the mania total");
            }
        }

        private static void ThePerformanceTakesTheComboAndMissesItIsGiven() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                PerformanceAttributes play =
                    new Performance { MaxCombo = 909, Accuracy = 1.0, Misses = 0 }.Calculate(beatmap);
                Check(play.Total > 0.0, "a play with an explicit combo");
            }
        }

        private static void ThePerformanceFromAttributesMatchesTheMap() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                Performance performance = new Performance {
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                };
                OsuPerformanceAttributes fromMap = (OsuPerformanceAttributes)performance.Calculate(beatmap);
                OsuPerformanceAttributes fromAttributes =
                    (OsuPerformanceAttributes)performance.Calculate(beatmap, difficulty);
                CheckClose(fromAttributes.Total, fromMap.Total, "the total from the attributes");
                CheckClose(fromAttributes.Aim, fromMap.Aim, "the aim from the attributes");
                CheckClose(fromAttributes.Speed, fromMap.Speed, "the speed from the attributes");
            }
        }

        private static void TheTimedAttributesMatchUpstream() {
            string[] names = {"osu/diffcalc-test.osu", "osu/2785319.osu", "taiko/diffcalc-test.osu",
                              "fruits/diffcalc-test.osu", "mania/diffcalc-test.osu", "osu/2785319.osu"};
            string[] mods = {"NM", "HD,DT", "DT", "DT", "NM", "4K"};
            Ruleset[] rulesets = {Ruleset.Osu, Ruleset.Osu, Ruleset.Taiko, Ruleset.Catch, Ruleset.Mania, Ruleset.Mania};
            int[] counts = {124, 601, 238, 93, 137, 838};
            double[] lastTimes = {103000, 115486.23529075173, 53000, 45250, 30500, 115486};
            double[] lastStars = {6.524323005451468, 8.898179651893287, 4.455142137225538, 5.152717389780087,
                                  2.3493769750220914, 2.653795415351293};
            int[] lastCombos = {239, 909, 200, 127, 242, 1072};

            for (int i = 0; i < names.Length; i++) {
                using (Beatmap beatmap = Beatmap.FromFile(MapPath(names[i]))) {
                    TimedDifficultyAttributes[] timed =
                        new Difficulty { Mods = mods[i], Ruleset = rulesets[i] }.CalculateTimed(beatmap);
                    Check(timed.Length == counts[i], names[i] + " " + mods[i] + " timed count");
                    TimedDifficultyAttributes last = timed[timed.Length - 1];
                    Check(last.Time == lastTimes[i], names[i] + " " + mods[i] + " last time");
                    CheckApprox(last.Attributes.StarRating, lastStars[i], names[i] + " " + mods[i] + " last stars");
                    Check(last.Attributes.MaxCombo == lastCombos[i], names[i] + " " + mods[i] + " last combo");
                    Check(last.Attributes.Ruleset == rulesets[i], names[i] + " " + mods[i] + " last ruleset");
                }
            }
        }

        private static void TheStrainGraphMatchesUpstream() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/diffcalc-test.osu"))) {
                OsuStrains osu = new Difficulty { Mods = "HD,FL" }.CalculateStrains(beatmap) as OsuStrains;
                Check(osu != null, "osu! strains");
                if (osu != null) {
                    Check(osu.Ruleset == Ruleset.Osu, "osu! strains ruleset");
                    Check(osu.StartTime == 800, "osu! strains start time");
                    Check(osu.SectionLength == 400, "osu! strains section length");
                    Check(osu.Aim.Length == 254 && osu.Flashlight.Length == 254, "osu! strains length");
                    CheckApprox(osu.Aim[46], 522.4551066294925, "osu! aim strain");
                    CheckApprox(osu.Flashlight[18], 41.02690817661656, "osu! flashlight strain");
                }
            }

            using (Beatmap beatmap = Beatmap.FromFile(MapPath("taiko/diffcalc-test.osu"))) {
                TaikoStrains taiko = new Difficulty { Mods = "DT" }.CalculateStrains(beatmap) as TaikoStrains;
                Check(taiko != null, "osu!taiko strains");
                if (taiko != null) {
                    Check(taiko.SectionLength == 600, "osu!taiko strains section length");
                    Check(taiko.SingleColourStamina.Length == 89, "osu!taiko strains length");
                    CheckApprox(taiko.SingleColourStamina[6], 4.473586847131765, "osu!taiko single colour stamina strain");
                }
            }

            using (Beatmap beatmap = Beatmap.FromFile(MapPath("fruits/diffcalc-test.osu"))) {
                CatchStrains fruits = new Difficulty { Mods = "DT" }.CalculateStrains(beatmap) as CatchStrains;
                Check(fruits != null, "osu!catch strains");
                if (fruits != null) {
                    Check(fruits.SectionLength == 1125, "osu!catch strains section length");
                    Check(fruits.Movement.Length == 41, "osu!catch strains length");
                    CheckApprox(fruits.Movement[11], 0.35301054964344125, "osu!catch movement strain");
                }
            }

            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                ManiaStrains mania =
                    new Difficulty { Mods = "4K", Ruleset = Ruleset.Mania }.CalculateStrains(beatmap) as ManiaStrains;
                Check(mania != null, "osu!mania strains");
                if (mania != null) {
                    Check(mania.StartTime == 2800, "osu!mania strains start time");
                    Check(mania.Strain.Length == 282, "osu!mania strains length");
                    CheckApprox(mania.Strain[195], 15.725755965531633, "osu!mania strain");
                }
            }
        }

        private static int Main() {
            TheVersionIsAVersion();
            FromFileReadsTheModel();
            FromBytesMatchesTheFile();
            AMissingFileIsTheFrameworkException();
            DifficultyMatchesThePinnedStars();
            ModsChangeTheCalculation();
            StableModBitsMatchTheirAcronyms();
            ClassicCanBeAddedToStableModBits();
            TheRulesetCanBeForced();
            TheAttributesAreTheLiveRulesetOnly();
            AnUnknownModIsAnArgumentException();
            ARulesetOutsideTheFourIsAnError();
            ThePerformanceOfOsuMatchesThePinnedPp();
            ThePerformanceOfTaikoMatchesThePinnedPp();
            ThePerformanceOfCatchMatchesThePinnedPp();
            ThePerformanceOfManiaMatchesThePinnedPp();
            ThePerformanceTakesTheComboAndMissesItIsGiven();
            ThePerformanceFromAttributesMatchesTheMap();
            TheTimedAttributesMatchUpstream();
            TheStrainGraphMatchesUpstream();

            Console.WriteLine(checks + " checks, " + failures + " failures");
            return failures == 0 ? 0 : 1;
        }
    }
}
