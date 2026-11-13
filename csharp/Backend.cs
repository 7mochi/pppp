namespace Pppp {
    public static unsafe class Backend {
        public static string Name {
            get { return Native.Utf8String(Native.Pppp_Backend()); }
        }
    }
}
