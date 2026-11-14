using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Pppp
{
    internal enum PpppErrorCode
    {
        Pppp_BufferSizeQuery = -1,
        Pppp_Success = 0,
        Pppp_InvalidArgument = 1,
        Pppp_RulesetUnavailable = 2,
        Pppp_UnexpectedRuleset = 3,
        Pppp_Parse = 4,
        Pppp_Allocation = 5,
        Pppp_NoSliderPathBackend = 6,
        Pppp_SliderPath = 7,
        Pppp_Internal = 8,
        Pppp_Failure = 127,
    }

    internal partial struct PpppBeatmap
    {
    }

    internal partial struct PpppRuleset
    {
    }

    internal partial struct PpppMods
    {
    }

    internal partial struct PpppOsuDifficultyCalculator
    {
    }

    internal partial struct PpppTaikoDifficultyCalculator
    {
    }

    internal partial struct PpppCatchDifficultyCalculator
    {
    }

    internal partial struct PpppManiaDifficultyCalculator
    {
    }

    internal partial struct PpppTimedResult
    {
    }

    internal partial struct PpppStrains
    {
    }

    internal partial struct PpppOsuPerformanceCalculator
    {
    }

    internal partial struct PpppTaikoPerformanceCalculator
    {
    }

    internal partial struct PpppCatchPerformanceCalculator
    {
    }

    internal partial struct PpppManiaPerformanceCalculator
    {
    }

    internal unsafe partial struct PpppNativeBeatmap
    {
        public PpppBeatmap* handle;

        [NativeTypeName("PpppInt32")]
        public int ruleset_id;

        [NativeTypeName("PpppInt64")]
        public long beatmap_id;

        public float approach_rate;

        public float drain_rate;

        public float overall_difficulty;

        public float circle_size;

        public double slider_multiplier;

        public double slider_tick_rate;

        [NativeTypeName("PpppInt32")]
        public int format_version;

        public float stack_leniency;
    }

    internal unsafe partial struct PpppNativeRuleset
    {
        public PpppRuleset* handle;

        [NativeTypeName("PpppInt32")]
        public int ruleset_id;
    }

    internal unsafe partial struct PpppNativeMods
    {
        public PpppMods* handle;
    }

    internal partial struct PpppOptionalInt64
    {
        [NativeTypeName("PpppBool")]
        public int has_value;

        [NativeTypeName("PpppInt64")]
        public long value;
    }

    internal partial struct PpppOptionalDouble
    {
        [NativeTypeName("PpppBool")]
        public int has_value;

        public double value;
    }

    internal unsafe partial struct PpppNativeScoreInfo
    {
        public PpppRuleset* ruleset;

        public PpppBeatmap* beatmap;

        public PpppMods* mods;

        [NativeTypeName("PpppInt32")]
        public int max_combo;

        public double accuracy;

        public PpppOptionalInt64 legacy_total_score;

        [NativeTypeName("PpppInt32")]
        public int count_miss;

        [NativeTypeName("PpppInt32")]
        public int count_meh;

        [NativeTypeName("PpppInt32")]
        public int count_ok;

        [NativeTypeName("PpppInt32")]
        public int count_good;

        [NativeTypeName("PpppInt32")]
        public int count_great;

        [NativeTypeName("PpppInt32")]
        public int count_perfect;

        [NativeTypeName("PpppInt32")]
        public int count_small_tick_miss;

        [NativeTypeName("PpppInt32")]
        public int count_small_tick_hit;

        [NativeTypeName("PpppInt32")]
        public int count_large_tick_miss;

        [NativeTypeName("PpppInt32")]
        public int count_large_tick_hit;

        [NativeTypeName("PpppInt32")]
        public int count_slider_tail_hit;
    }

    internal partial struct PpppOsuDifficultyAttributes
    {
        public double star_rating;

        [NativeTypeName("PpppInt32")]
        public int max_combo;

        public double aim_difficulty;

        public double speed_difficulty;

        public double reading_difficulty;

        public double flashlight_difficulty;

        public double slider_factor;

        public double aim_difficult_strain_count;

        public double speed_difficult_strain_count;

        public double reading_difficult_note_count;

        public double aim_difficult_slider_count;

        public double aim_top_weighted_slider_factor;

        public double speed_top_weighted_slider_factor;

        public double speed_note_count;

        [NativeTypeName("PpppInt32")]
        public int hit_circle_count;

        [NativeTypeName("PpppInt32")]
        public int slider_count;

        [NativeTypeName("PpppInt32")]
        public int large_tick_count;

        [NativeTypeName("PpppInt32")]
        public int spinner_count;

        public double nested_score_per_object;

        public double legacy_score_base_multiplier;

        public double maximum_legacy_combo_score;
    }

    internal partial struct PpppTaikoDifficultyAttributes
    {
        public double star_rating;

        [NativeTypeName("PpppInt32")]
        public int max_combo;

        public double mechanical_difficulty;

        public double rhythm_difficulty;

        public double reading_difficulty;

        public double colour_difficulty;

        public double stamina_difficulty;

        public double mono_stamina_factor;

        public double consistency_factor;

        public double stamina_top_strains;
    }

    internal partial struct PpppCatchDifficultyAttributes
    {
        public double star_rating;

        [NativeTypeName("PpppInt32")]
        public int max_combo;
    }

    internal partial struct PpppManiaDifficultyAttributes
    {
        public double star_rating;

        [NativeTypeName("PpppInt32")]
        public int max_combo;
    }

    internal partial struct PpppOsuPerformanceAttributes
    {
        public double total;

        public double aim;

        public double speed;

        public double accuracy;

        public double flashlight;

        public double reading;

        public double effective_miss_count;

        public double combo_based_estimated_miss_count;

        public PpppOptionalDouble score_based_estimated_miss_count;

        public double aim_estimated_slider_breaks;

        public double speed_estimated_slider_breaks;

        public PpppOptionalDouble speed_deviation;
    }

    internal partial struct PpppTaikoPerformanceAttributes
    {
        public double total;

        public double difficulty;

        public double accuracy;

        public PpppOptionalDouble estimated_unstable_rate;
    }

    internal partial struct PpppCatchPerformanceAttributes
    {
        public double total;
    }

    internal partial struct PpppManiaPerformanceAttributes
    {
        public double total;

        public double difficulty;
    }

    internal partial struct PpppTimedOsuDifficultyAttributes
    {
        public double time;

        public PpppOsuDifficultyAttributes attributes;
    }

    internal partial struct PpppTimedTaikoDifficultyAttributes
    {
        public double time;

        public PpppTaikoDifficultyAttributes attributes;
    }

    internal partial struct PpppTimedCatchDifficultyAttributes
    {
        public double time;

        public PpppCatchDifficultyAttributes attributes;
    }

    internal partial struct PpppTimedManiaDifficultyAttributes
    {
        public double time;

        public PpppManiaDifficultyAttributes attributes;
    }

    internal unsafe partial struct PpppNativeOsuDifficultyCalculator
    {
        public PpppOsuDifficultyCalculator* handle;
    }

    internal unsafe partial struct PpppNativeTaikoDifficultyCalculator
    {
        public PpppTaikoDifficultyCalculator* handle;
    }

    internal unsafe partial struct PpppNativeCatchDifficultyCalculator
    {
        public PpppCatchDifficultyCalculator* handle;
    }

    internal unsafe partial struct PpppNativeManiaDifficultyCalculator
    {
        public PpppManiaDifficultyCalculator* handle;
    }

    internal unsafe partial struct PpppNativeOsuPerformanceCalculator
    {
        public PpppOsuPerformanceCalculator* handle;
    }

    internal unsafe partial struct PpppNativeTaikoPerformanceCalculator
    {
        public PpppTaikoPerformanceCalculator* handle;
    }

    internal unsafe partial struct PpppNativeCatchPerformanceCalculator
    {
        public PpppCatchPerformanceCalculator* handle;
    }

    internal unsafe partial struct PpppNativeManiaPerformanceCalculator
    {
        public PpppManiaPerformanceCalculator* handle;
    }

    internal static unsafe partial class Native
    {
        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("const char *")]
        public static extern sbyte* Pppp_Version();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("PpppUint32")]
        public static extern uint Pppp_VersionNumber();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("const char *")]
        public static extern sbyte* Pppp_Backend();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("const char *")]
        public static extern sbyte* Pppp_ErrorHandler_GetLastMessage();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Beatmap_CreateFromFile([NativeTypeName("const char *")] sbyte* path, PpppNativeBeatmap* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Beatmap_CreateFromText([NativeTypeName("const char *")] sbyte* text, PpppNativeBeatmap* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Beatmap_CreateFromBytes([NativeTypeName("const void *")] void* data, [NativeTypeName("size_t")] UIntPtr size, PpppNativeBeatmap* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Beatmap_GetTitle(PpppBeatmap* beatmap, [NativeTypeName("char *")] sbyte* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Beatmap_GetArtist(PpppBeatmap* beatmap, [NativeTypeName("char *")] sbyte* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Beatmap_GetVersion(PpppBeatmap* beatmap, [NativeTypeName("char *")] sbyte* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_Beatmap_Destroy(PpppBeatmap* beatmap);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Ruleset_CreateFromId([NativeTypeName("PpppInt32")] int id, PpppNativeRuleset* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Ruleset_CreateFromShortName([NativeTypeName("const char *")] sbyte* name, PpppNativeRuleset* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Ruleset_GetShortName(PpppRuleset* ruleset, [NativeTypeName("char *")] sbyte* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_Ruleset_Destroy(PpppRuleset* ruleset);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_Create(PpppNativeMods* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_Parse(PpppMods* mods, [NativeTypeName("const char *")] sbyte* specification);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_FromLegacy(PpppMods* mods, [NativeTypeName("PpppUint32")] uint bits, [NativeTypeName("PpppInt32")] int ruleset);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_AddClassic(PpppMods* mods);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_Contains(PpppMods* mods, [NativeTypeName("const char *")] sbyte* acronym, [NativeTypeName("PpppBool *")] int* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_GetCount(PpppMods* mods, [NativeTypeName("PpppInt32 *")] int* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_GetAcronym(PpppMods* mods, [NativeTypeName("PpppInt32")] int index, [NativeTypeName("char *")] sbyte* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_GetClockRate(PpppMods* mods, double* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Mods_GetLegacyBits(PpppMods* mods, [NativeTypeName("PpppInt32")] int ruleset, [NativeTypeName("PpppUint32 *")] uint* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_Mods_Destroy(PpppMods* mods);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap, PpppNativeOsuDifficultyCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuDifficultyCalculator_Calculate(PpppOsuDifficultyCalculator* calculator, PpppMods* mods, PpppOsuDifficultyAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuDifficultyCalculator_CalculateTimed(PpppOsuDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuDifficultyCalculator_CalculateStrains(PpppOsuDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuDifficultyCalculator_SetClockRate(PpppOsuDifficultyCalculator* calculator, double clock_rate);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuDifficultyCalculator_ClearClockRate(PpppOsuDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_OsuDifficultyCalculator_Destroy(PpppOsuDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap, PpppNativeTaikoDifficultyCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoDifficultyCalculator_Calculate(PpppTaikoDifficultyCalculator* calculator, PpppMods* mods, PpppTaikoDifficultyAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoDifficultyCalculator_CalculateTimed(PpppTaikoDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoDifficultyCalculator_CalculateStrains(PpppTaikoDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoDifficultyCalculator_SetClockRate(PpppTaikoDifficultyCalculator* calculator, double clock_rate);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoDifficultyCalculator_ClearClockRate(PpppTaikoDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_TaikoDifficultyCalculator_Destroy(PpppTaikoDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap, PpppNativeCatchDifficultyCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchDifficultyCalculator_Calculate(PpppCatchDifficultyCalculator* calculator, PpppMods* mods, PpppCatchDifficultyAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchDifficultyCalculator_CalculateTimed(PpppCatchDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchDifficultyCalculator_CalculateStrains(PpppCatchDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchDifficultyCalculator_SetClockRate(PpppCatchDifficultyCalculator* calculator, double clock_rate);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchDifficultyCalculator_ClearClockRate(PpppCatchDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_CatchDifficultyCalculator_Destroy(PpppCatchDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap, PpppNativeManiaDifficultyCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaDifficultyCalculator_Calculate(PpppManiaDifficultyCalculator* calculator, PpppMods* mods, PpppManiaDifficultyAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaDifficultyCalculator_CalculateTimed(PpppManiaDifficultyCalculator* calculator, PpppMods* mods, PpppTimedResult** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaDifficultyCalculator_CalculateStrains(PpppManiaDifficultyCalculator* calculator, PpppMods* mods, PpppStrains** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaDifficultyCalculator_SetClockRate(PpppManiaDifficultyCalculator* calculator, double clock_rate);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaDifficultyCalculator_ClearClockRate(PpppManiaDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_ManiaDifficultyCalculator_Destroy(PpppManiaDifficultyCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuPerformanceCalculator_Create(PpppNativeOsuPerformanceCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_OsuPerformanceCalculator_Calculate(PpppOsuPerformanceCalculator* calculator, [NativeTypeName("const PpppNativeScoreInfo *")] PpppNativeScoreInfo* score, [NativeTypeName("const PpppOsuDifficultyAttributes *")] PpppOsuDifficultyAttributes* difficulty, PpppOsuPerformanceAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_OsuPerformanceCalculator_Destroy(PpppOsuPerformanceCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoPerformanceCalculator_Create(PpppNativeTaikoPerformanceCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TaikoPerformanceCalculator_Calculate(PpppTaikoPerformanceCalculator* calculator, [NativeTypeName("const PpppNativeScoreInfo *")] PpppNativeScoreInfo* score, [NativeTypeName("const PpppTaikoDifficultyAttributes *")] PpppTaikoDifficultyAttributes* difficulty, PpppTaikoPerformanceAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_TaikoPerformanceCalculator_Destroy(PpppTaikoPerformanceCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchPerformanceCalculator_Create(PpppNativeCatchPerformanceCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_CatchPerformanceCalculator_Calculate(PpppCatchPerformanceCalculator* calculator, [NativeTypeName("const PpppNativeScoreInfo *")] PpppNativeScoreInfo* score, [NativeTypeName("const PpppCatchDifficultyAttributes *")] PpppCatchDifficultyAttributes* difficulty, PpppCatchPerformanceAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_CatchPerformanceCalculator_Destroy(PpppCatchPerformanceCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaPerformanceCalculator_Create(PpppNativeManiaPerformanceCalculator* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_ManiaPerformanceCalculator_Calculate(PpppManiaPerformanceCalculator* calculator, [NativeTypeName("const PpppNativeScoreInfo *")] PpppNativeScoreInfo* score, [NativeTypeName("const PpppManiaDifficultyAttributes *")] PpppManiaDifficultyAttributes* difficulty, PpppManiaPerformanceAttributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_ManiaPerformanceCalculator_Destroy(PpppManiaPerformanceCalculator* calculator);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TimedResult_GetCount(PpppTimedResult* result, [NativeTypeName("PpppInt32 *")] int* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TimedResult_GetOsu(PpppTimedResult* result, PpppTimedOsuDifficultyAttributes* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TimedResult_GetTaiko(PpppTimedResult* result, PpppTimedTaikoDifficultyAttributes* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TimedResult_GetCatch(PpppTimedResult* result, PpppTimedCatchDifficultyAttributes* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_TimedResult_GetMania(PpppTimedResult* result, PpppTimedManiaDifficultyAttributes* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_TimedResult_Destroy(PpppTimedResult* result);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetRuleset(PpppStrains* strains, [NativeTypeName("PpppInt32 *")] int* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetStartTime(PpppStrains* strains, double* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetSectionLength(PpppStrains* strains, double* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetOsuAim(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetOsuAimNoSliders(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetOsuSpeed(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetOsuReading(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetOsuFlashlight(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetTaikoColour(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetTaikoReading(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetTaikoRhythm(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetTaikoStamina(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetTaikoSingleColourStamina(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetCatchMovement(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern PpppErrorCode Pppp_Strains_GetManiaStrain(PpppStrains* strains, double* buffer, [NativeTypeName("PpppInt32 *")] int* size);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void Pppp_Strains_Destroy(PpppStrains* strains);
    }
}
