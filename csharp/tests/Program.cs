using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using Newtonsoft.Json.Linq;
using Pppp;

namespace Pppp.Tests {
    internal static class Program {
        private const double Epsilon = 1e-9;

        private static int checks;
        private static int failures;
        private static string resources;
        private static JObject parityRoot;

        private static string Resources {
            get {
                if (resources == null) {
                    resources = FindUp(Path.Combine("tests", "resources"), true);
                }
                return resources;
            }
        }

        private static JObject ParityRoot {
            get {
                if (parityRoot == null) {
                    parityRoot = JObject.Parse(
                        File.ReadAllText(FindUp(Path.Combine(Path.Combine("tests", "parity"), "parity.json"), false)));
                }
                return parityRoot;
            }
        }

        private static string FindUp(string relative, bool directory) {
            DirectoryInfo current = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
            while (current != null) {
                string candidate = Path.Combine(current.FullName, relative);
                if (directory ? Directory.Exists(candidate) : File.Exists(candidate)) {
                    return candidate;
                }
                current = current.Parent;
            }
            throw new FileNotFoundException(relative);
        }

        private static string MapPath(string name) {
            return Path.Combine(Resources, name.Replace('/', Path.DirectorySeparatorChar));
        }

        private static Beatmap BeatmapOf(string name) {
            return Beatmap.FromFile(MapPath(name));
        }

        private static bool Approx(double actual, double expected, double epsilon) {
            return Math.Abs(actual - expected) < epsilon * (1.0 + Math.Max(Math.Abs(actual), Math.Abs(expected)));
        }

        private static void Check(bool condition, string what) {
            checks++;
            if (!condition) {
                failures++;
                Console.WriteLine("FAIL: " + what);
            }
        }

        private static void CheckClose(double actual, double expected, string what) {
            Check(Approx(actual, expected, Epsilon), what + " (was " + actual + ")");
        }

        private static void CheckApprox(double actual, double expected, double epsilon, string what) {
            Check(Approx(actual, expected, epsilon), what + " (was " + actual + ")");
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
            string version = VersionInfo.Current;
            Check(version != null && version.Split('.').Length == 3, "the version is a version");
        }

        private static void FromFileReadsTheModel() {
            using (Beatmap beatmap = BeatmapOf("osu/diffcalc-test.osu")) {
                Check(beatmap.FormatVersion == 14, "the format version");
                Check(beatmap.Mode == 0, "the mode");
                CheckApprox(beatmap.StackLeniency, 0.3, 1e-6, "the stack leniency");
                Check(beatmap.Difficulty.DrainRate == 5.0, "the drain rate");
                Check(beatmap.Difficulty.CircleSize == 4.0, "the circle size");
                Check(beatmap.Difficulty.OverallDifficulty == 7.0, "the overall difficulty");
                CheckApprox(beatmap.Difficulty.ApproachRate, 8.3, 1e-6, "the approach rate");
                Check(beatmap.Difficulty.SliderMultiplier == 1.6, "the slider multiplier");
                Check(beatmap.Difficulty.SliderTickRate == 1.0, "the slider tick rate");
                Check(beatmap.HitObjects.Length == 124, "the hit objects");
                Check(beatmap.Sliders.Length == 33, "the sliders");
                Check(beatmap.TimingPoints.Length == 3, "the timing points");
                Check(beatmap.Breaks.Length == 0, "the breaks");
            }
        }

        private static void FromFileResolvesToTheSameBeatmap() {
            string path = MapPath("osu/diffcalc-test.osu");
            using (Beatmap first = Beatmap.FromFile(path))
            using (Beatmap second = Beatmap.FromFile(path)) {
                CheckClose(new Difficulty().Calculate(first).StarRating,
                           new Difficulty().Calculate(second).StarRating, "fromFile resolves to the same beatmap");
            }
        }

