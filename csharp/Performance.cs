using System;
using System.Collections.Generic;

namespace Pppp {
    public sealed unsafe class Performance {
        public Performance() {
            Statistics = new Dictionary<HitResult, int>();
        }

        public Mods Mods { get; set; }

        public int? MaxCombo { get; set; }

        public double? Accuracy { get; set; }

        public int? Misses { get; set; }

        public Dictionary<HitResult, int> Statistics { get; set; }

        public long? LegacyTotalScore { get; set; }

        public PerformanceAttributes Calculate(Beatmap beatmap) {
            return Calculate(beatmap, null);
        }

        public PerformanceAttributes Calculate(Beatmap beatmap, DifficultyAttributes attributes) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }

            Ruleset ruleset = attributes != null ? attributes.Ruleset : (Ruleset)beatmap.Mode;
            using (RulesetHandle rulesetHandle = Native.CreateRuleset(ruleset))
            using (ModsHandle mods = Native.CreateMods(Mods, ruleset)) {
                PpppNativeScoreInfo score = Native.ScoreInfo(rulesetHandle, beatmap, mods, MaxCombo, Accuracy, Misses,
                                                             LegacyTotalScore, Statistics);

                switch (ruleset) {
                case Ruleset.Osu: {
                    PpppOsuDifficultyAttributes difficulty;
                    PpppOsuDifficultyAttributes* difficultyPointer = null;
                    if (attributes != null) {
                        OsuDifficultyAttributes osu = attributes as OsuDifficultyAttributes;
                        if (osu == null) {
                            throw new InvalidOperationException("the attributes are for another ruleset");
                        }
                        difficulty = osu.ToNative();
                        difficultyPointer = &difficulty;
                    }

                    PpppNativeOsuPerformanceCalculator calculator;
                    Native.Check(Native.Pppp_OsuPerformanceCalculator_Create(&calculator));
                    try {
                        PpppOsuPerformanceAttributes output;
                        Native.Check(Native.Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score,
                                                                                    difficultyPointer, &output));
                        GC.KeepAlive(beatmap);
                        return Native.FromNative(output);
                    } finally {
                        Native.Pppp_OsuPerformanceCalculator_Destroy(calculator.handle);
                    }
                }
                case Ruleset.Taiko: {
                    PpppTaikoDifficultyAttributes difficulty;
                    PpppTaikoDifficultyAttributes* difficultyPointer = null;
                    if (attributes != null) {
                        TaikoDifficultyAttributes taiko = attributes as TaikoDifficultyAttributes;
                        if (taiko == null) {
                            throw new InvalidOperationException("the attributes are for another ruleset");
                        }
                        difficulty = taiko.ToNative();
                        difficultyPointer = &difficulty;
                    }

                    PpppNativeTaikoPerformanceCalculator calculator;
                    Native.Check(Native.Pppp_TaikoPerformanceCalculator_Create(&calculator));
                    try {
                        PpppTaikoPerformanceAttributes output;
                        Native.Check(Native.Pppp_TaikoPerformanceCalculator_Calculate(calculator.handle, &score,
                                                                                      difficultyPointer, &output));
                        GC.KeepAlive(beatmap);
                        return Native.FromNative(output);
                    } finally {
                        Native.Pppp_TaikoPerformanceCalculator_Destroy(calculator.handle);
                    }
                }
                case Ruleset.Catch: {
                    PpppCatchDifficultyAttributes difficulty;
                    PpppCatchDifficultyAttributes* difficultyPointer = null;
                    if (attributes != null) {
                        CatchDifficultyAttributes catchAttributes = attributes as CatchDifficultyAttributes;
                        if (catchAttributes == null) {
                            throw new InvalidOperationException("the attributes are for another ruleset");
                        }
                        difficulty = catchAttributes.ToNative();
                        difficultyPointer = &difficulty;
                    }

                    PpppNativeCatchPerformanceCalculator calculator;
                    Native.Check(Native.Pppp_CatchPerformanceCalculator_Create(&calculator));
                    try {
                        PpppCatchPerformanceAttributes output;
                        Native.Check(Native.Pppp_CatchPerformanceCalculator_Calculate(calculator.handle, &score,
                                                                                      difficultyPointer, &output));
                        GC.KeepAlive(beatmap);
                        return Native.FromNative(output);
                    } finally {
                        Native.Pppp_CatchPerformanceCalculator_Destroy(calculator.handle);
                    }
                }
                case Ruleset.Mania: {
                    PpppManiaDifficultyAttributes difficulty;
                    PpppManiaDifficultyAttributes* difficultyPointer = null;
                    if (attributes != null) {
                        ManiaDifficultyAttributes mania = attributes as ManiaDifficultyAttributes;
                        if (mania == null) {
                            throw new InvalidOperationException("the attributes are for another ruleset");
                        }
                        difficulty = mania.ToNative();
                        difficultyPointer = &difficulty;
                    }

                    PpppNativeManiaPerformanceCalculator calculator;
                    Native.Check(Native.Pppp_ManiaPerformanceCalculator_Create(&calculator));
                    try {
                        PpppManiaPerformanceAttributes output;
                        Native.Check(Native.Pppp_ManiaPerformanceCalculator_Calculate(calculator.handle, &score,
                                                                                      difficultyPointer, &output));
                        GC.KeepAlive(beatmap);
                        return Native.FromNative(output);
                    } finally {
                        Native.Pppp_ManiaPerformanceCalculator_Destroy(calculator.handle);
                    }
                }
                default:
                    throw new ArgumentOutOfRangeException("ruleset", "ruleset must be one of 0, 1, 2, 3");
                }
            }
        }
    }
}
