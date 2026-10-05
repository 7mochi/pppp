using System;
using System.Collections.Generic;

namespace Pppp {
    /// <summary>Performance calculator on maps of any mode.</summary>
    public sealed class Performance {
        public Performance() { Statistics = new Dictionary<HitResult, int>(); }

        /// <summary>Specify mods.</summary>
        public Mods Mods { get; set; }

        /// <summary>Specify the max combo of the play.</summary>
        public int? MaxCombo { get; set; }

        /// <summary>Set the accuracy between 0.0 and 1.0.</summary>
        public double? Accuracy { get; set; }

        /// <summary>Specify the amount of misses of the play.</summary>
        public int? Misses { get; set; }

        public Dictionary<HitResult, int> Statistics { get; set; }

        /// <summary>Used to preserve the total score for legacy scores.</summary>
        /// <remarks>Not populated when the score is not a legacy score.</remarks>
        public long? LegacyTotalScore { get; set; }

        /// <summary>Perform the performance calculation for the map's or the attributes' mode.</summary>
        public PerformanceAttributes Calculate(Beatmap beatmap) { return Calculate(beatmap, null); }

        /// <summary>Use the given already-calculated attributes, skipping the difficulty
        /// calculation.</summary>
        public unsafe PerformanceAttributes Calculate(Beatmap beatmap, DifficultyAttributes attributes) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }
            Native.PerformanceOptions options = new Native.PerformanceOptions();
            options.score = Native.ToNative(Statistics, MaxCombo, Accuracy, LegacyTotalScore);
            options.has_score = 1;
            if (Misses.HasValue) {
                options.misses = Misses.Value;
                options.has_misses = 1;
            }
            Native.DifficultyAttributes provided = attributes == null ? new Native.DifficultyAttributes()
                                                                      : attributes.ToNative();
            if (attributes != null) {
                options.difficulty = (IntPtr)(&provided);
                options.has_difficulty = 1;
            }

            options.mods = Native.CreateMods(Mods);
            try {
                Native.PerformanceAttributes result;
                Native.Check(Native.pppp_calculate_performance(beatmap.Handle, ref options, out result));
                return Native.FromNative(result);
            } finally {
                Native.pppp_mods_free(options.mods);
            }
        }
    }
}
