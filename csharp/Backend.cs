using System.Runtime.InteropServices;

namespace Pppp {
    /// <summary>The parsing engine the native library loaded: `scalar`, `avx2` or `neon`.</summary>
    public static class Backend {
        public static string Name {
            get { return Marshal.PtrToStringAnsi(Native.pppp_backend()); }
        }
    }
}
