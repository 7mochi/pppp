namespace Pppp {
    /// <summary>The parsing engine the native library loaded: `scalar`, `avx2` or `neon`.</summary>
    public static unsafe class Backend {
        public static string Name {
            get { return Native.String(Native.pppp_backend()); }
        }
    }
}
