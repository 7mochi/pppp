using System;

namespace Pppp {
    /// <summary>Difficulty calculator on maps of any mode.</summary>
    public sealed class Difficulty {
        /// <summary>Specify mods.</summary>
        public Mods Mods { get; set; }

        /// <summary>Calculate for this ruleset instead of the beatmap's own mode. This is what a
        /// converted beatmap needs.</summary>
        public Ruleset? Ruleset { get; set; }

        /// <summary>Adjust the clock rate used in the calculation.</summary>
        public double? ClockRate { get; set; }

        /// <summary>Perform the difficulty calculation for the beatmap's mode.</summary>
        public DifficultyAttributes Calculate(Beatmap beatmap) {
            Native.DifficultyOptions options = Options(beatmap);
            try {
                Native.DifficultyAttributes attributes;
                Native.Check(Native.pppp_calculate_difficulty(beatmap.Handle, ref options, out attributes));
                return Native.FromNative(attributes);
            } finally {
                Native.pppp_mods_free(options.mods);
            }
        }

        /// <summary>Calculates the difficulty of the beatmap using a specific mod combination and returns a set
        /// of <see cref="TimedDifficultyAttributes"/> representing the difficulty at every relevant time value
        /// in the beatmap.</summary>
        public TimedDifficultyAttributes[] CalculateTimed(Beatmap beatmap) {
            Native.DifficultyOptions options = Options(beatmap);
            IntPtr timed = IntPtr.Zero;
            try {
                Native.Check(Native.pppp_calculate_timed_difficulty(beatmap.Handle, ref options, out timed));
                IntPtr pointer;
                UIntPtr count;
                Native.Check(Native.pppp_timed_difficulty_entries(timed, out pointer, out count));
                Native.TimedDifficultyAttributes[] entries =
                    Native.CopyArray<Native.TimedDifficultyAttributes>(pointer, count);
                TimedDifficultyAttributes[] result = new TimedDifficultyAttributes[entries.Length];
                for (int i = 0; i < entries.Length; i++) {
                    result[i] = new TimedDifficultyAttributes(entries[i].time, Native.FromNative(entries[i].attributes));
                }
                return result;
            } finally {
                Native.pppp_timed_difficulty_free(timed);
                Native.pppp_mods_free(options.mods);
            }
        }

        /// <summary>Perform the difficulty calculation but instead of evaluating the skill strains, return
        /// them as is.
        ///
        /// Suitable to plot the difficulty of a map over time.</summary>
        public Strains CalculateStrains(Beatmap beatmap) {
            Native.DifficultyOptions options = Options(beatmap);
            IntPtr strains = IntPtr.Zero;
            try {
                Native.Check(Native.pppp_calculate_strains(beatmap.Handle, ref options, out strains));
                int ruleset;
                double startTime;
                double sectionLength;
                Native.Check(Native.pppp_strains_info(strains, out ruleset, out startTime, out sectionLength));
                switch ((Pppp.Ruleset)ruleset) {
                case Pppp.Ruleset.Taiko: {
                    Native.TaikoStrains taiko;
                    Native.Check(Native.pppp_strains_taiko(strains, out taiko));
                    return new TaikoStrains(startTime, sectionLength, taiko);
                }
                case Pppp.Ruleset.Catch: {
                    Native.CatchStrains fruits;
                    Native.Check(Native.pppp_strains_catch(strains, out fruits));
                    return new CatchStrains(startTime, sectionLength, fruits);
                }
                case Pppp.Ruleset.Mania: {
                    Native.ManiaStrains mania;
                    Native.Check(Native.pppp_strains_mania(strains, out mania));
                    return new ManiaStrains(startTime, sectionLength, mania);
                }
                default: {
                    Native.OsuStrains osu;
                    Native.Check(Native.pppp_strains_osu(strains, out osu));
                    return new OsuStrains(startTime, sectionLength, osu);
                }
                }
            } finally {
                Native.pppp_strains_free(strains);
                Native.pppp_mods_free(options.mods);
            }
        }

        private Native.DifficultyOptions Options(Beatmap beatmap) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }
            Native.DifficultyOptions options = new Native.DifficultyOptions();
            if (Ruleset.HasValue) {
                if ((int)Ruleset.Value < 0 || (int)Ruleset.Value > 3) {
                    throw new ArgumentOutOfRangeException("Ruleset", "ruleset must be one of 0, 1, 2, 3");
                }
                options.ruleset = (int)Ruleset.Value;
                options.has_ruleset = 1;
            }
            if (ClockRate.HasValue) {
                options.clock_rate = ClockRate.Value;
                options.has_clock_rate = 1;
            }
            options.mods = Native.CreateMods(Mods);
            return options;
        }
    }
}
