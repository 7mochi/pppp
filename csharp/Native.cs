using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Pppp {
    internal static unsafe partial class Native {
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
    }
}