        private static void FromBytesMatchesTheFile() {
            string path = MapPath("osu/diffcalc-test.osu");
            using (Beatmap fromBytes = Beatmap.FromBytes(File.ReadAllBytes(path)))
            using (Beatmap fromFile = Beatmap.FromFile(path)) {
                CheckClose(new Difficulty().Calculate(fromBytes).StarRating,
                           new Difficulty().Calculate(fromFile).StarRating, "fromBytes matches the file");
            }
        }

        private static void AMissingFileIsTheFrameworkException() {
            CheckThrows<FileNotFoundException>(
                delegate { Beatmap.FromFile(MapPath("osu/does-not-exist.osu")); },
                "a missing file is the framework exception");
        }

        private static void FromBytesTakesBytesOnly() {
            CheckThrows<ArgumentNullException>(delegate { Beatmap.FromBytes(null); },
                                               "fromBytes takes bytes only");
        }

        private static void ABEatmapIsNotConstructedDirectly() {
            ConstructorInfo[] constructors = typeof(Beatmap).GetConstructors(
                BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
            bool hidden = true;
            foreach (ConstructorInfo constructor in constructors) {
                if (constructor.IsPublic) {
                    hidden = false;
                }
            }
            Check(hidden, "a beatmap is not constructed directly");
        }

        private static void HitObjectsPointAtTheirSliders() {
            using (Beatmap beatmap = BeatmapOf("osu/diffcalc-test.osu")) {
                int onSliders = 0;
                foreach (HitObject item in beatmap.HitObjects) {
                    if (item.Slider < 0) {
                        continue;
                    }
                    onSliders++;
                    Check(item.Slider < beatmap.Sliders.Length, "a hit object's slider index is in range");
                }
                Check(onSliders == beatmap.Sliders.Length, "hit objects point at their sliders");
            }
        }

        private static void TheSliderGeometryIsExposed() {
            using (Beatmap beatmap = BeatmapOf("osu/diffcalc-test.osu")) {
                Slider slider = beatmap.Sliders[0];
                Check(slider.Slides == 1, "the slider's slides");
                Check(slider.ExpectedLength == 160.0, "the slider's expected length");
                Check(slider.ControlPoints.Length == 1, "the slider's control points");
                Check(slider.Events.Length == 3, "the slider's events");
                Check(slider.Path.Length == slider.CumulativeLengths.Length,
                      "the slider's path matches its cumulative lengths");
            }
        }

        private static void ASpinnerIsCentred() {
            using (Beatmap beatmap = BeatmapOf("osu/diffcalc-test.osu")) {
                HitObject last = beatmap.HitObjects[beatmap.HitObjects.Length - 1];
                Check((last.Type & 8) != 0, "a spinner is a spinner");
                Check(last.Position.X == 256.0f && last.Position.Y == 192.0f, "a spinner is centred");
                Check(last.StartTime == 102125.0 && last.EndTime == 103000.0, "a spinner's times");
            }
        }

        private static void OneConfigurationServesManyMaps() {
            Difficulty difficulty = new Difficulty { Mods = "HR" };
            using (Beatmap catchMap = BeatmapOf("fruits/2118524.osu"))
            using (Beatmap osuMap = BeatmapOf("osu/2785319.osu")) {
                Check(difficulty.Calculate(catchMap) is CatchDifficultyAttributes,
                      "one configuration serves catch");
                Check(difficulty.Calculate(osuMap) is OsuDifficultyAttributes, "one configuration serves osu");
            }
        }

        private static void TheDifficultyMatchesThePinnedStars() {
            string[] names = {"osu/2785319.osu", "taiko/1028484.osu", "fruits/2118524.osu", "mania/1638954.osu"};
            double[] stars = {6.004027372552197, 2.9144215641617333, 3.2340182503279706, 3.358304846842773};
            int[] combos = {909, 289, 730, 956};
            for (int i = 0; i < names.Length; i++) {
                using (Beatmap beatmap = BeatmapOf(names[i])) {
                    DifficultyAttributes attributes = new Difficulty().Calculate(beatmap);
                    CheckClose(attributes.StarRating, stars[i], names[i] + " star rating");
                    Check(attributes.MaxCombo == combos[i], names[i] + " max combo");
                }
            }
        }

        private static void ModsChangeTheCalculation() {
            using (Beatmap beatmap = BeatmapOf("fruits/2118524.osu")) {
                CheckClose(new Difficulty { Mods = "HR" }.Calculate(beatmap).StarRating, 4.308291009137178,
                           "mods change the calculation");
            }
        }

        private static void ClassicScoresOnTheLazerAlgorithm() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                PerformanceAttributes classic = ClassicPlay(beatmap, "CL", difficulty);
                PerformanceAttributes twice = ClassicPlay(beatmap, "CL,CL", difficulty);
                PerformanceAttributes lazer = ClassicPlay(beatmap, null, difficulty);
                CheckClose(classic.Total, 298.5325815982227, "classic scores on the lazer algorithm");
                CheckClose(twice.Total, classic.Total, "adding classic twice changes nothing");
                CheckClose(lazer.Total, 316.5901855625614, "the same score without classic stays on lazer");
            }
        }

