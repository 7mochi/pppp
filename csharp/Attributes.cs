namespace Pppp {
    /// <summary>Describes the difficulty of a beatmap, as output by the difficulty calculator.</summary>
    public abstract class DifficultyAttributes {
        internal DifficultyAttributes() {}

        public abstract Ruleset Ruleset { get; }

        /// <summary>The combined star rating of all skills.</summary>
        public abstract double StarRating { get; set; }

        /// <summary>The maximum achievable combo.</summary>
        public abstract int MaxCombo { get; set; }

        internal abstract Native.DifficultyAttributes ToNative();
    }

    public sealed class OsuDifficultyAttributes : DifficultyAttributes {
        private Native.OsuDifficultyAttributes value;

        internal OsuDifficultyAttributes(Native.OsuDifficultyAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        public override double StarRating {
            get { return value.star_rating; }
            set { this.value.star_rating = value; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
            set { this.value.max_combo = value; }
        }

        /// <summary>The difficulty corresponding to the aim skill.</summary>
        public double AimDifficulty {
            get { return value.aim_difficulty; }
            set { this.value.aim_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the speed skill.</summary>
        public double SpeedDifficulty {
            get { return value.speed_difficulty; }
            set { this.value.speed_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the reading skill.</summary>
        public double ReadingDifficulty {
            get { return value.reading_difficulty; }
            set { this.value.reading_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the flashlight skill.</summary>
        public double FlashlightDifficulty {
            get { return value.flashlight_difficulty; }
            set { this.value.flashlight_difficulty = value; }
        }

        /// <summary>Describes how much of <see cref="AimDifficulty"/> is contributed to by hitcircles or
        /// sliders. A value closer to 1.0 indicates most of <see cref="AimDifficulty"/> is contributed by
        /// hitcircles. A value closer to 0.0 indicates most of <see cref="AimDifficulty"/> is contributed
        /// by sliders.</summary>
        public double SliderFactor {
            get { return value.slider_factor; }
        }

        public double AimDifficultStrainCount {
            get { return value.aim_difficult_strain_count; }
        }

        public double SpeedDifficultStrainCount {
            get { return value.speed_difficult_strain_count; }
        }

        public double ReadingDifficultNoteCount {
            get { return value.reading_difficult_note_count; }
        }

        /// <summary>The number of sliders weighted by difficulty.</summary>
        public double AimDifficultSliderCount {
            get { return value.aim_difficult_slider_count; }
        }

        /// <summary>Describes how much of <see cref="AimDifficultStrainCount"/> is contributed to by
        /// hitcircles or sliders. A value closer to 0.0 indicates most of
        /// <see cref="AimDifficultStrainCount"/> is contributed by hitcircles. A value closer to Infinity
        /// indicates most of <see cref="AimDifficultStrainCount"/> is contributed by sliders.</summary>
        public double AimTopWeightedSliderFactor {
            get { return value.aim_top_weighted_slider_factor; }
        }

        /// <summary>Describes how much of <see cref="SpeedDifficultStrainCount"/> is contributed to by
        /// hitcircles or sliders. A value closer to 0.0 indicates most of
        /// <see cref="SpeedDifficultStrainCount"/> is contributed by hitcircles. A value closer to Infinity
        /// indicates most of <see cref="SpeedDifficultStrainCount"/> is contributed by sliders.</summary>
        public double SpeedTopWeightedSliderFactor {
            get { return value.speed_top_weighted_slider_factor; }
        }

        /// <summary>The number of clickable objects weighted by difficulty.</summary>
        /// <seealso cref="SpeedDifficulty"/>
        public double SpeedNoteCount {
            get { return value.speed_note_count; }
        }

        /// <summary>The number of hitcircles in the beatmap.</summary>
        public int HitCircleCount {
            get { return value.hit_circle_count; }
        }

        /// <summary>The number of sliders in the beatmap.</summary>
        public int SliderCount {
            get { return value.slider_count; }
        }

        public int LargeTickCount {
            get { return value.large_tick_count; }
        }

        /// <summary>The number of spinners in the beatmap.</summary>
        public int SpinnerCount {
            get { return value.spinner_count; }
        }

        public double NestedScorePerObject {
            get { return value.nested_score_per_object; }
        }

        public double LegacyScoreBaseMultiplier {
            get { return value.legacy_score_base_multiplier; }
        }

        public double MaximumLegacyComboScore {
            get { return value.maximum_legacy_combo_score; }
        }

        internal override Native.DifficultyAttributes ToNative() {
            Native.DifficultyAttributes native = new Native.DifficultyAttributes();
            native.ruleset = (int)Ruleset.Osu;
            native.star_rating = value.star_rating;
            native.max_combo = value.max_combo;
            native.attributes.osu = value;
            return native;
        }
    }

    public sealed class TaikoDifficultyAttributes : DifficultyAttributes {
        private Native.TaikoDifficultyAttributes value;

        internal TaikoDifficultyAttributes(Native.TaikoDifficultyAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        public override double StarRating {
            get { return value.star_rating; }
            set { this.value.star_rating = value; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
            set { this.value.max_combo = value; }
        }

        /// <summary>
        /// The difficulty corresponding to the mechanical skills in osu!taiko. This includes colour and
        /// stamina combined.
        /// </summary>
        public double MechanicalDifficulty {
            get { return value.mechanical_difficulty; }
            set { this.value.mechanical_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the rhythm skill.</summary>
        public double RhythmDifficulty {
            get { return value.rhythm_difficulty; }
            set { this.value.rhythm_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the reading skill.</summary>
        public double ReadingDifficulty {
            get { return value.reading_difficulty; }
            set { this.value.reading_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the colour skill.</summary>
        public double ColourDifficulty {
            get { return value.colour_difficulty; }
            set { this.value.colour_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the stamina skill.</summary>
        public double StaminaDifficulty {
            get { return value.stamina_difficulty; }
            set { this.value.stamina_difficulty = value; }
        }

        /// <summary>
        /// The ratio of stamina difficulty from mono-color (single colour) streams to total stamina
        /// difficulty.
        /// </summary>
        public double MonoStaminaFactor {
            get { return value.mono_stamina_factor; }
        }

        /// <summary>The factor corresponding to the consistency of a map.</summary>
        public double ConsistencyFactor {
            get { return value.consistency_factor; }
        }

        public double StaminaTopStrains {
            get { return value.stamina_top_strains; }
        }

        internal override Native.DifficultyAttributes ToNative() {
            Native.DifficultyAttributes native = new Native.DifficultyAttributes();
            native.ruleset = (int)Ruleset.Taiko;
            native.star_rating = value.star_rating;
            native.max_combo = value.max_combo;
            native.attributes.taiko = value;
            return native;
        }
    }

    public sealed class CatchDifficultyAttributes : DifficultyAttributes {
        private Native.CatchDifficultyAttributes value;

        internal CatchDifficultyAttributes(Native.CatchDifficultyAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }

        public override double StarRating {
            get { return value.star_rating; }
            set { this.value.star_rating = value; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
            set { this.value.max_combo = value; }
        }

        internal override Native.DifficultyAttributes ToNative() {
            Native.DifficultyAttributes native = new Native.DifficultyAttributes();
            native.ruleset = (int)Ruleset.Catch;
            native.star_rating = value.star_rating;
            native.max_combo = value.max_combo;
            native.attributes.fruits = value;
            return native;
        }
    }

    public sealed class ManiaDifficultyAttributes : DifficultyAttributes {
        private Native.ManiaDifficultyAttributes value;

        internal ManiaDifficultyAttributes(Native.ManiaDifficultyAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }

        public override double StarRating {
            get { return value.star_rating; }
            set { this.value.star_rating = value; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
            set { this.value.max_combo = value; }
        }

        internal override Native.DifficultyAttributes ToNative() {
            Native.DifficultyAttributes native = new Native.DifficultyAttributes();
            native.ruleset = (int)Ruleset.Mania;
            native.star_rating = value.star_rating;
            native.max_combo = value.max_combo;
            native.attributes.mania = value;
            return native;
        }
    }

    /// <summary>Wraps a <see cref="DifficultyAttributes"/> object and adds a time value for which the
    /// attribute is valid. Output by <see cref="Difficulty.CalculateTimed"/>.</summary>
    public sealed class TimedDifficultyAttributes {
        internal TimedDifficultyAttributes(double time, DifficultyAttributes attributes) {
            Time = time;
            Attributes = attributes;
        }

        /// <summary>The non-clock-adjusted time value at which the attributes take effect.</summary>
        public double Time { get; private set; }

        /// <summary>The attributes.</summary>
        public DifficultyAttributes Attributes { get; private set; }
    }

    public abstract class PerformanceAttributes {
        internal PerformanceAttributes() {}

        public abstract Ruleset Ruleset { get; }

        /// <summary>Calculated score performance points.</summary>
        public abstract double Total { get; }
    }

    public sealed class OsuPerformanceAttributes : PerformanceAttributes {
        private readonly Native.OsuPerformanceAttributes value;

        internal OsuPerformanceAttributes(Native.OsuPerformanceAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        public override double Total {
            get { return value.total; }
        }

        public double Aim {
            get { return value.aim; }
        }

        public double Speed {
            get { return value.speed; }
        }

        public double Accuracy {
            get { return value.accuracy; }
        }

        public double Flashlight {
            get { return value.flashlight; }
        }

        public double Reading {
            get { return value.reading; }
        }

        public double EffectiveMissCount {
            get { return value.effective_miss_count; }
        }

        public double ComboBasedEstimatedMissCount {
            get { return value.combo_based_estimated_miss_count; }
        }

        public double? ScoreBasedEstimatedMissCount {
            get {
                return value.has_score_based_estimated_miss_count != 0
                           ? value.score_based_estimated_miss_count
                           : (double?)null;
            }
        }

        public double AimEstimatedSliderBreaks {
            get { return value.aim_estimated_slider_breaks; }
        }

        public double SpeedEstimatedSliderBreaks {
            get { return value.speed_estimated_slider_breaks; }
        }

        public double? SpeedDeviation {
            get { return value.has_speed_deviation != 0 ? value.speed_deviation : (double?)null; }
        }
    }

    public sealed class TaikoPerformanceAttributes : PerformanceAttributes {
        private readonly Native.TaikoPerformanceAttributes value;

        internal TaikoPerformanceAttributes(Native.TaikoPerformanceAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        public override double Total {
            get { return value.total; }
        }

        public double Difficulty {
            get { return value.difficulty; }
        }

        public double Accuracy {
            get { return value.accuracy; }
        }

        public double? EstimatedUnstableRate {
            get {
                return value.has_estimated_unstable_rate != 0 ? value.estimated_unstable_rate : (double?)null;
            }
        }
    }

    public sealed class CatchPerformanceAttributes : PerformanceAttributes {
        private readonly Native.CatchPerformanceAttributes value;

        internal CatchPerformanceAttributes(Native.CatchPerformanceAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }

        public override double Total {
            get { return value.total; }
        }
    }

    public sealed class ManiaPerformanceAttributes : PerformanceAttributes {
        private readonly Native.ManiaPerformanceAttributes value;

        internal ManiaPerformanceAttributes(Native.ManiaPerformanceAttributes value) { this.value = value; }

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }

        public override double Total {
            get { return value.total; }
        }

        public double Difficulty {
            get { return value.difficulty; }
        }
    }
}
