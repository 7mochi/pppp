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
        public unsafe DifficultyAttributes Calculate(Beatmap beatmap) {
            pppp_difficulty_options options = Options(beatmap);
            try {
                pppp_difficulty_attributes attributes;
                Native.Check(Native.pppp_calculate_difficulty((pppp_beatmap*)beatmap.Handle, &options, &attributes));
                return Native.FromNative(attributes);
            } finally {
                Native.pppp_mods_free(options.mods);
            }
        }

        /// <summary>Calculates the difficulty of the beatmap using a specific mod combination and returns a set
        /// of <see cref="TimedDifficultyAttributes"/> representing the difficulty at every relevant time value
        /// in the beatmap.</summary>
        public unsafe TimedDifficultyAttributes[] CalculateTimed(Beatmap beatmap) {
            pppp_difficulty_options options = Options(beatmap);
            pppp_timed_difficulty* timed = null;
            try {
                Native.Check(Native.pppp_calculate_timed_difficulty((pppp_beatmap*)beatmap.Handle, &options, &timed));
                pppp_timed_difficulty_attributes* pointer;
                UIntPtr count;
                Native.Check(Native.pppp_timed_difficulty_entries(timed, &pointer, &count));
                pppp_timed_difficulty_attributes[] entries =
                    Native.CopyArray<pppp_timed_difficulty_attributes>((IntPtr)pointer, count);
                TimedDifficultyAttributes[] result = new TimedDifficultyAttributes[entries.Length];
                for (int i = 0; i < entries.Length; i++) {
                    result[i] = new TimedDifficultyAttributes(entries[i].time,
                                                              Native.FromNative(entries[i].attributes));
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
        public unsafe Strains CalculateStrains(Beatmap beatmap) {
            pppp_difficulty_options options = Options(beatmap);
            pppp_strains* strains = null;
            try {
                Native.Check(Native.pppp_calculate_strains((pppp_beatmap*)beatmap.Handle, &options, &strains));
                int ruleset;
                double startTime;
                double sectionLength;
                Native.Check(Native.pppp_strains_info(strains, &ruleset, &startTime, &sectionLength));
                switch ((Pppp.Ruleset)ruleset) {
                case Pppp.Ruleset.Taiko: {
                    pppp_taiko_strains taiko;
                    Native.Check(Native.pppp_strains_taiko(strains, &taiko));
                    return new TaikoStrains(startTime, sectionLength, taiko);
                }
                case Pppp.Ruleset.Catch: {
                    pppp_catch_strains fruits;
                    Native.Check(Native.pppp_strains_catch(strains, &fruits));
                    return new CatchStrains(startTime, sectionLength, fruits);
                }
                case Pppp.Ruleset.Mania: {
                    pppp_mania_strains mania;
                    Native.Check(Native.pppp_strains_mania(strains, &mania));
                    return new ManiaStrains(startTime, sectionLength, mania);
                }
                default: {
                    pppp_osu_strains osu;
                    Native.Check(Native.pppp_strains_osu(strains, &osu));
                    return new OsuStrains(startTime, sectionLength, osu);
                }
                }
            } finally {
                Native.pppp_strains_free(strains);
                Native.pppp_mods_free(options.mods);
            }
        }

        private unsafe pppp_difficulty_options Options(Beatmap beatmap) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }
            pppp_difficulty_options options = new pppp_difficulty_options();
            options.version = Native.DifficultyOptionsVersion;
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
            options.mods = (pppp_mods*)Native.CreateMods(Mods,
                                                         Ruleset.HasValue ? (int)Ruleset.Value : beatmap.Mode);
            return options;
        }
    }
}
