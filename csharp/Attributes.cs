using System.Runtime.InteropServices;

namespace Pppp {
    [StructLayout(LayoutKind.Sequential)]
    public struct OsuDifficultyAttributes {
        public double star_rating;
        public int max_combo;

        /// <summary>The difficulty corresponding to the aim skill.</summary>
        public double aim_difficulty;

        /// <summary>The difficulty corresponding to the speed skill.</summary>
        public double speed_difficulty;

        /// <summary>The difficulty corresponding to the reading skill.</summary>
        public double reading_difficulty;

        /// <summary>The difficulty corresponding to the flashlight skill.</summary>
        public double flashlight_difficulty;

        /// <summary>Describes how much of aim_difficulty is contributed to by hitcircles or sliders.
        /// A value closer to 1.0 indicates most of aim_difficulty is contributed by hitcircles. A
        /// value closer to 0.0 indicates most of aim_difficulty is contributed by sliders.</summary>
        /// <seealso cref="aim_difficulty"/>
        public double slider_factor;

        public double aim_difficult_strain_count;
        public double speed_difficult_strain_count;
        public double reading_difficult_note_count;

        /// <summary>The number of sliders weighted by difficulty.</summary>
        public double aim_difficult_slider_count;

        /// <summary>Describes how much of aim_difficult_strain_count is contributed to by hitcircles
        /// or sliders. A value closer to 0.0 indicates most of aim_difficult_strain_count is
        /// contributed by hitcircles. A value closer to Infinity indicates most of
        /// aim_difficult_strain_count is contributed by sliders.</summary>
        /// <seealso cref="aim_difficult_strain_count"/>
        public double aim_top_weighted_slider_factor;

        /// <summary>Describes how much of speed_difficult_strain_count is contributed to by
        /// hitcircles or sliders. A value closer to 0.0 indicates most of
        /// speed_difficult_strain_count is contributed by hitcircles. A value closer to Infinity
        /// indicates most of speed_difficult_strain_count is contributed by sliders.</summary>
        /// <seealso cref="speed_difficult_strain_count"/>
        public double speed_top_weighted_slider_factor;

        /// <summary>The number of clickable objects weighted by difficulty.</summary>
        /// <seealso cref="speed_difficulty"/>
        public double speed_note_count;

        /// <summary>The number of hitcircles in the beatmap.</summary>
        public int hit_circle_count;

        /// <summary>The number of sliders in the beatmap.</summary>
        public int slider_count;

        public int large_tick_count;

        /// <summary>The number of spinners in the beatmap.</summary>
        public int spinner_count;

        public double nested_score_per_object;
        public double legacy_score_base_multiplier;
        public double maximum_legacy_combo_score;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TaikoDifficultyAttributes {
        public double star_rating;
        public int max_combo;

        /// <summary>
        /// The difficulty corresponding to the mechanical skills in osu!taiko. This includes colour
        /// and stamina combined.
        /// </summary>
        public double mechanical_difficulty;

        /// <summary>The difficulty corresponding to the rhythm skill.</summary>
        public double rhythm_difficulty;

        /// <summary>The difficulty corresponding to the reading skill.</summary>
        public double reading_difficulty;

        /// <summary>The difficulty corresponding to the colour skill.</summary>
        public double colour_difficulty;

        /// <summary>The difficulty corresponding to the stamina skill.</summary>
        public double stamina_difficulty;

        /// <summary>
        /// The ratio of stamina difficulty from mono-color (single colour) streams to total stamina
        /// difficulty.
        /// </summary>
        public double mono_stamina_factor;

        /// <summary>The factor corresponding to the consistency of a map.</summary>
        public double consistency_factor;

        public double stamina_top_strains;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CatchDifficultyAttributes {
        public double star_rating;
        public int max_combo;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ManiaDifficultyAttributes {
        public double star_rating;
        public int max_combo;
    }

    public struct OsuPerformanceAttributes {
        public double total;
        public double aim;
        public double speed;
        public double accuracy;
        public double flashlight;
        public double reading;
        public double effective_miss_count;
        public double combo_based_estimated_miss_count;
        public double? score_based_estimated_miss_count;
        public double aim_estimated_slider_breaks;
        public double speed_estimated_slider_breaks;
        public double? speed_deviation;
    }

    public struct TaikoPerformanceAttributes {
        public double total;
        public double difficulty;
        public double accuracy;
        public double? estimated_unstable_rate;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CatchPerformanceAttributes {
        public double total;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ManiaPerformanceAttributes {
        public double total;
        public double difficulty;
    }

    /// <summary>The four rulesets' difficulty attributes in one value: a tag plus the four mode
    /// structs.</summary>
    public struct DifficultyAttributes {
        public int ruleset;
        public double star_rating;
        public int max_combo;
        public OsuDifficultyAttributes osu;
        public TaikoDifficultyAttributes taiko;
        public CatchDifficultyAttributes fruits;
        public ManiaDifficultyAttributes mania;
    }

    /// <summary>The four rulesets' performance attributes in one value: a tag plus the four mode
    /// structs.</summary>
    public struct PerformanceAttributes {
        public int ruleset;
        public double total;
        public OsuPerformanceAttributes osu;
        public TaikoPerformanceAttributes taiko;
        public CatchPerformanceAttributes fruits;
        public ManiaPerformanceAttributes mania;
    }
}
