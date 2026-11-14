using System;
using Pppp;
using Xunit;

namespace Pppp.Tests {
    public sealed class PerformanceTests {
        [Fact]
        public void OsuPerformance() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                OsuDifficultyAttributes difficulty =
                    Assert.IsType<OsuDifficultyAttributes>(new Difficulty().Calculate(beatmap));
                OsuPerformanceAttributes play = Assert.IsType<OsuPerformanceAttributes>(
                    CreateOsuScore(difficulty).Calculate(beatmap));
                Assert.Equal(316.5901855625614, play.Total);
                Assert.Equal(148.75278891878943, play.Aim);
                Assert.Equal(61.34653468094172, play.Speed);
                Assert.Equal(98.99847982709288, play.Accuracy);
                Assert.Equal(2.2291238201795176, play.Reading);
                Assert.Null(play.ScoreBasedEstimatedMissCount);
            }
        }

        [Fact]
        public void TaikoPerformance() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("taiko/1028484.osu"))) {
                TaikoPerformanceAttributes play = Assert.IsType<TaikoPerformanceAttributes>(
                    new Performance {
                        MaxCombo = 289,
                        Accuracy = 1.0,
                        Statistics = { { HitResult.Great, 289 } },
                    }.Calculate(beatmap));
                Assert.Equal(130.26636361095524, play.Total);
                Assert.Equal(33.48488833057447, play.Difficulty);
                Assert.Equal(96.78147528038076, play.Accuracy);
                Assert.True(play.EstimatedUnstableRate.HasValue);
                Assert.Equal(146.3238357972284, play.EstimatedUnstableRate.Value);
            }
        }

        [Fact]
        public void CatchPerformance() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("fruits/2118524.osu"))) {
                CatchDifficultyAttributes difficulty =
                    Assert.IsType<CatchDifficultyAttributes>(new Difficulty().Calculate(beatmap));
                CatchPerformanceAttributes play = Assert.IsType<CatchPerformanceAttributes>(
                    new Performance {
                        MaxCombo = difficulty.MaxCombo,
                        Accuracy = 1.0,
                        Statistics = {
                            { HitResult.Great, 728 },
                            { HitResult.LargeTickHit, 2 },
                            { HitResult.SmallTickHit, 263 }
                        },
                    }.Calculate(beatmap));
                Assert.Equal(112.72215339177879, play.Total);
            }
        }

        [Fact]
        public void ManiaPerformance() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("mania/1638954.osu"))) {
                ManiaDifficultyAttributes difficulty =
                    Assert.IsType<ManiaDifficultyAttributes>(new Difficulty().Calculate(beatmap));
                ManiaPerformanceAttributes play = Assert.IsType<ManiaPerformanceAttributes>(
                    new Performance {
                        MaxCombo = difficulty.MaxCombo,
                        Accuracy = 1.0,
                        Statistics = { { HitResult.Perfect, 715 } },
                    }.Calculate(beatmap));
                Assert.Equal(108.92297471705167, play.Total);
                Assert.Equal(108.92297471705167, play.Difficulty);
            }
        }

        [Fact]
        public void ClassicModsAndAttributesReuse() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                OsuDifficultyAttributes difficulty =
                    Assert.IsType<OsuDifficultyAttributes>(new Difficulty().Calculate(beatmap));

                Performance score = CreateOsuScore(difficulty);
                Performance classic = CreateOsuScore(difficulty);
                classic.Mods = new Mods().WithClassic();
                Performance fromString = CreateOsuScore(difficulty);
                fromString.Mods = "CL";
                Performance twice = CreateOsuScore(difficulty);
                twice.Mods = "CL,CL";

                double classicTotal = classic.Calculate(beatmap).Total;
                double fromStringTotal = fromString.Calculate(beatmap).Total;
                double twiceTotal = twice.Calculate(beatmap).Total;
                Assert.Equal(298.5325815982227, classicTotal);
                Assert.Equal(classicTotal, fromStringTotal);
                Assert.Equal(fromStringTotal, twiceTotal);
                Assert.Equal(score.Calculate(beatmap, difficulty).Total, score.Calculate(beatmap).Total);
            }
        }

        [Fact]
        public void LegacyTotalScoreIsUsed() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                OsuDifficultyAttributes difficulty =
                    Assert.IsType<OsuDifficultyAttributes>(new Difficulty().Calculate(beatmap));
                OsuPerformanceAttributes play = Assert.IsType<OsuPerformanceAttributes>(
                    new Performance {
                        Mods = "CL",
                        MaxCombo = 800,
                        Accuracy = 0.98,
                        LegacyTotalScore = 1000000,
                        Statistics = {
                            { HitResult.Great, 600 },
                            { HitResult.Ok, 3 },
                            { HitResult.Miss, 2 },
                            { HitResult.SliderTailHit, difficulty.SliderCount }
                        },
                    }.Calculate(beatmap));
                Assert.True(play.ScoreBasedEstimatedMissCount.HasValue);
                Assert.Equal(1.2536127680950426, play.ScoreBasedEstimatedMissCount.Value);
            }
        }

        [Fact]
        public void MissesOverrideStatistics() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                Performance basePlay = new Performance { MaxCombo = 909, Accuracy = 1.0, Misses = 0 };
                Performance overridePlay = new Performance {
                    MaxCombo = 909,
                    Accuracy = 1.0,
                    Misses = 0,
                    Statistics = { { HitResult.Miss, 4 } },
                };
                Assert.Equal(basePlay.Calculate(beatmap).Total, overridePlay.Calculate(beatmap).Total);
            }
        }

        [Fact]
        public void PerformanceErrors() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                Assert.Throws<ArgumentOutOfRangeException>(
                    delegate {
                        new Performance { Statistics = { { (HitResult)99, 1 } } }.Calculate(beatmap);
                    });
                Assert.Throws<ArgumentNullException>(
                    delegate { new Performance().Calculate(null); });
            }

            Beatmap closed = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"));
            closed.Dispose();
            Assert.Throws<ObjectDisposedException>(
                delegate { new Performance().Calculate(closed); });
        }

        private static Performance CreateOsuScore(OsuDifficultyAttributes difficulty) {
            return new Performance {
                MaxCombo = 909,
                Accuracy = 1.0,
                Statistics = {
                    { HitResult.Great, 601 },
                    { HitResult.SliderTailHit, difficulty.SliderCount }
                },
            };
        }
    }
}
