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
        public TimedDifficultyAttributes[] CalculateTimed(Beatmap beatmap) {
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

                        PpppTimedResult* result = null;
                        try {
                            Native.Check(Native.Pppp_OsuDifficultyCalculator_CalculateTimed(calculator.handle,
                                                                                            modsPointer, &result));
                            PpppTimedOsuDifficultyAttributes[] entries = Native.ReadTimedOsu(result);
                            TimedDifficultyAttributes[] timed = new TimedDifficultyAttributes[entries.Length];
                            for (int i = 0; i < entries.Length; i++) {
                                timed[i] = new TimedDifficultyAttributes(entries[i].time,
                                                                         Native.FromNative(entries[i].attributes));
                            }

                            return timed;
                        } finally {
                            if (result != null) {
                                Native.Pppp_TimedResult_Destroy(result);
                            }
                        }
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

                        PpppTimedResult* result = null;
                        try {
                            Native.Check(Native.Pppp_TaikoDifficultyCalculator_CalculateTimed(calculator.handle,
                                                                                              modsPointer, &result));
                            PpppTimedTaikoDifficultyAttributes[] entries = Native.ReadTimedTaiko(result);
                            TimedDifficultyAttributes[] timed = new TimedDifficultyAttributes[entries.Length];
                            for (int i = 0; i < entries.Length; i++) {
                                timed[i] = new TimedDifficultyAttributes(entries[i].time,
                                                                         Native.FromNative(entries[i].attributes));
                            }

                            return timed;
                        } finally {
                            if (result != null) {
                                Native.Pppp_TimedResult_Destroy(result);
                            }
                        }
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

                        PpppTimedResult* result = null;
                        try {
                            Native.Check(Native.Pppp_CatchDifficultyCalculator_CalculateTimed(calculator.handle,
                                                                                              modsPointer, &result));
                            PpppTimedCatchDifficultyAttributes[] entries = Native.ReadTimedCatch(result);
                            TimedDifficultyAttributes[] timed = new TimedDifficultyAttributes[entries.Length];
                            for (int i = 0; i < entries.Length; i++) {
                                timed[i] = new TimedDifficultyAttributes(entries[i].time,
                                                                         Native.FromNative(entries[i].attributes));
                            }

                            return timed;
                        } finally {
                            if (result != null) {
                                Native.Pppp_TimedResult_Destroy(result);
                            }
                        }
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

                        PpppTimedResult* result = null;
                        try {
                            Native.Check(Native.Pppp_ManiaDifficultyCalculator_CalculateTimed(calculator.handle,
                                                                                              modsPointer, &result));
                            PpppTimedManiaDifficultyAttributes[] entries = Native.ReadTimedMania(result);
                            TimedDifficultyAttributes[] timed = new TimedDifficultyAttributes[entries.Length];
                            for (int i = 0; i < entries.Length; i++) {
                                timed[i] = new TimedDifficultyAttributes(entries[i].time,
                                                                         Native.FromNative(entries[i].attributes));
                            }
                            return timed;
                        } finally {
                            if (result != null) {
                                Native.Pppp_TimedResult_Destroy(result);
                            }
                        }
                    } finally {
                        Native.Pppp_ManiaDifficultyCalculator_Destroy(calculator.handle);
                    }
                }
                default:
                    throw new ArgumentOutOfRangeException("ruleset", "ruleset must be one of 0, 1, 2, 3");
                }
            }
        }

        public Strains CalculateStrains(Beatmap beatmap) {
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

                        PpppStrains* result = null;
                        try {
                            Native.Check(Native.Pppp_OsuDifficultyCalculator_CalculateStrains(calculator.handle,
                                                                                              modsPointer, &result));

                            return OsuStrainsFrom(result);
                        } finally {
                            if (result != null) {
                                Native.Pppp_Strains_Destroy(result);
                            }
                        }
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

                        PpppStrains* result = null;
                        try {
                            Native.Check(Native.Pppp_TaikoDifficultyCalculator_CalculateStrains(calculator.handle,
                                                                                                modsPointer, &result));

                            return TaikoStrainsFrom(result);
                        } finally {
                            if (result != null) {
                                Native.Pppp_Strains_Destroy(result);
                            }
                        }
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

                        PpppStrains* result = null;
                        try {
                            Native.Check(Native.Pppp_CatchDifficultyCalculator_CalculateStrains(calculator.handle,
                                                                                                modsPointer, &result));

                            return CatchStrainsFrom(result);
                        } finally {
                            if (result != null) {
                                Native.Pppp_Strains_Destroy(result);
                            }
                        }
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

                        PpppStrains* result = null;
                        try {
                            Native.Check(Native.Pppp_ManiaDifficultyCalculator_CalculateStrains(calculator.handle,
                                                                                                modsPointer, &result));

                            return ManiaStrainsFrom(result);
                        } finally {
                            if (result != null) {
                                Native.Pppp_Strains_Destroy(result);
                            }
                        }
                    } finally {
                        Native.Pppp_ManiaDifficultyCalculator_Destroy(calculator.handle);
                    }
                }
                default:
                    throw new ArgumentOutOfRangeException("ruleset", "ruleset must be one of 0, 1, 2, 3");
                }
            }
        }

        private static OsuStrains OsuStrainsFrom(PpppStrains* result) {
            double startTime;
            double sectionLength;
            Native.Check(Native.Pppp_Strains_GetStartTime(result, &startTime));
            Native.Check(Native.Pppp_Strains_GetSectionLength(result, &sectionLength));

            return new OsuStrains(startTime, sectionLength,
                                  Native.ReadDoubles(Native.Pppp_Strains_GetOsuAim, result),
                                  Native.ReadDoubles(Native.Pppp_Strains_GetOsuAimNoSliders, result),
                                  Native.ReadDoubles(Native.Pppp_Strains_GetOsuSpeed, result),
                                  Native.ReadDoubles(Native.Pppp_Strains_GetOsuReading, result),
                                  Native.ReadDoubles(Native.Pppp_Strains_GetOsuFlashlight, result));
        }

        private static TaikoStrains TaikoStrainsFrom(PpppStrains* result) {
            double startTime;
            double sectionLength;
            Native.Check(Native.Pppp_Strains_GetStartTime(result, &startTime));
            Native.Check(Native.Pppp_Strains_GetSectionLength(result, &sectionLength));

            return new TaikoStrains(startTime, sectionLength,
                                    Native.ReadDoubles(Native.Pppp_Strains_GetTaikoColour, result),
                                    Native.ReadDoubles(Native.Pppp_Strains_GetTaikoReading, result),
                                    Native.ReadDoubles(Native.Pppp_Strains_GetTaikoRhythm, result),
                                    Native.ReadDoubles(Native.Pppp_Strains_GetTaikoStamina, result),
                                    Native.ReadDoubles(Native.Pppp_Strains_GetTaikoSingleColourStamina, result));
        }

        private static CatchStrains CatchStrainsFrom(PpppStrains* result) {
            double startTime;
            double sectionLength;
            Native.Check(Native.Pppp_Strains_GetStartTime(result, &startTime));
            Native.Check(Native.Pppp_Strains_GetSectionLength(result, &sectionLength));

            return new CatchStrains(startTime, sectionLength,
                                    Native.ReadDoubles(Native.Pppp_Strains_GetCatchMovement, result));
        }

        private static ManiaStrains ManiaStrainsFrom(PpppStrains* result) {
            double startTime;
            double sectionLength;
            Native.Check(Native.Pppp_Strains_GetStartTime(result, &startTime));
            Native.Check(Native.Pppp_Strains_GetSectionLength(result, &sectionLength));

            return new ManiaStrains(startTime, sectionLength,
                                    Native.ReadDoubles(Native.Pppp_Strains_GetManiaStrain, result));
        }

    }
}
