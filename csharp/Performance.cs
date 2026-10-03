using System;

namespace Pppp {
    /// <summary>Performance calculator on maps of any mode.</summary>
    public sealed class Performance {
        private readonly Beatmap beatmap;
        private string mods = "";
        private ScoreInfo state;
        private int? combo;
        private double? accuracy;
        private int? misses;

        public Performance(Beatmap beatmap) {
            if (beatmap == null) {
                throw new ArgumentNullException("beatmap");
            }
            this.beatmap = beatmap;
        }

        /// <summary>Specify mods.</summary>
        public Performance Mods(string specification) {
            mods = specification == null ? "" : specification;
            return this;
        }

        /// <summary>Provide the score state through a ScoreInfo.</summary>
        public Performance State(ScoreInfo score) {
            state = score;
            return this;
        }

        /// <summary>Specify the max combo of the play.</summary>
        public Performance Combo(int value) {
            combo = value;
            return this;
        }

        /// <summary>Set the accuracy between 0.0 and 1.0.</summary>
        public Performance Accuracy(double value) {
            accuracy = value;
            return this;
        }

        /// <summary>Specify the amount of misses of the play.</summary>
        public Performance Misses(int value) {
            misses = value;
            return this;
        }

        /// <summary>Perform the performance calculation for the map's mode.</summary>
        public unsafe PerformanceAttributes Calculate() {
            Native.PerformanceOptions options = new Native.PerformanceOptions();
            fixed (byte* specification = Native.Utf8(mods)) {
                options.mods = (IntPtr)specification;
                if (state != null) {
                    options.score = Native.ToNative(state);
                    options.has_score = 1;
                }
                if (combo.HasValue) {
                    options.combo = combo.Value;
                    options.has_combo = 1;
                }
                if (accuracy.HasValue) {
                    options.accuracy = accuracy.Value;
                    options.has_accuracy = 1;
                }
                if (misses.HasValue) {
                    options.misses = misses.Value;
                    options.has_misses = 1;
                }

                Native.PerformanceAttributes attributes;
                int status =
                    Native.pppp_calculate_performance(beatmap.Handle, ref options, out attributes);
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
