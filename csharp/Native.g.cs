using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Pppp
{
    [NativeTypeName("unsigned int")]
    internal enum pppp_result : uint
    {
        PPPP_OK = 0,
        PPPP_INVALID_ARGUMENT,
        PPPP_ALLOCATION,
        PPPP_PARSE,
        PPPP_NO_SLIDER_PATH_BACKEND,
        PPPP_SLIDER_PATH,
        PPPP_INTERNAL,
        PPPP_NOT_PREPARED,
    }

    [NativeTypeName("unsigned int")]
    internal enum pppp_ruleset : uint
    {
        PPPP_RULESET_OSU = 0,
        PPPP_RULESET_TAIKO,
        PPPP_RULESET_CATCH,
        PPPP_RULESET_MANIA,
    }

    [NativeTypeName("unsigned int")]
    internal enum pppp_slider_event_type : uint
    {
        PPPP_SLIDER_EVENT_TICK = 0,
        PPPP_SLIDER_EVENT_LEGACY_LAST_TICK = 1,
        PPPP_SLIDER_EVENT_HEAD = 2,
        PPPP_SLIDER_EVENT_TAIL = 3,
        PPPP_SLIDER_EVENT_REPEAT = 4,
    }

    internal partial struct pppp_vector2
    {
        public float x;

        public float y;
    }

    internal partial struct pppp_beatmap_difficulty
    {
        public double drain_rate;

        public double circle_size;

        public double overall_difficulty;

        public double approach_rate;

        public double slider_multiplier;

        public double slider_tick_rate;
    }

    internal partial struct pppp_slider_event_descriptor
    {
        [NativeTypeName("pppp_int32")]
        public int type;

        public double time;

        [NativeTypeName("pppp_int32")]
        public int span_index;

        public double span_start_time;

        public double path_progress;

        public pppp_vector2 position;
    }

    internal partial struct pppp_hit_object
    {
        public pppp_vector2 position;

        [NativeTypeName("pppp_uint32")]
        public uint type;

        [NativeTypeName("pppp_uint32")]
        public uint hitsound;

        public double start_time;

        public double end_time;

        [NativeTypeName("pppp_int32")]
        public int new_combo;

        [NativeTypeName("pppp_int32")]
        public int combo_offset;

        [NativeTypeName("pppp_int32")]
        public int slider;
    }

    internal partial struct pppp_timing_point
    {
        public double time;

        public double beat_length;

        [NativeTypeName("pppp_int32")]
        public int meter;

        [NativeTypeName("pppp_int32")]
        public int uninherited;

        [NativeTypeName("pppp_uint32")]
        public uint effects;
    }

    internal partial struct pppp_break_period
    {
        public double start_time;

        public double end_time;
    }

    internal unsafe partial struct pppp_slider
    {
        [NativeTypeName("pppp_int32")]
        public int slides;

        public double expected_length;

        [NativeTypeName("const pppp_uint32 *")]
        public uint* node_sounds;

        [NativeTypeName("size_t")]
        public UIntPtr node_sound_count;

        [NativeTypeName("const pppp_vector2 *")]
        public pppp_vector2* control_points;

        [NativeTypeName("size_t")]
        public UIntPtr control_point_count;

        [NativeTypeName("const pppp_vector2 *")]
        public pppp_vector2* path;

        [NativeTypeName("size_t")]
        public UIntPtr path_count;

        [NativeTypeName("const double *")]
        public double* cumulative_lengths;

        [NativeTypeName("size_t")]
        public UIntPtr cumulative_length_count;

        [NativeTypeName("const pppp_vector2 *")]
        public pppp_vector2* undecimated_path;

        [NativeTypeName("size_t")]
        public UIntPtr undecimated_path_count;

        [NativeTypeName("const double *")]
        public double* undecimated_cumulative_lengths;

        [NativeTypeName("size_t")]
        public UIntPtr undecimated_cumulative_length_count;

        [NativeTypeName("const pppp_slider_event_descriptor *")]
        public pppp_slider_event_descriptor* events;

        [NativeTypeName("size_t")]
        public UIntPtr event_count;

        [NativeTypeName("const pppp_slider_event_descriptor *")]
        public pppp_slider_event_descriptor* catch_events;

        [NativeTypeName("size_t")]
        public UIntPtr catch_event_count;
    }

    internal unsafe partial struct pppp_score_info
    {
        [NativeTypeName("pppp_int32[18]")]
        public fixed int statistics[18];

        [NativeTypeName("pppp_int32[18]")]
        public fixed int maximum_statistics[18];

        [NativeTypeName("pppp_int32")]
        public int max_combo;

        public double accuracy;

        [NativeTypeName("pppp_int32")]
        public int has_legacy_total_score;

        [NativeTypeName("pppp_int64")]
        public long legacy_total_score;
    }

    internal partial struct pppp_osu_difficulty_attributes
    {
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

        [NativeTypeName("pppp_int32")]
        public int hit_circle_count;

        [NativeTypeName("pppp_int32")]
        public int slider_count;

        [NativeTypeName("pppp_int32")]
        public int large_tick_count;

        [NativeTypeName("pppp_int32")]
        public int spinner_count;

        public double nested_score_per_object;

        public double legacy_score_base_multiplier;

        public double maximum_legacy_combo_score;
    }

    internal partial struct pppp_taiko_difficulty_attributes
    {
        public double mechanical_difficulty;

        public double rhythm_difficulty;

        public double reading_difficulty;

        public double colour_difficulty;

        public double stamina_difficulty;

        public double mono_stamina_factor;

        public double consistency_factor;

        public double stamina_top_strains;
    }

    internal partial struct pppp_osu_performance_attributes
    {
        public double aim;

        public double speed;

        public double accuracy;

        public double flashlight;

        public double reading;

        public double effective_miss_count;

        public double combo_based_estimated_miss_count;

        public double score_based_estimated_miss_count;

        [NativeTypeName("pppp_int32")]
        public int has_score_based_estimated_miss_count;

        public double aim_estimated_slider_breaks;

        public double speed_estimated_slider_breaks;

        public double speed_deviation;

        [NativeTypeName("pppp_int32")]
        public int has_speed_deviation;
    }

    internal partial struct pppp_taiko_performance_attributes
    {
        public double difficulty;

        public double accuracy;

        public double estimated_unstable_rate;

        [NativeTypeName("pppp_int32")]
        public int has_estimated_unstable_rate;
    }

    internal partial struct pppp_mania_performance_attributes
    {
        public double difficulty;
    }

    internal unsafe partial struct pppp_difficulty_attributes
    {
        [NativeTypeName("pppp_int32")]
        public int ruleset;

        public double star_rating;

        [NativeTypeName("pppp_int32")]
        public int max_combo;

        [NativeTypeName("__AnonymousRecord_capi_L327_C22")]
        public _Anonymous_e__Union Anonymous;

        internal ref pppp_osu_difficulty_attributes osu
        {
            get
            {
                fixed (_Anonymous_e__Union* pField = &Anonymous)
                {
                    return ref pField->osu;
                }
            }
        }

        internal ref pppp_taiko_difficulty_attributes taiko
        {
            get
            {
                fixed (_Anonymous_e__Union* pField = &Anonymous)
                {
                    return ref pField->taiko;
                }
            }
        }

        [StructLayout(LayoutKind.Explicit)]
        internal partial struct _Anonymous_e__Union
        {
            [FieldOffset(0)]
            public pppp_osu_difficulty_attributes osu;

            [FieldOffset(0)]
            public pppp_taiko_difficulty_attributes taiko;
        }
    }

    internal unsafe partial struct pppp_performance_attributes
    {
        [NativeTypeName("pppp_int32")]
        public int ruleset;

        public double total;

        [NativeTypeName("__AnonymousRecord_capi_L339_C22")]
        public _Anonymous_e__Union Anonymous;

        internal ref pppp_osu_performance_attributes osu
        {
            get
            {
                fixed (_Anonymous_e__Union* pField = &Anonymous)
                {
                    return ref pField->osu;
                }
            }
        }

        internal ref pppp_taiko_performance_attributes taiko
        {
            get
            {
                fixed (_Anonymous_e__Union* pField = &Anonymous)
                {
                    return ref pField->taiko;
                }
            }
        }

        internal ref pppp_mania_performance_attributes mania
        {
            get
            {
                fixed (_Anonymous_e__Union* pField = &Anonymous)
                {
                    return ref pField->mania;
                }
            }
        }

        [StructLayout(LayoutKind.Explicit)]
        internal partial struct _Anonymous_e__Union
        {
            [FieldOffset(0)]
            public pppp_osu_performance_attributes osu;

            [FieldOffset(0)]
            public pppp_taiko_performance_attributes taiko;

            [FieldOffset(0)]
            public pppp_mania_performance_attributes mania;
        }
    }

    internal partial struct pppp_timed_difficulty_attributes
    {
        public double time;

        public pppp_difficulty_attributes attributes;
    }

    internal partial struct pppp_timed_difficulty
    {
    }

    internal partial struct pppp_strains
    {
    }

    internal unsafe partial struct pppp_osu_strains
    {
        [NativeTypeName("const double *")]
        public double* aim;

        [NativeTypeName("size_t")]
        public UIntPtr aim_count;

        [NativeTypeName("const double *")]
        public double* aim_no_sliders;

        [NativeTypeName("size_t")]
        public UIntPtr aim_no_sliders_count;

        [NativeTypeName("const double *")]
        public double* speed;

        [NativeTypeName("size_t")]
        public UIntPtr speed_count;

        [NativeTypeName("const double *")]
        public double* reading;

        [NativeTypeName("size_t")]
        public UIntPtr reading_count;

        [NativeTypeName("const double *")]
        public double* flashlight;

        [NativeTypeName("size_t")]
        public UIntPtr flashlight_count;
    }

    internal unsafe partial struct pppp_taiko_strains
    {
        [NativeTypeName("const double *")]
        public double* colour;

        [NativeTypeName("size_t")]
        public UIntPtr colour_count;

        [NativeTypeName("const double *")]
        public double* reading;

        [NativeTypeName("size_t")]
        public UIntPtr reading_count;

        [NativeTypeName("const double *")]
        public double* rhythm;

        [NativeTypeName("size_t")]
        public UIntPtr rhythm_count;

        [NativeTypeName("const double *")]
        public double* stamina;

        [NativeTypeName("size_t")]
        public UIntPtr stamina_count;

        [NativeTypeName("const double *")]
        public double* single_colour_stamina;

        [NativeTypeName("size_t")]
        public UIntPtr single_colour_stamina_count;
    }

    internal unsafe partial struct pppp_catch_strains
    {
        [NativeTypeName("const double *")]
        public double* movement;

        [NativeTypeName("size_t")]
        public UIntPtr movement_count;
    }

    internal unsafe partial struct pppp_mania_strains
    {
        [NativeTypeName("const double *")]
        public double* strain;

        [NativeTypeName("size_t")]
        public UIntPtr strain_count;
    }

    internal partial struct pppp_mods
    {
    }

    internal unsafe partial struct pppp_difficulty_options
    {
        [NativeTypeName("pppp_uint32")]
        public uint version;

        [NativeTypeName("const pppp_mods *")]
        public pppp_mods* mods;

        [NativeTypeName("pppp_int32")]
        public int ruleset;

        [NativeTypeName("pppp_int32")]
        public int has_ruleset;

        public double clock_rate;

        [NativeTypeName("pppp_int32")]
        public int has_clock_rate;
    }

    internal unsafe partial struct pppp_performance_options
    {
        [NativeTypeName("pppp_uint32")]
        public uint version;

        [NativeTypeName("const pppp_mods *")]
        public pppp_mods* mods;

        public pppp_score_info score;

        [NativeTypeName("pppp_int32")]
        public int has_score;

        [NativeTypeName("pppp_int32")]
        public int combo;

        [NativeTypeName("pppp_int32")]
        public int has_combo;

        public double accuracy;

        [NativeTypeName("pppp_int32")]
        public int has_accuracy;

        [NativeTypeName("pppp_int32")]
        public int misses;

        [NativeTypeName("pppp_int32")]
        public int has_misses;

        [NativeTypeName("const pppp_difficulty_attributes *")]
        public pppp_difficulty_attributes* difficulty;

        [NativeTypeName("pppp_int32")]
        public int has_difficulty;
    }

    internal partial struct pppp_beatmap
    {
    }

    internal static unsafe partial class Native
    {
        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("const char *")]
        public static extern sbyte* pppp_version();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("pppp_int32")]
        public static extern int pppp_abi_version();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("const char *")]
        public static extern sbyte* pppp_backend();

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("const char *")]
        public static extern sbyte* pppp_result_message(pppp_result result);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_mods_parse([NativeTypeName("const char *")] sbyte* specification, pppp_mods** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_mods_from_legacy([NativeTypeName("pppp_uint32")] uint bits, [NativeTypeName("pppp_int32")] int ruleset, pppp_mods** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_mods_add_classic(pppp_mods* mods);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void pppp_mods_free(pppp_mods* mods);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_from_file([NativeTypeName("const char *")] sbyte* path, pppp_beatmap** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_from_bytes([NativeTypeName("const void *")] void* data, [NativeTypeName("size_t")] UIntPtr size, pppp_beatmap** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void pppp_beatmap_free(pppp_beatmap* map);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_prepare_data(pppp_beatmap* map);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("pppp_int32")]
        public static extern int pppp_beatmap_format_version([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        [return: NativeTypeName("pppp_int32")]
        public static extern int pppp_beatmap_mode([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern double pppp_beatmap_stack_leniency([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_get_difficulty([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, pppp_beatmap_difficulty* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_hit_objects([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_hit_object **")] pppp_hit_object** @out, [NativeTypeName("size_t *")] UIntPtr* count);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_sliders([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_slider **")] pppp_slider** @out, [NativeTypeName("size_t *")] UIntPtr* count);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_timing_points([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_timing_point **")] pppp_timing_point** @out, [NativeTypeName("size_t *")] UIntPtr* count);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_beatmap_breaks([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_break_period **")] pppp_break_period** @out, [NativeTypeName("size_t *")] UIntPtr* count);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_calculate_difficulty([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_difficulty_options *")] pppp_difficulty_options* options, pppp_difficulty_attributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_calculate_performance([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_performance_options *")] pppp_performance_options* options, pppp_performance_attributes* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_calculate_timed_difficulty([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_difficulty_options *")] pppp_difficulty_options* options, pppp_timed_difficulty** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_timed_difficulty_entries([NativeTypeName("const pppp_timed_difficulty *")] pppp_timed_difficulty* timed, [NativeTypeName("const pppp_timed_difficulty_attributes **")] pppp_timed_difficulty_attributes** @out, [NativeTypeName("size_t *")] UIntPtr* count);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void pppp_timed_difficulty_free(pppp_timed_difficulty* timed);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_calculate_strains([NativeTypeName("const pppp_beatmap *")] pppp_beatmap* map, [NativeTypeName("const pppp_difficulty_options *")] pppp_difficulty_options* options, pppp_strains** @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_strains_info([NativeTypeName("const pppp_strains *")] pppp_strains* strains, [NativeTypeName("pppp_int32 *")] int* ruleset, double* start_time, double* section_length);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_strains_osu([NativeTypeName("const pppp_strains *")] pppp_strains* strains, pppp_osu_strains* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_strains_taiko([NativeTypeName("const pppp_strains *")] pppp_strains* strains, pppp_taiko_strains* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_strains_catch([NativeTypeName("const pppp_strains *")] pppp_strains* strains, pppp_catch_strains* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern pppp_result pppp_strains_mania([NativeTypeName("const pppp_strains *")] pppp_strains* strains, pppp_mania_strains* @out);

        [DllImport("pppp_c", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        public static extern void pppp_strains_free(pppp_strains* strains);
    }
}
