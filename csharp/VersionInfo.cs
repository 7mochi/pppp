namespace Pppp {
    /// <summary>The version of the native library, as `major.minor.patch`.</summary>
    public static unsafe class VersionInfo {
        public static string Current {
            get { return Native.String(Native.pppp_version()); }
        }
    }
}
