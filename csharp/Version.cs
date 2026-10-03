using System.Runtime.InteropServices;

namespace Pppp {
    /// <summary>The version as `major.minor.patch`.</summary>
    public static class Version {
        public static string Current {
            get { return Marshal.PtrToStringAnsi(Native.pppp_version()); }
        }
    }
}
