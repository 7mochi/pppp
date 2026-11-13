using System;
using System.IO;

namespace Pppp.Tests {
    internal static class TestResources {
        internal static string MapPath(string name) {
            DirectoryInfo current = new DirectoryInfo(AppContext.BaseDirectory);
            while (current != null) {
                string candidate = Path.Combine(current.FullName, "tests", "resources", name);
                if (File.Exists(candidate)) {
                    return candidate;
                }
                current = current.Parent;
            }
            throw new FileNotFoundException(name);
        }
    }
}
