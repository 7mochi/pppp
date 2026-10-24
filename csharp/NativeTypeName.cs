using System;

namespace System.Runtime.CompilerServices {
    /// <summary>
    /// Names the C type a generated declaration came from. ClangSharpPInvokeGenerator emits it;
    /// it is not part of the framework for the targets this package supports.
    /// </summary>
    [AttributeUsage(AttributeTargets.All, AllowMultiple = false)]
    internal sealed class NativeTypeNameAttribute : Attribute {
        internal NativeTypeNameAttribute(string name) { this.Name = name; }

        internal string Name { get; private set; }
    }
}
