using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Pppp {
    internal static unsafe class Native {
        internal const string Library = "pppp_c";

        internal const int Ok = 0;
        internal const int InvalidArgument = 1;
        internal const int Allocation = 2;
        internal const int Parse = 3;

        internal const int HitResultCount = 18;

        [StructLayout(LayoutKind.Sequential)]
        internal struct ScoreInfo {
            internal fixed int statistics[HitResultCount];
            internal fixed int maximum_statistics[HitResultCount];
            internal int max_combo;
            internal double accuracy;
            internal int has_legacy_total_score;
            internal long legacy_total_score;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DifficultyOptions {
            internal IntPtr mods;
            internal int ruleset;
            internal int has_ruleset;
            internal double clock_rate;
            internal int has_clock_rate;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct PerformanceOptions {
            internal IntPtr mods;
            internal ScoreInfo score;
            internal int has_score;
            internal int combo;
            internal int has_combo;
            internal double accuracy;
            internal int has_accuracy;
            internal int misses;
            internal int has_misses;
            internal IntPtr difficulty;
            internal int has_difficulty;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct Slider {
            internal int slides;
            internal double expected_length;

            internal IntPtr node_sounds;
            internal UIntPtr node_sound_count;

            internal IntPtr control_points;
            internal UIntPtr control_point_count;

            internal IntPtr path;
            internal UIntPtr path_count;

            internal IntPtr cumulative_lengths;
            internal UIntPtr cumulative_length_count;

            internal IntPtr undecimated_path;
            internal UIntPtr undecimated_path_count;

            internal IntPtr undecimated_cumulative_lengths;
            internal UIntPtr undecimated_cumulative_length_count;

            internal IntPtr events;
            internal UIntPtr event_count;

            internal IntPtr catch_events;
            internal UIntPtr catch_event_count;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct OsuDifficultyAttributes {
            internal double star_rating;
            internal int max_combo;
            internal double aim_difficulty;
            internal double speed_difficulty;
            internal double reading_difficulty;
            internal double flashlight_difficulty;
            internal double slider_factor;
            internal double aim_difficult_strain_count;
            internal double speed_difficult_strain_count;
            internal double reading_difficult_note_count;
            internal double aim_difficult_slider_count;
            internal double aim_top_weighted_slider_factor;
            internal double speed_top_weighted_slider_factor;
            internal double speed_note_count;
            internal int hit_circle_count;
            internal int slider_count;
            internal int large_tick_count;
            internal int spinner_count;
            internal double nested_score_per_object;
            internal double legacy_score_base_multiplier;
            internal double maximum_legacy_combo_score;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct TaikoDifficultyAttributes {
            internal double star_rating;
            internal int max_combo;
            internal double mechanical_difficulty;
            internal double rhythm_difficulty;
            internal double reading_difficulty;
            internal double colour_difficulty;
            internal double stamina_difficulty;
            internal double mono_stamina_factor;
            internal double consistency_factor;
            internal double stamina_top_strains;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct CatchDifficultyAttributes {
            internal double star_rating;
            internal int max_combo;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct ManiaDifficultyAttributes {
            internal double star_rating;
            internal int max_combo;
        }

        [StructLayout(LayoutKind.Explicit)]
        internal struct DifficultyUnion {
            [FieldOffset(0)]
            internal OsuDifficultyAttributes osu;
            [FieldOffset(0)]
            internal TaikoDifficultyAttributes taiko;
            [FieldOffset(0)]
            internal CatchDifficultyAttributes fruits;
            [FieldOffset(0)]
            internal ManiaDifficultyAttributes mania;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DifficultyAttributes {
            internal int ruleset;
            internal double star_rating;
            internal int max_combo;
            internal DifficultyUnion attributes;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct TimedDifficultyAttributes {
            internal double time;
            internal DifficultyAttributes attributes;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct OsuStrains {
            internal IntPtr aim;
            internal UIntPtr aim_count;
            internal IntPtr aim_no_sliders;
            internal UIntPtr aim_no_sliders_count;
            internal IntPtr speed;
            internal UIntPtr speed_count;
            internal IntPtr reading;
            internal UIntPtr reading_count;
            internal IntPtr flashlight;
            internal UIntPtr flashlight_count;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct TaikoStrains {
            internal IntPtr colour;
            internal UIntPtr colour_count;
            internal IntPtr reading;
            internal UIntPtr reading_count;
            internal IntPtr rhythm;
            internal UIntPtr rhythm_count;
            internal IntPtr stamina;
            internal UIntPtr stamina_count;
            internal IntPtr single_colour_stamina;
            internal UIntPtr single_colour_stamina_count;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct CatchStrains {
            internal IntPtr movement;
            internal UIntPtr movement_count;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct ManiaStrains {
            internal IntPtr strain;
            internal UIntPtr strain_count;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct OsuPerformanceAttributes {
            internal double total;
            internal double aim;
            internal double speed;
            internal double accuracy;
            internal double flashlight;
            internal double reading;
            internal double effective_miss_count;
            internal double combo_based_estimated_miss_count;
            internal double score_based_estimated_miss_count;
            internal int has_score_based_estimated_miss_count;
            internal double aim_estimated_slider_breaks;
            internal double speed_estimated_slider_breaks;
            internal double speed_deviation;
            internal int has_speed_deviation;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct TaikoPerformanceAttributes {
            internal double total;
            internal double difficulty;
            internal double accuracy;
            internal double estimated_unstable_rate;
            internal int has_estimated_unstable_rate;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct CatchPerformanceAttributes {
            internal double total;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct ManiaPerformanceAttributes {
            internal double total;
            internal double difficulty;
        }

        [StructLayout(LayoutKind.Explicit)]
        internal struct PerformanceUnion {
            [FieldOffset(0)]
            internal OsuPerformanceAttributes osu;
            [FieldOffset(0)]
            internal TaikoPerformanceAttributes taiko;
            [FieldOffset(0)]
            internal CatchPerformanceAttributes fruits;
            [FieldOffset(0)]
            internal ManiaPerformanceAttributes mania;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct PerformanceAttributes {
            internal int ruleset;
            internal double total;
            internal PerformanceUnion attributes;
        }

        internal static byte[] Utf8(string value) {
            byte[] bytes = Encoding.UTF8.GetBytes(value);
            byte[] terminated = new byte[bytes.Length + 1];
            Array.Copy(bytes, terminated, bytes.Length);
            return terminated;
        }

        internal static T[] CopyArray<T>(IntPtr pointer, UIntPtr count) where T : struct {
            long length = (long)count.ToUInt64();
            T[] values = new T[length];
            if (pointer == IntPtr.Zero) {
                return values;
            }
            int size = Marshal.SizeOf(typeof(T));
            for (long i = 0; i < length; i++) {
                values[i] = (T)Marshal.PtrToStructure(new IntPtr(pointer.ToInt64() + i * size), typeof(T));
            }
            return values;
        }

        internal static double[] CopyDoubles(IntPtr pointer, UIntPtr count) {
            double[] values = new double[(long)count.ToUInt64()];
            if (pointer != IntPtr.Zero && values.Length > 0) {
                Marshal.Copy(pointer, values, 0, values.Length);
            }
            return values;
        }

        internal static uint[] CopyUnsigned(IntPtr pointer, UIntPtr count) {
            uint[] values = new uint[(long)count.ToUInt64()];
            for (int i = 0; i < values.Length; i++) {
                values[i] = (uint)Marshal.ReadInt32(pointer, i * sizeof(int));
            }
            return values;
        }

        internal static void Check(int status) {
            if (status == Allocation) {
                throw new OutOfMemoryException();
            }
            if (status != Ok) {
                throw new InvalidOperationException(Marshal.PtrToStringAnsi(pppp_result_message(status)));
            }
        }

        internal static IntPtr CreateMods(Pppp.Mods mods) {
            IntPtr handle;
            int status;
            if (mods.IsLegacyBitmask) {
                status = pppp_mods_from_legacy((uint)mods.Legacy, out handle);
            } else {
                status = pppp_mods_parse(Utf8(mods.Specification), out handle);
            }
            if (status == Allocation) {
                throw new OutOfMemoryException();
            }
            if (status != Ok) {
                throw new ArgumentException("invalid mod specification", "Mods");
            }
            if (mods.Classic) {
                if (pppp_mods_add_classic(handle) != Ok) {
                    pppp_mods_free(handle);
                    throw new ArgumentException("invalid mod specification", "Mods");
                }
            }
            return handle;
        }

        internal static ScoreInfo ToNative(Dictionary<HitResult, int> statistics, int? maxCombo, double? accuracy,
                                           long? legacyTotalScore) {
            ScoreInfo native = new ScoreInfo();
            if (statistics != null) {
                foreach (KeyValuePair<HitResult, int> entry in statistics) {
                    int index = (int)entry.Key;
                    if (index < 0 || index >= HitResultCount) {
                        throw new ArgumentOutOfRangeException("Statistics", "statistics keys must be hit results");
                    }
                    native.statistics[index] = entry.Value;
                }
            }
            native.max_combo = maxCombo.GetValueOrDefault();
            native.accuracy = accuracy.GetValueOrDefault();
            native.has_legacy_total_score = legacyTotalScore.HasValue ? 1 : 0;
            native.legacy_total_score = legacyTotalScore.GetValueOrDefault();
            return native;
        }

        internal static Pppp.DifficultyAttributes FromNative(DifficultyAttributes native) {
            switch (native.ruleset) {
            case (int)Ruleset.Taiko:
                return new Pppp.TaikoDifficultyAttributes(native.attributes.taiko);
            case (int)Ruleset.Catch:
                return new Pppp.CatchDifficultyAttributes(native.attributes.fruits);
            case (int)Ruleset.Mania:
                return new Pppp.ManiaDifficultyAttributes(native.attributes.mania);
            default:
                return new Pppp.OsuDifficultyAttributes(native.attributes.osu);
            }
        }

        internal static Pppp.PerformanceAttributes FromNative(PerformanceAttributes native) {
            switch (native.ruleset) {
            case (int)Ruleset.Taiko:
                return new Pppp.TaikoPerformanceAttributes(native.attributes.taiko);
            case (int)Ruleset.Catch:
                return new Pppp.CatchPerformanceAttributes(native.attributes.fruits);
            case (int)Ruleset.Mania:
                return new Pppp.ManiaPerformanceAttributes(native.attributes.mania);
            default:
                return new Pppp.OsuPerformanceAttributes(native.attributes.osu);
            }
        }

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr pppp_version();

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr pppp_result_message(int result);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_mods_parse(byte[] specification, out IntPtr mods);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_mods_from_legacy(uint bits, out IntPtr mods);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_mods_add_classic(IntPtr mods);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void pppp_mods_free(IntPtr mods);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_from_bytes(byte[] data, UIntPtr size, out IntPtr map);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void pppp_beatmap_free(IntPtr map);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_format_version(IntPtr map);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_mode(IntPtr map);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern double pppp_beatmap_stack_leniency(IntPtr map);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_get_difficulty(IntPtr map, out BeatmapDifficulty settings);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_hit_objects(IntPtr map, out IntPtr objects,
                                                            out UIntPtr count);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_sliders(IntPtr map, out IntPtr sliders, out UIntPtr count);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_timing_points(IntPtr map, out IntPtr points,
                                                              out UIntPtr count);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_breaks(IntPtr map, out IntPtr breaks, out UIntPtr count);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_calculate_difficulty(IntPtr map, ref DifficultyOptions options,
                                                             out DifficultyAttributes attributes);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_calculate_timed_difficulty(IntPtr map, ref DifficultyOptions options,
                                                                   out IntPtr timed);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_timed_difficulty_entries(IntPtr timed, out IntPtr entries, out UIntPtr count);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void pppp_timed_difficulty_free(IntPtr timed);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_calculate_strains(IntPtr map, ref DifficultyOptions options, out IntPtr strains);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_strains_info(IntPtr strains, out int ruleset, out double startTime,
                                                     out double sectionLength);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_strains_osu(IntPtr strains, out OsuStrains osu);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_strains_taiko(IntPtr strains, out TaikoStrains taiko);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_strains_catch(IntPtr strains, out CatchStrains fruits);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_strains_mania(IntPtr strains, out ManiaStrains mania);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void pppp_strains_free(IntPtr strains);

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_calculate_performance(IntPtr map, ref PerformanceOptions options,
                                                              out PerformanceAttributes attributes);
    }
}
