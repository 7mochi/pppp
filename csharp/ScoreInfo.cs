namespace Pppp {
    public sealed class ScoreInfo {
        public ScoreInfo() {
            statistics = new int[Native.HitResultCount];
            maximum_statistics = new int[Native.HitResultCount];
        }

        public int[] statistics { get; set; }
        public int[] maximum_statistics { get; set; }
        public int max_combo { get; set; }
        public double accuracy { get; set; }

        /// <summary>Used to preserve the total score for legacy scores.</summary>
        /// <remarks>Not populated when the score is not a legacy score.</remarks>
        public long? legacy_total_score { get; set; }
    }
}
