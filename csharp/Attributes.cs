
namespace Pppp {
    /// <summary>Base class for difficulty attributes, as output by the difficulty calculator.</summary>
    public abstract class DifficultyAttributes {
        internal pppp_difficulty_attributes value;

        internal DifficultyAttributes(pppp_difficulty_attributes value) { this.value = value; }

        public abstract Ruleset Ruleset { get; }

        /// <summary>The combined star rating of all skills.</summary>
        public double StarRating {
            get { return value.star_rating; }
            set { this.value.star_rating = value; }
        }

        /// <summary>The maximum achievable combo.</summary>
        public int MaxCombo {
            get { return value.max_combo; }
            set { this.value.max_combo = value; }
        }

        internal pppp_difficulty_attributes ToNative() { return value; }
    }

    /// <summary>Difficulty attributes for osu!standard mode.</summary>
    public sealed class OsuDifficultyAttributes : DifficultyAttributes {
        internal OsuDifficultyAttributes(pppp_difficulty_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        /// <summary>The difficulty corresponding to the aim skill.</summary>
        public double AimDifficulty {
            get { return value.osu.aim_difficulty; }
            set { this.value.osu.aim_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the speed skill.</summary>
        public double SpeedDifficulty {
            get { return value.osu.speed_difficulty; }
            set { this.value.osu.speed_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the reading skill.</summary>
        public double ReadingDifficulty {
            get { return value.osu.reading_difficulty; }
            set { this.value.osu.reading_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the flashlight skill.</summary>
        public double FlashlightDifficulty {
            get { return value.osu.flashlight_difficulty; }
            set { this.value.osu.flashlight_difficulty = value; }
        }

        /// <summary>
        /// How much of the aim difficulty is contributed by hitcircles or sliders. A value closer to 1.0
        /// indicates mostly hitcircles; a value closer to 0.0 indicates mostly sliders.
        /// </summary>
        public double SliderFactor {
            get { return value.osu.slider_factor; }
            set { this.value.osu.slider_factor = value; }
        }

        /// <summary>The number of difficult aim strains.</summary>
        public double AimDifficultStrainCount {
            get { return value.osu.aim_difficult_strain_count; }
            set { this.value.osu.aim_difficult_strain_count = value; }
        }

        /// <summary>The number of difficult speed strains.</summary>
        public double SpeedDifficultStrainCount {
            get { return value.osu.speed_difficult_strain_count; }
            set { this.value.osu.speed_difficult_strain_count = value; }
        }

        /// <summary>The number of difficult reading notes.</summary>
        public double ReadingDifficultNoteCount {
            get { return value.osu.reading_difficult_note_count; }
            set { this.value.osu.reading_difficult_note_count = value; }
        }

        /// <summary>The number of sliders weighted by difficulty.</summary>
        public double AimDifficultSliderCount {
            get { return value.osu.aim_difficult_slider_count; }
            set { this.value.osu.aim_difficult_slider_count = value; }
        }

        /// <summary>How much of the aim difficult strain count is contributed by hitcircles or sliders.</summary>
        public double AimTopWeightedSliderFactor {
            get { return value.osu.aim_top_weighted_slider_factor; }
            set { this.value.osu.aim_top_weighted_slider_factor = value; }
        }

        /// <summary>How much of the speed difficult strain count is contributed by hitcircles or sliders.</summary>
        public double SpeedTopWeightedSliderFactor {
            get { return value.osu.speed_top_weighted_slider_factor; }
            set { this.value.osu.speed_top_weighted_slider_factor = value; }
        }

        /// <summary>The number of clickable objects weighted by difficulty.</summary>
        public double SpeedNoteCount {
            get { return value.osu.speed_note_count; }
            set { this.value.osu.speed_note_count = value; }
        }

        /// <summary>The number of hitcircles in the beatmap.</summary>
        public int HitCircleCount {
            get { return value.osu.hit_circle_count; }
            set { this.value.osu.hit_circle_count = value; }
        }

        /// <summary>The number of sliders in the beatmap.</summary>
        public int SliderCount {
            get { return value.osu.slider_count; }
            set { this.value.osu.slider_count = value; }
        }

        /// <summary>The number of large ticks in the beatmap.</summary>
        public int LargeTickCount {
            get { return value.osu.large_tick_count; }
            set { this.value.osu.large_tick_count = value; }
        }

        /// <summary>The number of spinners in the beatmap.</summary>
        public int SpinnerCount {
            get { return value.osu.spinner_count; }
            set { this.value.osu.spinner_count = value; }
        }

        /// <summary>The nested score per object.</summary>
        public double NestedScorePerObject {
            get { return value.osu.nested_score_per_object; }
            set { this.value.osu.nested_score_per_object = value; }
        }

        /// <summary>The base multiplier for legacy scoring.</summary>
        public double LegacyScoreBaseMultiplier {
            get { return value.osu.legacy_score_base_multiplier; }
            set { this.value.osu.legacy_score_base_multiplier = value; }
        }

        /// <summary>The maximum legacy combo score.</summary>
        public double MaximumLegacyComboScore {
            get { return value.osu.maximum_legacy_combo_score; }
            set { this.value.osu.maximum_legacy_combo_score = value; }
        }
    }

    /// <summary>Difficulty attributes for osu!taiko mode.</summary>
    public sealed class TaikoDifficultyAttributes : DifficultyAttributes {
        internal TaikoDifficultyAttributes(pppp_difficulty_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        /// <summary>The difficulty corresponding to the mechanical skills in osu!taiko, combining colour and stamina.</summary>
        public double MechanicalDifficulty {
            get { return value.taiko.mechanical_difficulty; }
            set { this.value.taiko.mechanical_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the rhythm skill.</summary>
        public double RhythmDifficulty {
            get { return value.taiko.rhythm_difficulty; }
            set { this.value.taiko.rhythm_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the reading skill.</summary>
        public double ReadingDifficulty {
            get { return value.taiko.reading_difficulty; }
            set { this.value.taiko.reading_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the colour skill.</summary>
        public double ColourDifficulty {
            get { return value.taiko.colour_difficulty; }
            set { this.value.taiko.colour_difficulty = value; }
        }

        /// <summary>The difficulty corresponding to the stamina skill.</summary>
        public double StaminaDifficulty {
            get { return value.taiko.stamina_difficulty; }
            set { this.value.taiko.stamina_difficulty = value; }
        }

        /// <summary>The ratio of stamina difficulty from single-colour streams to total stamina difficulty.</summary>
        public double MonoStaminaFactor {
            get { return value.taiko.mono_stamina_factor; }
            set { this.value.taiko.mono_stamina_factor = value; }
        }

        /// <summary>The factor corresponding to the consistency of a map.</summary>
        public double ConsistencyFactor {
            get { return value.taiko.consistency_factor; }
            set { this.value.taiko.consistency_factor = value; }
        }

        /// <summary>The top stamina strains.</summary>
        public double StaminaTopStrains {
            get { return value.taiko.stamina_top_strains; }
            set { this.value.taiko.stamina_top_strains = value; }
        }
    }

    /// <summary>Difficulty attributes for osu!catch mode. This class has no additional attributes beyond the base class.</summary>
    public sealed class CatchDifficultyAttributes : DifficultyAttributes {
        internal CatchDifficultyAttributes(pppp_difficulty_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }
    }

    /// <summary>Difficulty attributes for osu!mania mode. This class has no additional attributes beyond the base class.</summary>
    public sealed class ManiaDifficultyAttributes : DifficultyAttributes {
        internal ManiaDifficultyAttributes(pppp_difficulty_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }
    }

    /// <summary>
    /// Wraps a `pppp_difficulty_attributes` and adds a time value for which the attribute is valid.
    /// Output by `pppp_calculate_timed_difficulty`.
    /// </summary>
    public sealed class TimedDifficultyAttributes {
        internal TimedDifficultyAttributes(double time, DifficultyAttributes attributes) {
            Time = time;
            Attributes = attributes;
        }

        public double Time { get; private set; }
        public DifficultyAttributes Attributes { get; private set; }
    }

    /// <summary>Base class for performance attributes.</summary>
    public abstract class PerformanceAttributes {
        internal readonly pppp_performance_attributes value;

        internal PerformanceAttributes(pppp_performance_attributes value) { this.value = value; }

        public abstract Ruleset Ruleset { get; }

        /// <summary>The final calculated performance points.</summary>
        public double Total {
            get { return value.total; }
        }
    }

    /// <summary>Performance attributes for osu!standard mode.</summary>
    public sealed class OsuPerformanceAttributes : PerformanceAttributes {
        internal OsuPerformanceAttributes(pppp_performance_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        /// <summary>The aim portion of the final performance points.</summary>
        public double Aim {
            get { return value.osu.aim; }
        }

        /// <summary>The speed portion of the final performance points.</summary>
        public double Speed {
            get { return value.osu.speed; }
        }

        /// <summary>The accuracy portion of the final performance points.</summary>
        public double Accuracy {
            get { return value.osu.accuracy; }
        }

        /// <summary>The flashlight portion of the final performance points.</summary>
        public double Flashlight {
            get { return value.osu.flashlight; }
        }

        /// <summary>The reading portion of the final performance points.</summary>
        public double Reading {
            get { return value.osu.reading; }
        }

        /// <summary>Misses including an approximated amount of slider breaks.</summary>
        public double EffectiveMissCount {
            get { return value.osu.effective_miss_count; }
        }

        /// <summary>Estimated miss count based on the player's combo.</summary>
        public double ComboBasedEstimatedMissCount {
            get { return value.osu.combo_based_estimated_miss_count; }
        }

        /// <summary>Estimated miss count based on the player's score, if available.</summary>
        public double? ScoreBasedEstimatedMissCount {
            get { return value.osu.has_score_based_estimated_miss_count != 0 ? (double?)value.osu.score_based_estimated_miss_count : null; }
        }

        /// <summary>Estimated number of slider breaks that affected the aim portion.</summary>
        public double AimEstimatedSliderBreaks {
            get { return value.osu.aim_estimated_slider_breaks; }
        }

        /// <summary>Estimated number of slider breaks that affected the speed portion.</summary>
        public double SpeedEstimatedSliderBreaks {
            get { return value.osu.speed_estimated_slider_breaks; }
        }

        /// <summary>Approximated unstable rate, if available.</summary>
        public double? SpeedDeviation {
            get { return value.osu.has_speed_deviation != 0 ? (double?)value.osu.speed_deviation : null; }
        }
    }

    /// <summary>Performance attributes for osu!taiko mode.</summary>
    public sealed class TaikoPerformanceAttributes : PerformanceAttributes {
        internal TaikoPerformanceAttributes(pppp_performance_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        /// <summary>The difficulty portion of the final performance points.</summary>
        public double Difficulty {
            get { return value.taiko.difficulty; }
        }

        /// <summary>The accuracy portion of the final performance points.</summary>
        public double Accuracy {
            get { return value.taiko.accuracy; }
        }

        /// <summary>Approximated unstable rate, if available.</summary>
        public double? EstimatedUnstableRate {
            get { return value.taiko.has_estimated_unstable_rate != 0 ? (double?)value.taiko.estimated_unstable_rate : null; }
        }
    }

    /// <summary>Performance attributes for osu!catch mode. This class has no additional attributes beyond the base class.</summary>
    public sealed class CatchPerformanceAttributes : PerformanceAttributes {
        internal CatchPerformanceAttributes(pppp_performance_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }
    }

    /// <summary>Performance attributes for osu!mania mode.</summary>
    public sealed class ManiaPerformanceAttributes : PerformanceAttributes {
        internal ManiaPerformanceAttributes(pppp_performance_attributes value) : base(value) {}

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }

        /// <summary>The difficulty portion of the final performance points.</summary>
        public double Difficulty {
            get { return value.mania.difficulty; }
        }
    }
}
