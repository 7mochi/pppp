using System;
using Pppp;
using Xunit;

namespace Pppp.Tests {
    public sealed class DifficultyTests {
        [Theory]
        [InlineData("osu/2785319.osu", 6.004027372552197, 909, Ruleset.Osu)]
        [InlineData("taiko/1028484.osu", 2.9144215641617333, 289, Ruleset.Taiko)]
        [InlineData("fruits/2118524.osu", 3.2340182503279706, 730, Ruleset.Catch)]
        [InlineData("mania/1638954.osu", 3.358304846842773, 956, Ruleset.Mania)]
        public void DifficultyValues(string map, double stars, int combo, Ruleset ruleset) {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath(map))) {
                DifficultyAttributes attributes = new Difficulty().Calculate(beatmap);
                Assert.Equal(ruleset, attributes.Ruleset);
                Assert.Equal(stars, attributes.StarRating);
                Assert.Equal(combo, attributes.MaxCombo);
            }
        }

        [Fact]
        public void ModsChangeTheCalculation() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes named = new Difficulty { Mods = "HR" }.Calculate(beatmap);
                DifficultyAttributes legacy = new Difficulty { Mods = (LegacyMods)16 }.Calculate(beatmap);
                Assert.Equal(4.308291009137178, named.StarRating);
                Assert.Equal(4.308291009137178, legacy.StarRating);
                Assert.Equal(named.StarRating, legacy.StarRating);
                Assert.Equal(named.MaxCombo, legacy.MaxCombo);
            }
        }

        [Fact]
        public void StableModBitsMatchAcronyms() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("fruits/2118524.osu"))) {
                DifficultyAttributes legacy = new Difficulty { Mods = (LegacyMods)16 }.Calculate(beatmap);
                DifficultyAttributes named = new Difficulty { Mods = "HR" }.Calculate(beatmap);
                Assert.Equal(named.StarRating, legacy.StarRating);
                Assert.Equal(named.MaxCombo, legacy.MaxCombo);
            }
        }

        [Fact]
        public void ClockRateChangesTheCalculation() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/diffcalc-test.osu"))) {
                Assert.Equal(9.467769487798352, new Difficulty { Mods = "DT" }.Calculate(beatmap).StarRating);
                Assert.Equal(12.120899066435245, new Difficulty { Mods = "DT", ClockRate = 2.0 }.Calculate(beatmap).StarRating);
            }
        }

        [Fact]
        public void ErrorsUseFrameworkTypes() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                ArgumentException error = Assert.Throws<ArgumentException>(
                    delegate { new Difficulty { Mods = "XX" }.Calculate(beatmap); });
                Assert.Equal("Mods", error.ParamName);
                Assert.Throws<ArgumentOutOfRangeException>(
                    delegate { new Difficulty { Ruleset = (Ruleset)9 }.Calculate(beatmap); });
                Assert.Throws<ArgumentNullException>(
                    delegate { new Difficulty().Calculate(null); });
            }

            Beatmap closed = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"));
            closed.Dispose();
            Assert.Throws<ObjectDisposedException>(
                delegate { new Difficulty().Calculate(closed); });
        }

        [Fact]
        public void ForcedRulesetIsHonoured() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                DifficultyAttributes attributes = new Difficulty { Ruleset = Ruleset.Taiko }.Calculate(beatmap);
                Assert.Equal(Ruleset.Taiko, attributes.Ruleset);
                Assert.Equal(4.752572620626138, attributes.StarRating);
            }
        }

        [Fact]
        public void TimedValues() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/diffcalc-test.osu"))) {
                TimedDifficultyAttributes[] timed = new Difficulty().CalculateTimed(beatmap);
                Assert.Equal(124, timed.Length);
                Assert.Equal(18000, timed[62].Time);
                Assert.Equal(5.727266936277771, timed[62].Attributes.StarRating);
                Assert.Equal(63, timed[62].Attributes.MaxCombo);
                Assert.Equal(103000, timed[timed.Length - 1].Time);
                Assert.Equal(239, timed[timed.Length - 1].Attributes.MaxCombo);
            }
        }

        [Fact]
        public void EmptyTimedResult() {
            string text = "osu file format v14\n[General]\nMode: 0\n";
            using (Beatmap beatmap = Beatmap.FromText(text)) {
                Assert.Empty(new Difficulty().CalculateTimed(beatmap));
            }
        }

        [Theory]
        [InlineData(Ruleset.Osu)]
        [InlineData(Ruleset.Taiko)]
        [InlineData(Ruleset.Catch)]
        [InlineData(Ruleset.Mania)]
        public void TimedRulesets(Ruleset ruleset) {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/diffcalc-test.osu"))) {
                TimedDifficultyAttributes[] timed =
                    new Difficulty { Ruleset = ruleset }.CalculateTimed(beatmap);
                Assert.NotEmpty(timed);
                Assert.Equal(ruleset, timed[0].Attributes.Ruleset);
            }
        }

        [Fact]
        public void OsuStrains() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/diffcalc-test.osu"))) {
                OsuStrains strains =
                    Assert.IsType<OsuStrains>(
                        new Difficulty { Mods = "HD,FL" }.CalculateStrains(beatmap));
                Assert.Equal(800, strains.StartTime);
                Assert.Equal(400, strains.SectionLength);
                Assert.Equal(254, strains.Flashlight.Length);
                Assert.Equal(41.02690817661656, strains.Flashlight[18]);
            }
        }

        [Fact]
        public void StrainSeriesByRuleset() {
            AssertStrainSeries<TaikoStrains>("taiko/diffcalc-test.osu", Ruleset.Taiko,
                                             s => s.SingleColourStamina);
            AssertStrainSeries<CatchStrains>("fruits/diffcalc-test.osu", Ruleset.Catch, s => s.Movement);
            AssertStrainSeries<ManiaStrains>("mania/diffcalc-test.osu", Ruleset.Mania, s => s.Strain);
        }

        [Fact]
        public void EmptyStrains() {
            string text = "osu file format v14\n[General]\nMode: 0\n";
            using (Beatmap beatmap = Beatmap.FromText(text)) {
                OsuStrains strains = Assert.IsType<OsuStrains>(new Difficulty().CalculateStrains(beatmap));
                Assert.Empty(strains.Aim);
                Assert.Empty(strains.Speed);
            }
        }

        private static void AssertStrainSeries<T>(string map, Ruleset ruleset, Func<T, double[]> selector)
            where T : Strains {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath(map))) {
                Strains strains = new Difficulty { Ruleset = ruleset }.CalculateStrains(beatmap);
                T typed = Assert.IsType<T>(strains);
                Assert.Equal(ruleset, typed.Ruleset);
                Assert.NotEmpty(selector(typed));
            }
        }
    }
}
