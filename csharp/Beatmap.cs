using System;
using System.IO;
using Microsoft.Win32.SafeHandles;

namespace Pppp {
    internal sealed class BeatmapHandle : SafeHandleZeroOrMinusOneIsInvalid {
        internal BeatmapHandle(IntPtr handle)
            : base(true) {
            SetHandle(handle);
        }

        protected override bool ReleaseHandle() {
            Native.pppp_beatmap_free(handle);
            return true;
        }
    }

    /// <summary>A loaded beatmap.</summary>
    public sealed class Beatmap : IDisposable {
        private readonly BeatmapHandle handle;

        private HitObject[] hitObjects;
        private Slider[] sliders;
        private TimingPoint[] timingPoints;
        private BreakPeriod[] breaks;

        private Beatmap(BeatmapHandle handle) { this.handle = handle; }

        /// <summary>Parse a <see cref="Beatmap"/> by providing a path to a `.osu` file.</summary>
        public static Beatmap FromFile(string path) {
            if (path == null) {
                throw new ArgumentNullException("path");
            }
            return FromBytes(File.ReadAllBytes(path));
        }

        /// <summary>Parse a <see cref="Beatmap"/> by providing the content of a `.osu` file as a slice of bytes.</summary>
        public static Beatmap FromBytes(byte[] data) {
            if (data == null) {
                throw new ArgumentNullException("data");
            }
            IntPtr map;
            int status = Native.pppp_beatmap_from_bytes(data, new UIntPtr((uint)data.Length), out map);
            if (status == Native.Allocation) {
                throw new OutOfMemoryException();
            }
            if (status != Native.Ok) {
                throw new InvalidDataException("cannot parse the beatmap");
            }
            return new Beatmap(new BeatmapHandle(map));
        }

        internal IntPtr Handle {
            get {
                if (handle.IsClosed) {
                    throw new ObjectDisposedException("Beatmap");
                }
                return handle.DangerousGetHandle();
            }
        }

        public int FormatVersion {
            get { return Native.pppp_beatmap_format_version(Handle); }
        }

        public int Mode {
            get { return Native.pppp_beatmap_mode(Handle); }
        }

        public double StackLeniency {
            get { return Native.pppp_beatmap_stack_leniency(Handle); }
        }

        /// <summary>A representation of all top-level difficulty settings for a beatmap.</summary>
        public BeatmapDifficulty Difficulty {
            get {
                BeatmapDifficulty settings;
                Native.pppp_beatmap_get_difficulty(Handle, out settings);
                return settings;
            }
        }

        public HitObject[] HitObjects {
            get {
                if (hitObjects == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_hit_objects(Handle, out pointer, out count);
                    hitObjects = Native.CopyArray<HitObject>(pointer, count);
                }
                return hitObjects;
            }
        }

        public Slider[] Sliders {
            get {
                if (sliders == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_sliders(Handle, out pointer, out count);
                    Native.Slider[] views = Native.CopyArray<Native.Slider>(pointer, count);
                    sliders = new Slider[views.Length];
                    for (int i = 0; i < views.Length; i++) {
                        sliders[i] = new Slider(views[i].slides, views[i].expected_length,
                                                Native.CopyUnsigned(views[i].node_sounds, views[i].node_sound_count),
                                                Native.CopyArray<Vector2>(views[i].control_points,
                                                                          views[i].control_point_count),
                                                Native.CopyArray<Vector2>(views[i].path, views[i].path_count),
                                                Native.CopyDoubles(views[i].cumulative_lengths,
                                                                   views[i].cumulative_length_count),
                                                Native.CopyArray<Vector2>(views[i].undecimated_path,
                                                                          views[i].undecimated_path_count),
                                                Native.CopyDoubles(views[i].undecimated_cumulative_lengths,
                                                                   views[i].undecimated_cumulative_length_count),
                                                Native.CopyArray<SliderEventDescriptor>(views[i].events,
                                                                                       views[i].event_count),
                                                Native.CopyArray<SliderEventDescriptor>(views[i].catch_events,
                                                                                       views[i].catch_event_count));
                    }
                }
                return sliders;
            }
        }

        public TimingPoint[] TimingPoints {
            get {
                if (timingPoints == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_timing_points(Handle, out pointer, out count);
                    timingPoints = Native.CopyArray<TimingPoint>(pointer, count);
                }
                return timingPoints;
            }
        }

        public BreakPeriod[] Breaks {
            get {
                if (breaks == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_breaks(Handle, out pointer, out count);
                    breaks = Native.CopyArray<BreakPeriod>(pointer, count);
                }
                return breaks;
            }
        }

        public void Dispose() {
            handle.Dispose();
            GC.SuppressFinalize(this);
        }
    }
}
