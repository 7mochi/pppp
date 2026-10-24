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
            Native.pppp_beatmap_free((pppp_beatmap*)handle);
            return true;
        }
    }

    /// <summary>A loaded beatmap.</summary>
    public sealed unsafe class Beatmap : IDisposable {
        private readonly BeatmapHandle handle;

        private bool prepared;
        private HitObject[] hitObjects;
        private Slider[] sliders;
        private TimingPoint[] timingPoints;
        private BreakPeriod[] breaks;

        private Beatmap(BeatmapHandle handle) { this.handle = handle; }

        private pppp_beatmap* Map {
            get {
                if (handle.IsClosed) {
                    throw new ObjectDisposedException("Beatmap");
                }
                return (pppp_beatmap*)handle.DangerousGetHandle();
            }
        }

        private void EnsurePrepared() {
            if (handle.IsClosed) {
                throw new ObjectDisposedException("Beatmap");
            }
            if (prepared) {
                return;
            }
            Native.Check(Native.pppp_beatmap_prepare_data(Map));
            prepared = true;
        }

        internal IntPtr Handle {
            get { return (IntPtr)Map; }
        }

        /// <summary>Parse a <see cref="Beatmap"/> by providing a path to a `.osu` file.</summary>
        public static Beatmap FromFile(string path) {
            if (path == null) {
                throw new ArgumentNullException("path");
            }
            return FromBytes(File.ReadAllBytes(path));
        }

        /// <summary>Parse a <see cref="Beatmap"/> by providing the content of a `.osu` file as a slice of bytes.</summary>
        public static unsafe Beatmap FromBytes(byte[] data) {
            if (data == null) {
                throw new ArgumentNullException("data");
            }
            pppp_beatmap* map;
            pppp_result status;
            fixed (byte* bytes = data) {
                status = Native.pppp_beatmap_from_bytes(bytes, new UIntPtr((ulong)data.Length), &map);
            }
            if (status == pppp_result.PPPP_ALLOCATION) {
                throw new OutOfMemoryException();
            }
            if (status != pppp_result.PPPP_OK) {
                throw new InvalidDataException("cannot parse the beatmap");
            }
            return new Beatmap(new BeatmapHandle((IntPtr)map));
        }

        public int FormatVersion {
            get { return Native.pppp_beatmap_format_version(Map); }
        }

        public int Mode {
            get { return Native.pppp_beatmap_mode(Map); }
        }

        public double StackLeniency {
            get { return Native.pppp_beatmap_stack_leniency(Map); }
        }

        /// <summary>A representation of all top-level difficulty settings for a beatmap.</summary>
        public BeatmapDifficulty Difficulty {
            get {
                BeatmapDifficulty settings;
                // The public struct is the ABI struct field for field; it adds read-only
                // properties over private fields.
                Native.Check(Native.pppp_beatmap_get_difficulty(Map, (pppp_beatmap_difficulty*)&settings));
                return settings;
            }
        }

        public HitObject[] HitObjects {
            get {
                EnsurePrepared();
                if (hitObjects == null) {
                    pppp_hit_object* pointer;
                    UIntPtr count;
                    Native.Check(Native.pppp_beatmap_hit_objects(Map, &pointer, &count));
                    hitObjects = Native.CopyArray<HitObject>((IntPtr)pointer, count);
                }
                return hitObjects;
            }
        }

        public Slider[] Sliders {
            get {
                EnsurePrepared();
                if (sliders == null) {
                    pppp_slider* pointer;
                    UIntPtr count;
                    Native.Check(Native.pppp_beatmap_sliders(Map, &pointer, &count));
                    pppp_slider[] views = Native.CopyArray<pppp_slider>((IntPtr)pointer, count);
                    sliders = new Slider[views.Length];
                    for (int i = 0; i < views.Length; i++) {
                        sliders[i] = new Slider(views[i].slides, views[i].expected_length,
                                                Native.CopyUnsigned((IntPtr)views[i].node_sounds,
                                                                    views[i].node_sound_count),
                                                Native.CopyArray<Vector2>((IntPtr)views[i].control_points,
                                                                          views[i].control_point_count),
                                                Native.CopyArray<Vector2>((IntPtr)views[i].path,
                                                                          views[i].path_count),
                                                Native.CopyDoubles((IntPtr)views[i].cumulative_lengths,
                                                                   views[i].cumulative_length_count),
                                                Native.CopyArray<Vector2>((IntPtr)views[i].undecimated_path,
                                                                          views[i].undecimated_path_count),
                                                Native.CopyDoubles((IntPtr)views[i].undecimated_cumulative_lengths,
                                                                   views[i].undecimated_cumulative_length_count),
                                                Native.CopyArray<SliderEventDescriptor>((IntPtr)views[i].events,
                                                                                        views[i].event_count),
                                                Native.CopyArray<SliderEventDescriptor>((IntPtr)views[i].catch_events,
                                                                                        views[i].catch_event_count));
                    }
                }
                return sliders;
            }
        }

        public TimingPoint[] TimingPoints {
            get {
                EnsurePrepared();
                if (timingPoints == null) {
                    pppp_timing_point* pointer;
                    UIntPtr count;
                    Native.Check(Native.pppp_beatmap_timing_points(Map, &pointer, &count));
                    timingPoints = Native.CopyArray<TimingPoint>((IntPtr)pointer, count);
                }
                return timingPoints;
            }
        }

        public BreakPeriod[] Breaks {
            get {
                EnsurePrepared();
                if (breaks == null) {
                    pppp_break_period* pointer;
                    UIntPtr count;
                    Native.Check(Native.pppp_beatmap_breaks(Map, &pointer, &count));
                    breaks = Native.CopyArray<BreakPeriod>((IntPtr)pointer, count);
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
