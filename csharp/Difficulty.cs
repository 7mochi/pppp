using System;

namespace Pppp {
    public sealed unsafe class Difficulty {
        public Mods Mods { get; set; }

        public Ruleset? Ruleset { get; set; }

        public double? ClockRate { get; set; }

        public DifficultyAttributes Calculate(Beatmap beatmap) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }

            Ruleset ruleset = Ruleset.HasValue ? Ruleset.Value : (Pppp.Ruleset)beatmap.Mode;
            using (RulesetHandle rulesetHandle = Native.CreateRuleset(ruleset))
            using (ModsHandle mods = Native.CreateMods(Mods, ruleset)) {
                PpppMods* modsPointer = null;
                if (mods != null) {
                    modsPointer = mods.Pointer;
                }

                switch (ruleset) {
                case Pppp.Ruleset.Osu: {
                    PpppNativeOsuDifficultyCalculator calculator;
                    Native.Check(Native.Pppp_OsuDifficultyCalculator_Create(rulesetHandle.Pointer, beatmap.Map,
                                                                            &calculator));
                    GC.KeepAlive(beatmap);
                    try {
                        if (ClockRate.HasValue) {
                            Native.Check(Native.Pppp_OsuDifficultyCalculator_SetClockRate(calculator.handle,
                                                                                          ClockRate.Value));
                        }

                        PpppOsuDifficultyAttributes attributes;
                        Native.Check(Native.Pppp_OsuDifficultyCalculator_Calculate(calculator.handle, modsPointer,
                                                                                   &attributes));

                        return Native.FromNative(attributes);
                    } finally {
                        Native.Pppp_OsuDifficultyCalculator_Destroy(calculator.handle);
                    }
                }
                case Pppp.Ruleset.Taiko: {
                    PpppNativeTaikoDifficultyCalculator calculator;
                    Native.Check(Native.Pppp_TaikoDifficultyCalculator_Create(rulesetHandle.Pointer, beatmap.Map,
                                                                              &calculator));
                    GC.KeepAlive(beatmap);
                    try {
                        if (ClockRate.HasValue) {
                            Native.Check(Native.Pppp_TaikoDifficultyCalculator_SetClockRate(calculator.handle,
                                                                                            ClockRate.Value));
                        }

                        PpppTaikoDifficultyAttributes attributes;
                        Native.Check(Native.Pppp_TaikoDifficultyCalculator_Calculate(calculator.handle, modsPointer,
                                                                                     &attributes));

                        return Native.FromNative(attributes);
                    } finally {
                        Native.Pppp_TaikoDifficultyCalculator_Destroy(calculator.handle);
                    }
                }
                case Pppp.Ruleset.Catch: {
                    PpppNativeCatchDifficultyCalculator calculator;
                    Native.Check(Native.Pppp_CatchDifficultyCalculator_Create(rulesetHandle.Pointer, beatmap.Map,
                                                                              &calculator));
                    GC.KeepAlive(beatmap);
                    try {
                        if (ClockRate.HasValue) {
                            Native.Check(Native.Pppp_CatchDifficultyCalculator_SetClockRate(calculator.handle,
                                                                                            ClockRate.Value));
                        }

                        PpppCatchDifficultyAttributes attributes;
                        Native.Check(Native.Pppp_CatchDifficultyCalculator_Calculate(calculator.handle, modsPointer,
                                                                                     &attributes));

                        return Native.FromNative(attributes);
                    } finally {
                        Native.Pppp_CatchDifficultyCalculator_Destroy(calculator.handle);
                    }
                }
                case Pppp.Ruleset.Mania: {
                    PpppNativeManiaDifficultyCalculator calculator;
                    Native.Check(Native.Pppp_ManiaDifficultyCalculator_Create(rulesetHandle.Pointer, beatmap.Map,
                                                                              &calculator));
                    GC.KeepAlive(beatmap);
                    try {
                        if (ClockRate.HasValue) {
                            Native.Check(Native.Pppp_ManiaDifficultyCalculator_SetClockRate(calculator.handle,
                                                                                            ClockRate.Value));
                        }

                        PpppManiaDifficultyAttributes attributes;
                        Native.Check(Native.Pppp_ManiaDifficultyCalculator_Calculate(calculator.handle, modsPointer,
                                                                                     &attributes));

                        return Native.FromNative(attributes);
                    } finally {
                        Native.Pppp_ManiaDifficultyCalculator_Destroy(calculator.handle);
                    }
                }
                default:
                    throw new ArgumentOutOfRangeException("ruleset", "ruleset must be one of 0, 1, 2, 3");
                }
            }
        }
    }
}
