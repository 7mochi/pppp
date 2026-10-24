using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Pppp {
    // The other half of this class is generated into Native.g.cs from include/pppp/capi.h.
    internal static unsafe partial class Native {
        // The header declares these as macros, which the generator cannot see.
        internal const int AbiVersion = 1;
        internal const int HitResultCount = 18;
        internal const uint DifficultyOptionsVersion = 1;
        internal const uint PerformanceOptionsVersion = 1;

        internal static byte[] Utf8(string value) {
            byte[] bytes = Encoding.UTF8.GetBytes(value);
            byte[] terminated = new byte[bytes.Length + 1];
            Array.Copy(bytes, terminated, bytes.Length);
            return terminated;
        }

        internal static string String(sbyte* value) {
            return Marshal.PtrToStringAnsi((IntPtr)value);
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
            long length = (long)count.ToUInt64();
            uint[] values = new uint[length];
            if (pointer == IntPtr.Zero) {
                return values;
            }
            for (long i = 0; i < length; i++) {
                values[i] = (uint)Marshal.ReadInt32(pointer, (int)(i * sizeof(int)));
            }
            return values;
        }

        internal static void Check(pppp_result status) {
            if (status == pppp_result.PPPP_ALLOCATION) {
                throw new OutOfMemoryException();
            }
            if (status != pppp_result.PPPP_OK) {
                throw new InvalidOperationException(String(pppp_result_message(status)));
            }
        }

        internal static IntPtr CreateMods(Pppp.Mods mods, int ruleset) {
            pppp_mods* handle;
            pppp_result status;
            if (mods.IsLegacyBitmask) {
                status = pppp_mods_from_legacy((uint)mods.Legacy, ruleset, &handle);
            } else {
                byte[] bytes = Utf8(mods.Specification);
                fixed (byte* specification = bytes) {
                    status = pppp_mods_parse((sbyte*)specification, &handle);
                }
            }
            if (status == pppp_result.PPPP_ALLOCATION) {
                throw new OutOfMemoryException();
            }
            if (status != pppp_result.PPPP_OK) {
                throw new ArgumentException("invalid mod specification", "Mods");
            }
            if (mods.Classic) {
                if (pppp_mods_add_classic(handle) != pppp_result.PPPP_OK) {
                    pppp_mods_free(handle);
                    throw new ArgumentException("invalid mod specification", "Mods");
                }
            }
            return (IntPtr)handle;
        }

        internal static pppp_score_info ToNative(Dictionary<HitResult, int> statistics, int? maxCombo, double? accuracy,
                                              long? legacyTotalScore) {
            pppp_score_info native = new pppp_score_info();
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

        internal static Pppp.DifficultyAttributes FromNative(pppp_difficulty_attributes native) {
            switch (native.ruleset) {
            case (int)Ruleset.Taiko:
                return new Pppp.TaikoDifficultyAttributes(native);
            case (int)Ruleset.Catch:
                return new Pppp.CatchDifficultyAttributes(native);
            case (int)Ruleset.Mania:
                return new Pppp.ManiaDifficultyAttributes(native);
            default:
                return new Pppp.OsuDifficultyAttributes(native);
            }
        }

        internal static Pppp.PerformanceAttributes FromNative(pppp_performance_attributes native) {
            switch (native.ruleset) {
            case (int)Ruleset.Taiko:
                return new Pppp.TaikoPerformanceAttributes(native);
            case (int)Ruleset.Catch:
                return new Pppp.CatchPerformanceAttributes(native);
            case (int)Ruleset.Mania:
                return new Pppp.ManiaPerformanceAttributes(native);
            default:
                return new Pppp.OsuPerformanceAttributes(native);
            }
        }
    }
}