        private static void ClassicWorksFromTheStableBits() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                Mods plainBits = LegacyMods.None;
                PerformanceAttributes plain = new Performance {
                    Mods = plainBits,
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                PerformanceAttributes classic = new Performance {
                    Mods = plainBits.WithClassic(),
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                CheckClose(classic.Total, 298.5325815982227, "classic works from the stable bits");
                CheckClose(plain.Total, 316.5901855625614, "the same bits without classic stay on lazer");
            }
        }

        private static PerformanceAttributes ClassicPlay(Beatmap beatmap, string mods,
                                                         OsuDifficultyAttributes difficulty) {
            Performance play = new Performance {
                MaxCombo = 909,
                Accuracy = 1.0,
                Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
            };
            if (mods != null) {
                play.Mods = mods;
            }
            return play.Calculate(beatmap);
        }

        private static void StableModBitsMatchTheirAcronyms() {
            using (Beatmap beatmap = BeatmapOf("fruits/2118524.osu")) {
                CheckClose(new Difficulty { Mods = LegacyMods.HardRock }.Calculate(beatmap).StarRating,
                           new Difficulty { Mods = "HR" }.Calculate(beatmap).StarRating,
                           "stable mod bits match their acronyms");
            }
        }

        private static void TheRulesetCanBeForced() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                DifficultyAttributes attributes = new Difficulty { Ruleset = Ruleset.Taiko }.Calculate(beatmap);
                TaikoDifficultyAttributes taiko = attributes as TaikoDifficultyAttributes;
                Check(attributes.Ruleset == Ruleset.Taiko, "the forced ruleset");
                CheckClose(attributes.StarRating, 4.752572620626138, "the forced ruleset's star rating");
                Check(taiko != null && taiko.MechanicalDifficulty > 0.0,
                      "the forced ruleset's mechanical difficulty");
            }
        }

        private static void TheAttributesAreTheLiveRulesetOnly() {
            using (Beatmap beatmap = BeatmapOf("taiko/1028484.osu")) {
                DifficultyAttributes attributes = new Difficulty().Calculate(beatmap);
                Check(attributes.Ruleset == Ruleset.Taiko, "the live ruleset");
                Check(attributes is TaikoDifficultyAttributes, "the live ruleset's class");
                Check(!(attributes is OsuDifficultyAttributes), "no osu attributes on a taiko map");
            }
        }

        private static void AnUnknownModIsAnError() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                CheckThrows<ArgumentException>(
                    delegate { new Difficulty { Mods = "XX" }.Calculate(beatmap); },
                    "an unknown mod is an error");
            }
        }

        private static void ABeatmapIsRequired() {
            CheckThrows<ArgumentNullException>(delegate { new Difficulty().Calculate(null); },
                                               "a beatmap is required");
        }

        private static void ARulesetOutsideTheFourIsAnError() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                CheckThrows<ArgumentOutOfRangeException>(
                    delegate { new Difficulty { Ruleset = (Ruleset)9 }.Calculate(beatmap); },
                    "a ruleset outside the four is an error");
            }
        }

        private static void ThePerformanceOfOsuMatchesThePinnedPp() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                PerformanceAttributes play = new Performance {
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                }.Calculate(beatmap);
                OsuPerformanceAttributes osu = (OsuPerformanceAttributes)play;
                Check(play.Ruleset == Ruleset.Osu, "the osu ruleset");
                CheckClose(play.Total, 316.5901855625614, "the osu total");
                CheckClose(osu.Aim, 148.75278891878943, "the osu aim");
                CheckClose(osu.Speed, 61.34653468094172, "the osu speed");
                CheckClose(osu.Accuracy, 98.99847982709288, "the osu accuracy");
                CheckClose(osu.Reading, 2.2291238201795176, "the osu reading");
                Check(!osu.ScoreBasedEstimatedMissCount.HasValue, "the osu score based estimated miss count");
            }
        }

        private static void ThePerformanceOfTaikoMatchesThePinnedPp() {
            using (Beatmap beatmap = BeatmapOf("taiko/1028484.osu")) {
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
                Check(taiko.EstimatedUnstableRate.HasValue &&
                          Approx(taiko.EstimatedUnstableRate.GetValueOrDefault(), 146.3238357972284, Epsilon),
                      "the taiko estimated unstable rate");
            }
        }

        private static void ThePerformanceOfCatchMatchesThePinnedPp() {
            using (Beatmap beatmap = BeatmapOf("fruits/2118524.osu")) {
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
            using (Beatmap beatmap = BeatmapOf("mania/1638954.osu")) {
                DifficultyAttributes difficulty = new Difficulty().Calculate(beatmap);
                PerformanceAttributes play = new Performance {
                    MaxCombo = difficulty.MaxCombo,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Perfect, 715}},
                }.Calculate(beatmap);
                Check(play.Ruleset == Ruleset.Mania, "the mania ruleset");
                CheckClose(play.Total, 108.92297471705167, "the mania total");
                CheckClose(((ManiaPerformanceAttributes)play).Difficulty, 108.92297471705167,
                           "the mania difficulty");
            }
        }

        private static void ThePerformanceTakesTheComboAndMissesItIsGiven() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                PerformanceAttributes play = new Performance { MaxCombo = 909, Accuracy = 1.0, Misses = 0 }
                                                         .Calculate(beatmap);
                Check(play.Total > 0.0, "the performance takes the combo and misses it is given");
            }
        }

        private static void AStatisticsKeyOutsideTheHitResultsIsAnError() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                CheckThrows<ArgumentException>(
                    delegate { new Performance { Statistics = {{(HitResult)99, 1}} }.Calculate(beatmap); },
                    "a statistics key outside the hit results is an error");
            }
        }

        private static void ThePerformanceFromAttributesMatchesTheMap() {
            using (Beatmap beatmap = BeatmapOf("osu/2785319.osu")) {
                OsuDifficultyAttributes difficulty = (OsuDifficultyAttributes)new Difficulty().Calculate(beatmap);
                Performance performance = new Performance {
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Statistics = {{HitResult.Great, 601}, {HitResult.SliderTailHit, difficulty.SliderCount}},
                };
                CheckClose(performance.Calculate(beatmap, difficulty).Total,
                           performance.Calculate(beatmap).Total,
                           "the performance from attributes matches the map");
            }
        }

        private static void TheTimedAttributesMatchUpstream() {
            string[] names = {"osu/diffcalc-test.osu", "osu/2785319.osu", "taiko/diffcalc-test.osu",
                              "fruits/diffcalc-test.osu", "mania/diffcalc-test.osu", "osu/2785319.osu"};
            string[] mods = {"NM", "HD,DT", "DT", "DT", "NM", "4K"};
            Ruleset[] rulesets = {Ruleset.Osu, Ruleset.Osu, Ruleset.Taiko, Ruleset.Catch, Ruleset.Mania, Ruleset.Mania};
            int[] counts = {124, 601, 238, 93, 137, 838};
            double[][] middles = {
                new[] {18000.0, 5.727266936277771, 63.0},   new[] {61309.235290751734, 8.272126401328721, 433.0},
                new[] {21624.0, 4.397186455763557, 118.0},  new[] {14500.0, 4.27412411307246, 47.0},
                new[] {16250.0, 1.8228476946125378, 69.0},  new[] {61662.0, 2.5718820820666117, 530.0},
            };
            double[][] lasts = {
                new[] {103000.0, 6.524323005451468, 239.0},   new[] {115486.23529075173, 8.898179651893287, 909.0},
                new[] {53000.0, 4.455142137225538, 200.0},    new[] {45250.0, 5.152717389780087, 127.0},
                new[] {30500.0, 2.3493769750220914, 242.0},   new[] {115486.0, 2.653795415351293, 1072.0},
            };

            for (int i = 0; i < names.Length; i++) {
                string what = names[i] + " " + mods[i];
                using (Beatmap beatmap = BeatmapOf(names[i])) {
                    TimedDifficultyAttributes[] timed =
                        new Difficulty { Mods = mods[i], Ruleset = rulesets[i] }.CalculateTimed(beatmap);
                    Check(timed.Length == counts[i], what + " timed count");

                    int middle = counts[i] / 2;
                    foreach (int at in new[] {middle, counts[i] - 1}) {
                        double[] expected = at == middle ? middles[i] : lasts[i];
                        Check(timed[at].Time == expected[0], what + " timed time");
                        CheckClose(timed[at].Attributes.StarRating, expected[1], what + " timed star rating");
                        Check(timed[at].Attributes.MaxCombo == (int)expected[2], what + " timed max combo");
                        Check(timed[at].Attributes.Ruleset == rulesets[i], what + " timed ruleset");
                    }
                }
            }
        }

        private static void TheStrainGraphMatchesUpstream() {
            string[] names = {"osu/diffcalc-test.osu", "osu/2785319.osu", "taiko/diffcalc-test.osu",
                              "fruits/diffcalc-test.osu", "mania/diffcalc-test.osu", "osu/2785319.osu"};
            string[] mods = {"HD,FL", "DT", "DT", "DT", "NM", "4K"};
            Ruleset[] rulesets = {Ruleset.Osu, Ruleset.Osu, Ruleset.Taiko, Ruleset.Catch, Ruleset.Mania, Ruleset.Mania};
            double[] starts = {800, 2400, 0, 0, 400, 2800};
            double[] sections = {400, 600, 600, 1125, 400, 400};
            int[] counts = {254, 189, 89, 41, 76, 282};
            int[] indexes = {18, 66, 6, 11, 70, 195};
            double[] values = {
                41.02690817661656, 517.1805782727978, 4.473586847131765,
                0.35301054964344125, 16.8859847732691, 15.725755965531633,
            };

            for (int i = 0; i < names.Length; i++) {
                string what = names[i] + " " + mods[i];
                using (Beatmap beatmap = BeatmapOf(names[i])) {
                    Strains strains =
                        new Difficulty { Mods = mods[i], Ruleset = rulesets[i] }.CalculateStrains(beatmap);
                    Check(strains.Ruleset == rulesets[i], what + " strain ruleset");
                    Check(strains.StartTime == starts[i], what + " strain start time");
                    Check(strains.SectionLength == sections[i], what + " strain section length");
                    double[] series = StrainSeries(strains, i);
                    Check(series.Length == counts[i], what + " strain count");
                    CheckClose(series[indexes[i]], values[i], what + " strain value");
                }
            }
        }

        private static double[] StrainSeries(Strains strains, int index) {
            switch (index) {
            case 0:
                return ((OsuStrains)strains).Flashlight;
            case 1:
                return ((OsuStrains)strains).AimNoSliders;
            case 2:
                return ((TaikoStrains)strains).SingleColourStamina;
            case 3:
                return ((CatchStrains)strains).Movement;
            default:
                return ((ManiaStrains)strains).Strain;
            }
        }

        private static bool EngineIsBesideTheNativeLibrary() {
            List<string> roots = new List<string> {AppDomain.CurrentDomain.BaseDirectory};
            string loaderPath = Environment.GetEnvironmentVariable("LD_LIBRARY_PATH");
            if (string.IsNullOrEmpty(loaderPath)) {
                loaderPath = Environment.GetEnvironmentVariable("PATH");
            }
            if (!string.IsNullOrEmpty(loaderPath)) {
                roots.AddRange(loaderPath.Split(Path.PathSeparator));
            }
            foreach (string root in roots) {
                if (Directory.Exists(root) &&
                    Directory.GetFiles(root, "*fosu_engine_*", SearchOption.AllDirectories).Length > 0) {
                    return true;
                }
            }
            return false;
        }

        private static string ProbeBackend(string variant) {
            string entry = Assembly.GetEntryAssembly().Location;
            ProcessStartInfo info = entry.EndsWith(".exe")
                                        ? new ProcessStartInfo(entry, "--print-backend")
                                        : new ProcessStartInfo("dotnet", "\"" + entry + "\" --print-backend");
            info.UseShellExecute = false;
            info.RedirectStandardOutput = true;
            info.EnvironmentVariables["FOSU_BACKEND"] = variant;
            using (Process process = Process.Start(info)) {
                string output = process.StandardOutput.ReadToEnd();
                process.WaitForExit();
                return output.Trim();
            }
        }

        private static void BackendReportsTheEngine() {
            if (Environment.GetEnvironmentVariable("FOSU_BACKEND") == "scalar") {
                Check(Backend.Name == "scalar", "backend reports the engine");
                return;
            }
            string expected = "scalar";
            foreach (string variant in new[] {"avx2", "neon"}) {
                if (ProbeBackend(variant) == variant) {
                    expected = variant;
                    break;
                }
            }
            Check(Backend.Name == expected, "backend reports the engine");
        }

        private static void BackendShipsBesideTheLibrary() {
            Check(EngineIsBesideTheNativeLibrary(), "backend ships beside the library");
        }

        private static void FOSUBackendScalarForcesTheScalarEngine() {
            Check(ProbeBackend("scalar") == "scalar", "FOSU_BACKEND=scalar forces the scalar engine");
        }

        private static void TheDecodeParityRowsMatchUpstream() {
            foreach (JToken token in (JArray)ParityRoot["decode"]) {
                JObject row = (JObject)token;
                string map = (string)row["map"];
                JObject expected = (JObject)row["decode"];
                using (Beatmap beatmap = BeatmapOf(map)) {
                    int uninherited = 0;
                    foreach (TimingPoint point in beatmap.TimingPoints) {
                        if (point.Uninherited) {
                            uninherited++;
                        }
                    }
                    Check(beatmap.FormatVersion == (int)expected["format"], map + " format");
                    Check(beatmap.Mode == (int)expected["mode"], map + " mode");
                    Check(beatmap.HitObjects.Length == (int)expected["hit_objects"], map + " hit objects");
                    Check(uninherited == (int)expected["uninherited_points"], map + " uninherited points");
                    Check(beatmap.Breaks.Length == (int)expected["breaks"], map + " breaks");
                    CheckApprox(beatmap.Difficulty.ApproachRate, (double)expected["ar"], 1e-6, map + " ar");
                    CheckApprox(beatmap.Difficulty.OverallDifficulty, (double)expected["od"], 1e-6, map + " od");
                    CheckApprox(beatmap.Difficulty.CircleSize, (double)expected["cs"], 1e-6, map + " cs");
                    CheckApprox(beatmap.Difficulty.DrainRate, (double)expected["hp"], 1e-6, map + " hp");
                    CheckApprox(beatmap.Difficulty.SliderMultiplier, (double)expected["slider_multiplier"], 1e-6,
                                map + " slider multiplier");
                    CheckApprox(beatmap.Difficulty.SliderTickRate, (double)expected["slider_tick_rate"], 1e-6,
                                map + " slider tick rate");
                    CheckApprox(beatmap.StackLeniency, (double)expected["stack_leniency"], 1e-6,
                                map + " stack leniency");
                }
            }
        }

        private static readonly Dictionary<string, Ruleset> ParityRulesets = new Dictionary<string, Ruleset> {
            {"osu", Ruleset.Osu},
            {"taiko", Ruleset.Taiko},
            {"catch", Ruleset.Catch},
            {"mania", Ruleset.Mania},
        };

        private static void TheParityDifficultyRowsMatchUpstream() {
            JObject difficulty = (JObject)ParityRoot["difficulty"];
            foreach (KeyValuePair<string, Ruleset> ruleset in ParityRulesets) {
                foreach (JToken token in (JArray)difficulty[ruleset.Key]) {
                    JObject row = (JObject)token;
                    string what = ruleset.Key + " " + (string)row["map"] + " " + (string)row["mods"];
                    JObject expected = (JObject)row["attributes"];
                    using (Beatmap beatmap = BeatmapOf((string)row["map"])) {
                        DifficultyAttributes attributes =
                            new Difficulty { Mods = (string)row["mods"], Ruleset = ruleset.Value }.Calculate(beatmap);
                        CheckApprox(attributes.StarRating, (double)expected["star_rating"], 1e-6,
                                    what + " star rating");
                        Check(attributes.MaxCombo == (int)expected["max_combo"], what + " max combo");
                    }
                }
            }
        }

        private static void TheParityPerformanceRowsMatchUpstream() {
            JObject performance = (JObject)ParityRoot["performance"];
            foreach (KeyValuePair<string, Ruleset> ruleset in ParityRulesets) {
                foreach (JToken token in (JArray)performance[ruleset.Key]) {
                    JObject row = (JObject)token;
                    string what = ruleset.Key + " " + (string)row["map"] + " " + (string)row["mods"];
                    JObject expected = (JObject)row["attributes"];
                    using (Beatmap beatmap = BeatmapOf((string)row["map"])) {
                        DifficultyAttributes difficulty =
                            new Difficulty { Mods = (string)row["mods"], Ruleset = ruleset.Value }.Calculate(beatmap);
                        PerformanceAttributes play =
                            ParityPlay(ruleset.Key, (string)row["mods"], (JObject)row["score"], difficulty)
                                .Calculate(beatmap, difficulty);
                        CheckApprox(play.Total, (double)expected["total"], Epsilon, what + " total");
                    }
                }
            }
        }

        private static double ClassicAccuracy(int great, int ok, int meh, int miss) {
            int total = great + ok + meh + miss;
            return total <= 0 ? 1.0 : (great * 300.0 + ok * 100.0 + meh * 50.0) / (total * 300.0);
        }

        private static Performance ParityPlay(string mode, string mods, JObject score,
                                               DifficultyAttributes attributes) {
            Performance play = new Performance { Mods = mods };
            switch (mode) {
            case "osu": {
                OsuDifficultyAttributes osu = (OsuDifficultyAttributes)attributes;
                play.MaxCombo = (int)score["combo"];
                double accuracy = (double)score["accuracy"];
                play.Accuracy = accuracy < 0.0
                                    ? ClassicAccuracy((int)score["great"], (int)score["ok"], (int)score["meh"],
                                                      (int)score["miss"])
                                    : accuracy;
                play.Statistics[HitResult.Great] = (int)score["great"];
                play.Statistics[HitResult.Ok] = (int)score["ok"];
                play.Statistics[HitResult.Meh] = (int)score["meh"];
                play.Statistics[HitResult.Miss] = (int)score["miss"];
                play.Statistics[HitResult.SliderTailHit] = osu.SliderCount;
                long legacy = (long)score["legacy_total_score"];
                if (legacy != 0) {
                    play.LegacyTotalScore = legacy;
                }
                break;
            }
            case "taiko": {
                int great = (int)score["great"];
                int ok = (int)score["ok"];
                int miss = (int)score["miss"];
                int combo = (int)score["combo"];
                play.MaxCombo = combo != 0 ? combo : attributes.MaxCombo;
                play.Accuracy = (great * 300.0 + ok * 150.0) / ((great + ok + miss) * 300.0);
                play.Statistics[HitResult.Great] = great;
                play.Statistics[HitResult.Ok] = ok;
                play.Statistics[HitResult.Miss] = miss;
                break;
            }
            case "catch": {
                int fruits = (int)score["fruits"];
                int droplets = (int)score["droplets"];
                int tiny = (int)score["tiny_droplets"];
                int tinyMisses = (int)score["tiny_misses"];
                int miss = (int)score["miss"];
                int combo = (int)score["combo"];
                play.MaxCombo = combo != 0 ? combo : attributes.MaxCombo;
                play.Accuracy = (double)(fruits + droplets + tiny) / (fruits + droplets + tiny + tinyMisses + miss);
                play.Statistics[HitResult.Great] = fruits;
                play.Statistics[HitResult.LargeTickHit] = droplets;
                play.Statistics[HitResult.SmallTickHit] = tiny;
                play.Statistics[HitResult.SmallTickMiss] = tinyMisses;
                play.Statistics[HitResult.Miss] = miss;
                break;
            }
            default: {
                int perfect = (int)score["perfect"];
                int great = (int)score["great"];
                int good = (int)score["good"];
                int ok = (int)score["ok"];
                int meh = (int)score["meh"];
                int miss = (int)score["miss"];
                int combo = (int)score["combo"];
                play.MaxCombo = combo != 0 ? combo : attributes.MaxCombo;
                play.Accuracy = (perfect * 305.0 + great * 300.0 + good * 200.0 + ok * 100.0 + meh * 50.0) /
                                ((perfect + great + good + ok + meh + miss) * 305.0);
                play.Statistics[HitResult.Perfect] = perfect;
                play.Statistics[HitResult.Great] = great;
                play.Statistics[HitResult.Good] = good;
                play.Statistics[HitResult.Ok] = ok;
                play.Statistics[HitResult.Meh] = meh;
                play.Statistics[HitResult.Miss] = miss;
                break;
            }
            }
            return play;
        }

        private static int Main(string[] args) {
            if (args.Length > 0 && args[0] == "--print-backend") {
                Console.WriteLine(Backend.Name);
                return 0;
            }
            TheVersionIsAVersion();
            FromFileReadsTheModel();
            FromFileResolvesToTheSameBeatmap();
            FromBytesMatchesTheFile();
            AMissingFileIsTheFrameworkException();
            FromBytesTakesBytesOnly();
            ABEatmapIsNotConstructedDirectly();
            HitObjectsPointAtTheirSliders();
            TheSliderGeometryIsExposed();
            ASpinnerIsCentred();
            OneConfigurationServesManyMaps();
            TheDifficultyMatchesThePinnedStars();
            ModsChangeTheCalculation();
            ClassicScoresOnTheLazerAlgorithm();
            ClassicWorksFromTheStableBits();
            StableModBitsMatchTheirAcronyms();
            TheRulesetCanBeForced();
            TheAttributesAreTheLiveRulesetOnly();
            AnUnknownModIsAnError();
            ABeatmapIsRequired();
            ARulesetOutsideTheFourIsAnError();
            ThePerformanceOfOsuMatchesThePinnedPp();
            ThePerformanceOfTaikoMatchesThePinnedPp();
            ThePerformanceOfCatchMatchesThePinnedPp();
            ThePerformanceOfManiaMatchesThePinnedPp();
            ThePerformanceTakesTheComboAndMissesItIsGiven();
            AStatisticsKeyOutsideTheHitResultsIsAnError();
            ThePerformanceFromAttributesMatchesTheMap();
            TheTimedAttributesMatchUpstream();
            TheStrainGraphMatchesUpstream();
            BackendReportsTheEngine();
            BackendShipsBesideTheLibrary();
            FOSUBackendScalarForcesTheScalarEngine();
            TheDecodeParityRowsMatchUpstream();
            TheParityDifficultyRowsMatchUpstream();
            TheParityPerformanceRowsMatchUpstream();

            Console.WriteLine(checks + " checks, " + failures + " failures");
            return failures == 0 ? 0 : 1;
        }
    }
}
