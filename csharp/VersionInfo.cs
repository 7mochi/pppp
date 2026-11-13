namespace Pppp {
    public static unsafe class VersionInfo {
        public static string Current {
            get { return Native.Utf8String(Native.Pppp_Version()); }
        }
    }
}
