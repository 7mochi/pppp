using System;

namespace System.Runtime.CompilerServices {
    [AttributeUsage(AttributeTargets.All, AllowMultiple = false)]
    internal sealed class NativeTypeNameAttribute : Attribute {
        private readonly string name;

        internal NativeTypeNameAttribute(string name) { this.name = name; }

        internal string Name {
            get { return name; }
        }
    }
}
