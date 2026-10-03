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

        private HitObject[] _hit_objects;
        private Slider[] _sliders;
        private TimingPoint[] _timing_points;
        private BreakPeriod[] _breaks;

        private Beatmap(BeatmapHandle handle) { this.handle = handle; }

        /// <summary>Load a beatmap from a `.osu` file path.</summary>
        public static Beatmap FromFile(string path) {
            if (path == null) {
                throw new ArgumentNullException("path");
            }
            IntPtr map;
            int status = Native.pppp_beatmap_from_file(Native.Utf8(path), out map);
            if (status == Native.Allocation) {
                throw new OutOfMemoryException();
            }
            if (status != Native.Ok) {
                throw new InvalidDataException("cannot parse the beatmap (result " + status + ")");
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

        public int format_version {
            get { return Native.pppp_beatmap_format_version(Handle); }
        }

        public int mode {
            get { return Native.pppp_beatmap_mode(Handle); }
        }

        public double stack_leniency {
            get { return Native.pppp_beatmap_stack_leniency(Handle); }
        }

        /// <summary>A representation of all top-level difficulty settings for a beatmap.</summary>
        public BeatmapDifficulty difficulty {
            get {
                BeatmapDifficulty settings;
                Native.pppp_beatmap_get_difficulty(Handle, out settings);
                return settings;
            }
        }

        public HitObject[] hit_objects {
            get {
                if (_hit_objects == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_hit_objects(Handle, out pointer, out count);
                    _hit_objects = Native.CopyArray<HitObject>(pointer, count);
                }
                return _hit_objects;
            }
        }

        public Slider[] sliders {
            get {
                if (_sliders == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_sliders(Handle, out pointer, out count);
                    Native.Slider[] views = Native.CopyArray<Native.Slider>(pointer, count);
                    _sliders = new Slider[views.Length];
                    for (int i = 0; i < views.Length; i++) {
                        _sliders[i] = new Slider(views[i].slides, views[i].expected_length,
                                                 Native.CopyUnsigned(views[i].node_sounds,
                                                                     views[i].node_sound_count),
                                                 Native.CopyArray<Vector2>(views[i].control_points,
                                                                           views[i].control_point_count),
                                                 Native.CopyArray<Vector2>(views[i].path,
                                                                           views[i].path_count),
                                                 Native.CopyDoubles(views[i].cumulative_lengths,
                                                                    views[i].cumulative_length_count),
                                                 Native.CopyArray<Vector2>(views[i].undecimated_path,
                                                                           views[i].undecimated_path_count),
                                                 Native.CopyDoubles(
                                                     views[i].undecimated_cumulative_lengths,
                                                     views[i].undecimated_cumulative_length_count),
                                                 Native.CopyArray<SliderEventDescriptor>(views[i].events,
                                                                                        views[i].event_count),
                                                 Native.CopyArray<SliderEventDescriptor>(views[i].catch_events,
                                                                                        views[i].catch_event_count));
                    }
                }
                return _sliders;
            }
        }

        public TimingPoint[] timing_points {
            get {
                if (_timing_points == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_timing_points(Handle, out pointer, out count);
                    _timing_points = Native.CopyArray<TimingPoint>(pointer, count);
                }
                return _timing_points;
            }
        }

        public BreakPeriod[] breaks {
            get {
                if (_breaks == null) {
                    IntPtr pointer;
                    UIntPtr count;
                    Native.pppp_beatmap_breaks(Handle, out pointer, out count);
                    _breaks = Native.CopyArray<BreakPeriod>(pointer, count);
                }
                return _breaks;
            }
        }

        public void Dispose() {
            handle.Dispose();
            GC.SuppressFinalize(this);
        }
    }
}
