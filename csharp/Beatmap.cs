using System;
using System.IO;
using Microsoft.Win32.SafeHandles;

namespace Pppp {
    internal sealed class BeatmapHandle : SafeHandleZeroOrMinusOneIsInvalid {
        internal BeatmapHandle(IntPtr handle)
            : base(true) {
            SetHandle(handle);
        }

        protected override unsafe bool ReleaseHandle() {
            Native.Pppp_Beatmap_Destroy((PpppBeatmap*)handle);

            return true;
        }
    }

    public sealed unsafe class Beatmap : IDisposable {
        private readonly BeatmapHandle handle;
        private readonly PpppNativeBeatmap native;

        private Beatmap(PpppNativeBeatmap native) {
            this.native = native;
            handle = new BeatmapHandle((IntPtr)native.handle);
        }

        internal PpppBeatmap* Map {
            get {
                CheckOpen();
                return native.handle;
            }
        }

        private void CheckOpen() {
            if (handle.IsClosed) {
                throw new ObjectDisposedException("Beatmap");
            }
        }

        public static Beatmap FromFile(string path) {
            if (path == null) {
                throw new ArgumentNullException("path");
            }

            return FromBytes(File.ReadAllBytes(path));
        }

        public static Beatmap FromBytes(byte[] data) {
            if (data == null) {
                throw new ArgumentNullException("data");
            }

            PpppNativeBeatmap native;
            PpppErrorCode status;

            if (data.Length == 0) {
                byte dummy = 0;
                status = Native.Pppp_Beatmap_CreateFromBytes(&dummy, UIntPtr.Zero, &native);
            } else {
                fixed (byte* bytes = data) {
                    status = Native.Pppp_Beatmap_CreateFromBytes(bytes, new UIntPtr((ulong)data.Length), &native);
                }
            }

            Native.CheckBeatmap(status);

            return new Beatmap(native);
        }

        public static Beatmap FromText(string text) {
            if (text == null) {
                throw new ArgumentNullException("text");
            }

            PpppNativeBeatmap native;
            byte[] bytes = Native.Utf8(text);
            PpppErrorCode status;

            fixed (byte* buffer = bytes) {
                status = Native.Pppp_Beatmap_CreateFromText((sbyte*)buffer, &native);
            }

            Native.CheckBeatmap(status);

            return new Beatmap(native);
        }

        public string Title {
            get {
                string value = Native.ReadString(Native.Pppp_Beatmap_GetTitle, Map);
                GC.KeepAlive(this);

                return value;
            }
        }

        public string Artist {
            get {
                string value = Native.ReadString(Native.Pppp_Beatmap_GetArtist, Map);
                GC.KeepAlive(this);

                return value;
            }
        }

        public string Version {
            get {
                string value = Native.ReadString(Native.Pppp_Beatmap_GetVersion, Map);
                GC.KeepAlive(this);

                return value;
            }
        }

        public long BeatmapId {
            get {
                CheckOpen();

                return native.beatmap_id;
            }
        }

        public int FormatVersion {
            get {
                CheckOpen();

                return native.format_version;
            }
        }

        public int Mode {
            get {
                CheckOpen();

                return native.ruleset_id;
            }
        }

        public double StackLeniency {
            get {
                CheckOpen();

                return native.stack_leniency;
            }
        }

        public BeatmapDifficulty Difficulty {
            get {
                CheckOpen();

                return new BeatmapDifficulty(native.drain_rate, native.circle_size, native.overall_difficulty,
                                              native.approach_rate, native.slider_multiplier,
                                              native.slider_tick_rate);
            }
        }

        public void Dispose() {
            handle.Dispose();
        }
    }
}
