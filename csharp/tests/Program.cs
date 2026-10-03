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

        private static void CheckThrows(Throwing action, string message, string what) {
            checks++;
            try {
                action();
            } catch (Exception error) {
                if (error.Message == message) {
                    return;
                }
                failures++;
                Console.WriteLine("FAIL: " + what + " (message was " + error.Message + ")");
                return;
            }
            failures++;
            Console.WriteLine("FAIL: " + what + " (nothing was thrown)");
        }

        // The hit-result indices are the library's: `GREAT = 5`, `SMALL_TICK_HIT = 8`,
        // `LARGE_TICK_HIT = 10` and `SLIDER_TAIL_HIT = 16`.
        private static ScoreInfo Score(int[] indices, int[] counts, int combo) {
            ScoreInfo score = new ScoreInfo();
            for (int i = 0; i < indices.Length; i++) {
                score.statistics[indices[i]] = counts[i];
            }
            score.max_combo = combo;
            score.accuracy = 1.0;
            return score;
        }

        private static void TheVersionIsAVersion() {
            string version = Version.Current;
            Check(version != null && version.Split('.').Length == 3, "the version is a version");
        }

        private static void FromFileReadsTheModel() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/diffcalc-test.osu"))) {
                Check(beatmap.format_version == 14, "format_version");
                Check(beatmap.mode == 0, "mode");
                CheckApprox(beatmap.stack_leniency, 0.3, "stack_leniency");
                Check(beatmap.difficulty.drain_rate == 5.0, "drain_rate");
                Check(beatmap.difficulty.circle_size == 4.0, "circle_size");
                Check(beatmap.difficulty.overall_difficulty == 7.0, "overall_difficulty");
                CheckApprox(beatmap.difficulty.approach_rate, 8.3, "approach_rate");
                Check(beatmap.difficulty.slider_multiplier == 1.6, "slider_multiplier");
                Check(beatmap.difficulty.slider_tick_rate == 1.0, "slider_tick_rate");
                Check(beatmap.hit_objects.Length == 124, "hit_objects");
                Check(beatmap.sliders.Length == 33, "sliders");
                Check(beatmap.timing_points.Length == 3, "timing_points");
                Check(beatmap.breaks.Length == 0, "breaks");

                int on_sliders = 0;
                for (int i = 0; i < beatmap.hit_objects.Length; i++) {
                    if (beatmap.hit_objects[i].slider >= 0) {
                        on_sliders++;
                        Check(beatmap.hit_objects[i].slider < beatmap.sliders.Length,
                              "a hit object's slider index is in range");
                    }
                }
                Check(on_sliders == beatmap.sliders.Length, "hit objects point at their sliders");

                Slider slider = beatmap.sliders[0];
                Check(slider.slides == 1, "the first slider's slides");
                Check(slider.expected_length == 160.0, "the first slider's expected_length");
                Check(slider.control_points.Length == 1, "the first slider's control_points");
                Check(slider.events.Length == 3, "the first slider's events");
                Check(slider.path.Length == slider.cumulative_lengths.Length,
                      "a slider's path matches its cumulative lengths");
            }
        }

        private static void AMissingFileIsAnError() {
            CheckThrows(delegate { Beatmap.FromFile(MapPath("osu/does-not-exist.osu")); },
                        "cannot parse the beatmap (result 3)", "a missing file is an error");
        }

        private static void DifficultyMatchesThePinnedStars() {
            string[] names = {"osu/2785319.osu", "taiko/1028484.osu", "fruits/2118524.osu",
                              "mania/1638954.osu"};
            double[] stars = {6.004027372552197, 2.9144215641617333, 3.2340182503279706,
                              3.358304846842773};
            int[] combos = {909, 289, 730, 956};

            for (int i = 0; i < names.Length; i++) {
                using (Beatmap beatmap = Beatmap.FromFile(MapPath(names[i]))) {
                    DifficultyAttributes attributes = new Difficulty(beatmap).Calculate();
                    CheckClose(attributes.star_rating, stars[i], names[i] + " star rating");
                    Check(attributes.max_combo == combos[i], names[i] + " max combo");
                }
            }
        }

        private static void ModsChangeTheCalculation() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes attributes = new Difficulty(beatmap).Mods("HR").Calculate();
                CheckClose(attributes.star_rating, 4.308291009137178, "HR star rating");
            }
        }

        private static void TheRulesetCanBeForced() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                DifficultyAttributes attributes =
                    new Difficulty(beatmap).Ruleset(Ruleset.Taiko).Calculate();
                Check(attributes.ruleset == (int)Ruleset.Taiko, "the forced ruleset");
                CheckClose(attributes.taiko.star_rating, 4.752572620626138, "the taiko star rating");
                Check(attributes.taiko.mechanical_difficulty > 0.0, "the taiko mechanical difficulty");
            }
        }

        private static void ATaikoMapFillsTheTaikoAttributes() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("taiko/1028484.osu"))) {
                DifficultyAttributes attributes = new Difficulty(beatmap).Calculate();
                Check(attributes.ruleset == (int)Ruleset.Taiko, "a taiko map's ruleset");
                Check(attributes.osu.star_rating == 0.0, "the inactive osu attributes");
                Check(attributes.taiko.mechanical_difficulty > 0.0, "the taiko mechanical difficulty");
            }
        }

        private static void AnUnknownModIsAnError() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                CheckThrows(delegate { new Difficulty(beatmap).Mods("XX").Calculate(); },
                            "invalid mod specification", "an unknown mod is an error");
            }
        }

        private static void ARulesetOutsideTheFourIsAnError() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                CheckThrows(delegate { new Difficulty(beatmap).Ruleset((Ruleset)9); },
                            "ruleset must be one of 0, 1, 2, 3", "a ruleset outside the four");
            }
        }

        private static void ThePerformanceOfOsuMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                DifficultyAttributes difficulty = new Difficulty(beatmap).Calculate();
                ScoreInfo score = Score(new[] {5, 16}, new[] {601, difficulty.osu.slider_count}, 909);
                PerformanceAttributes play =
                    new Performance(beatmap).State(score).Calculate();
                CheckClose(play.total, 316.5901855625614, "the osu total");
                CheckClose(play.osu.aim, 148.75278891878943, "the osu aim");
                CheckClose(play.osu.speed, 61.34653468094172, "the osu speed");
                CheckClose(play.osu.accuracy, 98.99847982709288, "the osu accuracy");
                CheckClose(play.osu.reading, 2.2291238201795176, "the osu reading");
                Check(!play.osu.score_based_estimated_miss_count.HasValue,
                      "the unset optional stays null");
            }
        }

        private static void ThePerformanceOfTaikoMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("taiko/1028484.osu"))) {
                ScoreInfo score = Score(new[] {5}, new[] {289}, 289);
                PerformanceAttributes play = new Performance(beatmap).State(score).Calculate();
                Check(play.ruleset == (int)Ruleset.Taiko, "the taiko ruleset");
                CheckClose(play.total, 130.26636361095524, "the taiko total");
                CheckClose(play.taiko.difficulty, 33.48488833057447, "the taiko difficulty");
                CheckClose(play.taiko.accuracy, 96.78147528038076, "the taiko accuracy");
                CheckClose(play.taiko.estimated_unstable_rate.Value, 146.3238357972284,
                           "the taiko unstable rate");
                Check(play.taiko.estimated_unstable_rate.HasValue, "the set optional has a value");
            }
        }

        private static void ThePerformanceOfCatchMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes difficulty = new Difficulty(beatmap).Calculate();
                ScoreInfo score = Score(new[] {5, 10, 8}, new[] {728, 2, 263}, difficulty.max_combo);
                PerformanceAttributes play = new Performance(beatmap).State(score).Calculate();
                Check(play.ruleset == (int)Ruleset.Catch, "the catch ruleset");
                CheckClose(play.total, 112.72215339177879, "the catch total");
            }
        }

        private static void ThePerformanceOfManiaMatchesThePinnedPp() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("mania/1638954.osu"))) {
                DifficultyAttributes difficulty = new Difficulty(beatmap).Calculate();
                ScoreInfo score = Score(new[] {6}, new[] {715}, difficulty.max_combo);
                PerformanceAttributes play = new Performance(beatmap).State(score).Calculate();
                Check(play.ruleset == (int)Ruleset.Mania, "the mania ruleset");
                CheckClose(play.total, 108.92297471705167, "the mania total");
            }
        }

        private static void ThePerformanceTakesTheComboAndMissesItIsGiven() {
            using (Beatmap beatmap = Beatmap.FromFile(MapPath("osu/2785319.osu"))) {
                PerformanceAttributes play =
                    new Performance(beatmap).Combo(909).Accuracy(1.0).Misses(0).Calculate();
                Check(play.total > 0.0, "a play with an explicit combo");
            }
        }

        private static int Main() {
            TheVersionIsAVersion();
            FromFileReadsTheModel();
            AMissingFileIsAnError();
            DifficultyMatchesThePinnedStars();
            ModsChangeTheCalculation();
            TheRulesetCanBeForced();
            ATaikoMapFillsTheTaikoAttributes();
            AnUnknownModIsAnError();
            ARulesetOutsideTheFourIsAnError();
            ThePerformanceOfOsuMatchesThePinnedPp();
            ThePerformanceOfTaikoMatchesThePinnedPp();
            ThePerformanceOfCatchMatchesThePinnedPp();
            ThePerformanceOfManiaMatchesThePinnedPp();
            ThePerformanceTakesTheComboAndMissesItIsGiven();

            Console.WriteLine(checks + " checks, " + failures + " failures");
            return failures == 0 ? 0 : 1;
        }
    }
}
