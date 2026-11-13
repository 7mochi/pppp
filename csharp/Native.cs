using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Win32.SafeHandles;

namespace Pppp {
    internal static unsafe partial class Native {
        internal delegate PpppErrorCode BeatmapStringGetter(PpppBeatmap* handle, sbyte* buffer, int* size);
        internal delegate PpppErrorCode DoubleGetter(PpppStrains* handle, double* buffer, int* size);

        internal static byte[] Utf8(string value) {
            byte[] bytes = Encoding.UTF8.GetBytes(value);
            byte[] terminated = new byte[bytes.Length + 1];
            Array.Copy(bytes, terminated, bytes.Length);
            return terminated;
        }

        internal static string Utf8String(sbyte* value) {
            if (value == null) {
                return null;
            }

            int length = 0;
            while (value[length] != 0) {
                length++;
            }

            if (length == 0) {
                return "";
            }

            byte[] bytes = new byte[length];
            Marshal.Copy((IntPtr)value, bytes, 0, length);
            return Encoding.UTF8.GetString(bytes);
        }

        internal static void Check(PpppErrorCode status) {
            if (status == PpppErrorCode.Pppp_Allocation) {
                throw new OutOfMemoryException();
            }

            if (status != PpppErrorCode.Pppp_Success) {
                string message = Utf8String(Pppp_ErrorHandler_GetLastMessage());
                throw new InvalidOperationException(string.IsNullOrEmpty(message) ? status.ToString() : message);
            }
        }

        internal static void CheckBeatmap(PpppErrorCode status) {
            if (status == PpppErrorCode.Pppp_Parse) {
                string message = Utf8String(Pppp_ErrorHandler_GetLastMessage());

                throw new InvalidDataException(string.IsNullOrEmpty(message) ? "cannot parse the beatmap" : message);
            }

            Check(status);
        }

        internal static string ReadString(BeatmapStringGetter getter, PpppBeatmap* handle) {
            int size = 0;
            PpppErrorCode status = getter(handle, null, &size);
            if (status != PpppErrorCode.Pppp_BufferSizeQuery) {
                Check(status);
            }

            if (size <= 0) {
                return "";
            }

            byte[] bytes = new byte[size];
            fixed (byte* buffer = bytes) {
                status = getter(handle, (sbyte*)buffer, &size);
            }
            Check(status);

            int length = size - 1;
            while (length > 0 && bytes[length - 1] == 0) {
                length--;
            }

            return Encoding.UTF8.GetString(bytes, 0, length);
        }

        internal static RulesetHandle CreateRuleset(Ruleset ruleset) {
            PpppNativeRuleset native;
            PpppErrorCode status = Pppp_Ruleset_CreateFromId((int)ruleset, &native);
            if (status == PpppErrorCode.Pppp_RulesetUnavailable) {
                throw new ArgumentOutOfRangeException("ruleset", "ruleset must be one of 0, 1, 2, 3");
            }

            Check(status);

            return new RulesetHandle((IntPtr)native.handle);
        }

        internal static ModsHandle CreateMods(Mods mods, Ruleset ruleset) {
            if (mods.IsEmpty) {
                return null;
            }

            PpppNativeMods native;
            Check(Pppp_Mods_Create(&native));

            PpppErrorCode status;
            if (mods.IsLegacyBitmask) {
                status = Pppp_Mods_FromLegacy(native.handle, (uint)mods.Legacy, (int)ruleset);
            } else {
                byte[] bytes = Utf8(mods.Specification);
                fixed (byte* specification = bytes) {
                    status = Pppp_Mods_Parse(native.handle, (sbyte*)specification);
                }
            }

            if (status != PpppErrorCode.Pppp_Success) {
                Pppp_Mods_Destroy(native.handle);
                if (status == PpppErrorCode.Pppp_Allocation) {
                    throw new OutOfMemoryException();
                }

                throw new ArgumentException("invalid mod specification", "Mods");
            }

            if (mods.Classic && Pppp_Mods_AddClassic(native.handle) != PpppErrorCode.Pppp_Success) {
                Pppp_Mods_Destroy(native.handle);

                throw new ArgumentException("invalid mod specification", "Mods");
            }

            return new ModsHandle((IntPtr)native.handle);
        }

        internal static DifficultyAttributes FromNative(PpppOsuDifficultyAttributes value) {
            return new OsuDifficultyAttributes(value);
        }

        internal static DifficultyAttributes FromNative(PpppTaikoDifficultyAttributes value) {
            return new TaikoDifficultyAttributes(value);
        }

        internal static DifficultyAttributes FromNative(PpppCatchDifficultyAttributes value) {
            return new CatchDifficultyAttributes(value);
        }

        internal static DifficultyAttributes FromNative(PpppManiaDifficultyAttributes value) {
            return new ManiaDifficultyAttributes(value);
        }

    }

    internal sealed class RulesetHandle : SafeHandleZeroOrMinusOneIsInvalid {
        internal RulesetHandle(IntPtr handle)
            : base(true) {
            SetHandle(handle);
        }

        internal unsafe PpppRuleset* Pointer {
            get { return (PpppRuleset*)DangerousGetHandle(); }
        }

        protected override unsafe bool ReleaseHandle() {
            Native.Pppp_Ruleset_Destroy((PpppRuleset*)handle);
            return true;
        }
    }

    internal sealed class ModsHandle : SafeHandleZeroOrMinusOneIsInvalid {
        internal ModsHandle(IntPtr handle)
            : base(true) {
            SetHandle(handle);
        }

        internal unsafe PpppMods* Pointer {
            get { return (PpppMods*)DangerousGetHandle(); }
        }

        protected override unsafe bool ReleaseHandle() {
            Native.Pppp_Mods_Destroy((PpppMods*)handle);

            return true;
        }
    }
}
