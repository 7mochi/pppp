using System;

namespace Pppp {
    /// <summary>Difficulty calculator on maps of any mode.</summary>
    public sealed class Difficulty {
        private readonly Beatmap beatmap;
        private string mods = "";
        private Ruleset? ruleset;
        private double? clock_rate;

        public Difficulty(Beatmap beatmap) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }
            this.beatmap = beatmap;
        }

        /// <summary>Specify mods.</summary>
        public Difficulty Mods(string specification) {
            mods = specification == null ? "" : specification;
            return this;
        }

        /// <summary>Calculate for this ruleset instead of the beatmap's own mode. This is what a
        /// converted beatmap needs.</summary>
        public Difficulty Ruleset(Ruleset value) {
            if ((int)value < 0 || (int)value > 3) {
                throw new ArgumentException("ruleset must be one of 0, 1, 2, 3");
            }
            ruleset = value;
            return this;
        }

        /// <summary>Adjust the clock rate used in the calculation.</summary>
        public Difficulty ClockRate(double rate) {
            clock_rate = rate;
            return this;
        }

        /// <summary>Perform the difficulty calculation for the beatmap's mode.</summary>
        public unsafe DifficultyAttributes Calculate() {
            Native.DifficultyOptions options = new Native.DifficultyOptions();
            fixed (byte* specification = Native.Utf8(mods)) {
                options.mods = (IntPtr)specification;
                if (ruleset.HasValue) {
                    options.ruleset = (int)ruleset.Value;
                    options.has_ruleset = 1;
                }
                if (clock_rate.HasValue) {
                    options.clock_rate = clock_rate.Value;
                    options.has_clock_rate = 1;
                }

                Native.DifficultyAttributes attributes;
                int status = Native.pppp_calculate_difficulty(beatmap.Handle, ref options, out attributes);
                if (status == Native.Allocation) {
                    throw new OutOfMemoryException();
                }
                if (status != Native.Ok) {
                    throw new ArgumentException("invalid mod specification");
                }
                return Native.FromNative(attributes);
            }
        }
    }
}
