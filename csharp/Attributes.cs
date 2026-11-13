namespace Pppp {
    public abstract class DifficultyAttributes {
        public abstract Ruleset Ruleset { get; }

        public abstract double StarRating { get; }

        public abstract int MaxCombo { get; }
    }

    public sealed class OsuDifficultyAttributes : DifficultyAttributes {
        private readonly PpppOsuDifficultyAttributes value;

        internal OsuDifficultyAttributes(PpppOsuDifficultyAttributes value) {
            this.value = value;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        public override double StarRating {
            get { return value.star_rating; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
        }

        public double AimDifficulty {
            get { return value.aim_difficulty; }
        }

        public double SpeedDifficulty {
            get { return value.speed_difficulty; }
        }

        public double ReadingDifficulty {
            get { return value.reading_difficulty; }
        }

        public double FlashlightDifficulty {
            get { return value.flashlight_difficulty; }
        }

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

        public double AimDifficultSliderCount {
            get { return value.aim_difficult_slider_count; }
        }

        public double AimTopWeightedSliderFactor {
            get { return value.aim_top_weighted_slider_factor; }
        }

        public double SpeedTopWeightedSliderFactor {
            get { return value.speed_top_weighted_slider_factor; }
        }

        public double SpeedNoteCount {
            get { return value.speed_note_count; }
        }

        public int HitCircleCount {
            get { return value.hit_circle_count; }
        }

        public int SliderCount {
            get { return value.slider_count; }
        }

        public int LargeTickCount {
            get { return value.large_tick_count; }
        }

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

        internal PpppOsuDifficultyAttributes ToNative() {
            return value;
        }
    }

    public sealed class TaikoDifficultyAttributes : DifficultyAttributes {
        private readonly PpppTaikoDifficultyAttributes value;

        internal TaikoDifficultyAttributes(PpppTaikoDifficultyAttributes value) {
            this.value = value;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        public override double StarRating {
            get { return value.star_rating; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
        }

        public double MechanicalDifficulty {
            get { return value.mechanical_difficulty; }
        }

        public double RhythmDifficulty {
            get { return value.rhythm_difficulty; }
        }

        public double ReadingDifficulty {
            get { return value.reading_difficulty; }
        }

        public double ColourDifficulty {
            get { return value.colour_difficulty; }
        }

        public double StaminaDifficulty {
            get { return value.stamina_difficulty; }
        }

        public double MonoStaminaFactor {
            get { return value.mono_stamina_factor; }
        }

        public double ConsistencyFactor {
            get { return value.consistency_factor; }
        }

        public double StaminaTopStrains {
            get { return value.stamina_top_strains; }
        }

        internal PpppTaikoDifficultyAttributes ToNative() {
            return value;
        }
    }

    public sealed class CatchDifficultyAttributes : DifficultyAttributes {
        private readonly PpppCatchDifficultyAttributes value;

        internal CatchDifficultyAttributes(PpppCatchDifficultyAttributes value) {
            this.value = value;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }

        public override double StarRating {
            get { return value.star_rating; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
        }

        internal PpppCatchDifficultyAttributes ToNative() {
            return value;
        }
    }

    public sealed class ManiaDifficultyAttributes : DifficultyAttributes {
        private readonly PpppManiaDifficultyAttributes value;

        internal ManiaDifficultyAttributes(PpppManiaDifficultyAttributes value) {
            this.value = value;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }

        public override double StarRating {
            get { return value.star_rating; }
        }

        public override int MaxCombo {
            get { return value.max_combo; }
        }

        internal PpppManiaDifficultyAttributes ToNative() {
            return value;
        }
    }

    public sealed class TimedDifficultyAttributes {
        private readonly double time;
        private readonly DifficultyAttributes attributes;

        internal TimedDifficultyAttributes(double time, DifficultyAttributes attributes) {
            this.time = time;
            this.attributes = attributes;
        }

        public double Time {
            get { return time; }
        }

        public DifficultyAttributes Attributes {
            get { return attributes; }
        }
    }

    public abstract class PerformanceAttributes {
        public abstract Ruleset Ruleset { get; }

        public abstract double Total { get; }
    }

    public sealed class OsuPerformanceAttributes : PerformanceAttributes {
        private readonly PpppOsuPerformanceAttributes value;

        internal OsuPerformanceAttributes(PpppOsuPerformanceAttributes value) {
            this.value = value;
        }

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
                return value.score_based_estimated_miss_count.has_value != 0
                           ? (double?)value.score_based_estimated_miss_count.value
                           : null;
            }
        }

        public double AimEstimatedSliderBreaks {
            get { return value.aim_estimated_slider_breaks; }
        }

        public double SpeedEstimatedSliderBreaks {
            get { return value.speed_estimated_slider_breaks; }
        }

        public double? SpeedDeviation {
            get {
                return value.speed_deviation.has_value != 0 ? (double?)value.speed_deviation.value : null;
            }
        }
    }

    public sealed class TaikoPerformanceAttributes : PerformanceAttributes {
        private readonly PpppTaikoPerformanceAttributes value;

        internal TaikoPerformanceAttributes(PpppTaikoPerformanceAttributes value) {
            this.value = value;
        }

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
                return value.estimated_unstable_rate.has_value != 0
                           ? (double?)value.estimated_unstable_rate.value
                           : null;
            }
        }
    }

    public sealed class CatchPerformanceAttributes : PerformanceAttributes {
        private readonly PpppCatchPerformanceAttributes value;

        internal CatchPerformanceAttributes(PpppCatchPerformanceAttributes value) {
            this.value = value;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }

        public override double Total {
            get { return value.total; }
        }
    }

    public sealed class ManiaPerformanceAttributes : PerformanceAttributes {
        private readonly PpppManiaPerformanceAttributes value;

        internal ManiaPerformanceAttributes(PpppManiaPerformanceAttributes value) {
            this.value = value;
        }

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
