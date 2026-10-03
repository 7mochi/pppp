using System;
using System.Runtime.InteropServices;

namespace Pppp {
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2 {
        public float x;
        public float y;
    }

    /// <summary>A representation of all top-level difficulty settings for a beatmap.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct BeatmapDifficulty {
        /// <summary>The drain rate of the associated beatmap.</summary>
        public double drain_rate;

        /// <summary>The circle size of the associated beatmap.</summary>
        public double circle_size;

        /// <summary>The overall difficulty of the associated beatmap.</summary>
        public double overall_difficulty;

        /// <summary>The approach rate of the associated beatmap.</summary>
        public double approach_rate;

        /// <summary>
        /// The base slider velocity of the associated beatmap. This was known as "SliderMultiplier"
        /// in the .osu format and stable editor.
        /// </summary>
        public double slider_multiplier;

        /// <summary>The slider tick rate of the associated beatmap.</summary>
        public double slider_tick_rate;
    }

    /// <summary>A HitObject describes an object in a Beatmap.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct HitObject {
        public Vector2 position;
        public uint type;
        public uint hitsound;

        /// <summary>The time at which the HitObject starts.</summary>
        public double start_time;

        public double end_time;
        [MarshalAs(UnmanagedType.Bool)]
        public bool new_combo;
        public int combo_offset;
        public int slider;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TimingPoint {
        public double time;

        /// <summary>The beat length at this control point.</summary>
        public double beat_length;

        /// <summary>The time signature at this control point.</summary>
        public int meter;

        [MarshalAs(UnmanagedType.Bool)]
        public bool uninherited;
        public uint effects;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct BreakPeriod {
        /// <summary>The break start time.</summary>
        public double start_time;

        /// <summary>The break end time.</summary>
        public double end_time;
    }

    public enum SliderEventType {
        Tick = 0,

        /// <summary>Occurs just before the tail. Should generally be ignored.</summary>
        LegacyLastTick = 1,
        Head = 2,
        Tail = 3,
        Repeat = 4
    }

    /// <summary>Describes a point in time on a slider given special meaning. Should be used by
    /// rulesets to visualise the slider.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SliderEventDescriptor {
        /// <summary>The type of event.</summary>
        public int type;

        /// <summary>The time of this event.</summary>
        public double time;

        /// <summary>
        /// The zero-based index of the span. In the case of repeat sliders, this will increase after
        /// each repeat.
        /// </summary>
        public int span_index;

        /// <summary>The time at which the contained span_index begins.</summary>
        public double span_start_time;

        /// <summary>The progress along the slider's path at which this event occurs.</summary>
        public double path_progress;

        /// <summary>The position on the path at path_progress, resolved once when the descriptor is built.</summary>
        public Vector2 position;
    }

    public sealed class Slider {
        internal Slider(int slides, double expected_length, uint[] node_sounds, Vector2[] control_points,
                        Vector2[] path, double[] cumulative_lengths, Vector2[] undecimated_path,
                        double[] undecimated_cumulative_lengths, SliderEventDescriptor[] events,
                        SliderEventDescriptor[] catch_events) {
            this.slides = slides;
            this.expected_length = expected_length;
            this.node_sounds = node_sounds;
            this.control_points = control_points;
            this.path = path;
            this.cumulative_lengths = cumulative_lengths;
            this.undecimated_path = undecimated_path;
            this.undecimated_cumulative_lengths = undecimated_cumulative_lengths;
            this.events = events;
            this.catch_events = catch_events;
        }

        public int slides { get; private set; }
        public double expected_length { get; private set; }
        public uint[] node_sounds { get; private set; }
        public Vector2[] control_points { get; private set; }
        public Vector2[] path { get; private set; }
        public double[] cumulative_lengths { get; private set; }
        public Vector2[] undecimated_path { get; private set; }
        public double[] undecimated_cumulative_lengths { get; private set; }
        public SliderEventDescriptor[] events { get; private set; }
        public SliderEventDescriptor[] catch_events { get; private set; }
    }
}
