using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Pppp {
    internal static unsafe partial class Native {
        internal delegate PpppErrorCode BeatmapStringGetter(PpppBeatmap* handle, sbyte* buffer, int* size);

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
                throw new InvalidDataException("cannot parse the beatmap");
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
    }
}
