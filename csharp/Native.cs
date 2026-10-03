using System;
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
        internal struct DifficultyAttributes {
            internal int ruleset;
            internal double star_rating;
            internal int max_combo;
            internal OsuDifficultyAttributes osu;
            internal TaikoDifficultyAttributes taiko;
            internal CatchDifficultyAttributes fruits;
            internal ManiaDifficultyAttributes mania;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct PerformanceAttributes {
            internal int ruleset;
            internal double total;
            internal OsuPerformanceAttributes osu;
            internal TaikoPerformanceAttributes taiko;
            internal CatchPerformanceAttributes fruits;
            internal ManiaPerformanceAttributes mania;
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

        internal static ScoreInfo ToNative(Pppp.ScoreInfo score) {
            if (score.statistics.Length != HitResultCount) {
                throw new ArgumentException("statistics must be a list of " + HitResultCount + " counts");
            }
            if (score.maximum_statistics.Length != HitResultCount) {
                throw new ArgumentException("statistics must be a list of " + HitResultCount + " counts");
            }
            ScoreInfo native = new ScoreInfo();
            for (int i = 0; i < HitResultCount; i++) {
                native.statistics[i] = score.statistics[i];
                native.maximum_statistics[i] = score.maximum_statistics[i];
            }
            native.max_combo = score.max_combo;
            native.accuracy = score.accuracy;
            native.has_legacy_total_score = score.legacy_total_score.HasValue ? 1 : 0;
            native.legacy_total_score = score.legacy_total_score.GetValueOrDefault();
            return native;
        }

        internal static Pppp.DifficultyAttributes FromNative(DifficultyAttributes attributes) {
            Pppp.DifficultyAttributes managed = new Pppp.DifficultyAttributes();
            managed.ruleset = attributes.ruleset;
            managed.star_rating = attributes.star_rating;
            managed.max_combo = attributes.max_combo;
            managed.osu = attributes.osu;
            managed.taiko = attributes.taiko;
            managed.fruits = attributes.fruits;
            managed.mania = attributes.mania;
            return managed;
        }

        internal static Pppp.OsuPerformanceAttributes FromNative(OsuPerformanceAttributes attributes) {
            Pppp.OsuPerformanceAttributes managed = new Pppp.OsuPerformanceAttributes();
            managed.total = attributes.total;
            managed.aim = attributes.aim;
            managed.speed = attributes.speed;
            managed.accuracy = attributes.accuracy;
            managed.flashlight = attributes.flashlight;
            managed.reading = attributes.reading;
            managed.effective_miss_count = attributes.effective_miss_count;
            managed.combo_based_estimated_miss_count = attributes.combo_based_estimated_miss_count;
            if (attributes.has_score_based_estimated_miss_count != 0) {
                managed.score_based_estimated_miss_count = attributes.score_based_estimated_miss_count;
            }
            managed.aim_estimated_slider_breaks = attributes.aim_estimated_slider_breaks;
            managed.speed_estimated_slider_breaks = attributes.speed_estimated_slider_breaks;
            if (attributes.has_speed_deviation != 0) {
                managed.speed_deviation = attributes.speed_deviation;
            }
            return managed;
        }

        internal static Pppp.TaikoPerformanceAttributes FromNative(TaikoPerformanceAttributes attributes) {
            Pppp.TaikoPerformanceAttributes managed = new Pppp.TaikoPerformanceAttributes();
            managed.total = attributes.total;
            managed.difficulty = attributes.difficulty;
            managed.accuracy = attributes.accuracy;
            if (attributes.has_estimated_unstable_rate != 0) {
                managed.estimated_unstable_rate = attributes.estimated_unstable_rate;
            }
            return managed;
        }

        internal static Pppp.PerformanceAttributes FromNative(PerformanceAttributes attributes) {
            Pppp.PerformanceAttributes managed = new Pppp.PerformanceAttributes();
            managed.ruleset = attributes.ruleset;
            managed.total = attributes.total;
            managed.osu = FromNative(attributes.osu);
            managed.taiko = FromNative(attributes.taiko);
            managed.fruits = attributes.fruits;
            managed.mania = attributes.mania;
            return managed;
        }

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr pppp_version();

        [DllImport(Library, ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int pppp_beatmap_from_file(byte[] path, out IntPtr map);

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
        internal static extern int pppp_calculate_performance(IntPtr map, ref PerformanceOptions options,
                                                              out PerformanceAttributes attributes);
    }
}
