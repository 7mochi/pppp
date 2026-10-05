using System.Runtime.InteropServices;

#pragma warning disable 0649

namespace Pppp {
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2 {
        public float X;
        public float Y;
    }

    /// <summary>A representation of all top-level difficulty settings for a beatmap.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct BeatmapDifficulty {
        private double drain_rate;
        private double circle_size;
        private double overall_difficulty;
        private double approach_rate;
        private double slider_multiplier;
        private double slider_tick_rate;

        /// <summary>The drain rate of the associated beatmap.</summary>
        public double DrainRate {
            get { return drain_rate; }
        }

        /// <summary>The circle size of the associated beatmap.</summary>
        public double CircleSize {
            get { return circle_size; }
        }

        /// <summary>The overall difficulty of the associated beatmap.</summary>
        public double OverallDifficulty {
            get { return overall_difficulty; }
        }

        /// <summary>The approach rate of the associated beatmap.</summary>
        public double ApproachRate {
            get { return approach_rate; }
        }

        /// <summary>
        /// The base slider velocity of the associated beatmap. This was known as "SliderMultiplier"
        /// in the .osu format and stable editor.
        /// </summary>
        public double SliderMultiplier {
            get { return slider_multiplier; }
        }

        /// <summary>The slider tick rate of the associated beatmap.</summary>
        public double SliderTickRate {
            get { return slider_tick_rate; }
        }
    }

    /// <summary>A HitObject describes an object in a Beatmap.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct HitObject {
        private Vector2 position;
        private uint type;
        private uint hitsound;
        private double start_time;
        private double end_time;
        [MarshalAs(UnmanagedType.Bool)]
        private bool new_combo;
        private int combo_offset;
        private int slider;

        public Vector2 Position {
            get { return position; }
        }

        public uint Type {
            get { return type; }
        }

        public uint Hitsound {
            get { return hitsound; }
        }

        /// <summary>The time at which the HitObject starts.</summary>
        public double StartTime {
            get { return start_time; }
        }

        public double EndTime {
            get { return end_time; }
        }

        public bool NewCombo {
            get { return new_combo; }
        }

        public int ComboOffset {
            get { return combo_offset; }
        }

        public int Slider {
            get { return slider; }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TimingPoint {
        private double time;
        private double beat_length;
        private int meter;
        [MarshalAs(UnmanagedType.Bool)]
        private bool uninherited;
        private uint effects;

        public double Time {
            get { return time; }
        }

        /// <summary>The beat length at this control point.</summary>
        public double BeatLength {
            get { return beat_length; }
        }

        /// <summary>The time signature at this control point.</summary>
        public int Meter {
            get { return meter; }
        }

        public bool Uninherited {
            get { return uninherited; }
        }

        public uint Effects {
            get { return effects; }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct BreakPeriod {
        private double start_time;
        private double end_time;

        /// <summary>The break start time.</summary>
        public double StartTime {
            get { return start_time; }
        }

        /// <summary>The break end time.</summary>
        public double EndTime {
            get { return end_time; }
        }
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
        private int type;
        private double time;
        private int span_index;
        private double span_start_time;
        private double path_progress;
        private Vector2 position;

        /// <summary>The type of event.</summary>
        public SliderEventType Type {
            get { return (SliderEventType)type; }
        }

        /// <summary>The time of this event.</summary>
        public double Time {
            get { return time; }
        }

        /// <summary>
        /// The zero-based index of the span. In the case of repeat sliders, this will increase after
        /// each repeat.
        /// </summary>
        public int SpanIndex {
            get { return span_index; }
        }

        /// <summary>The time at which the contained <see cref="SpanIndex"/> begins.</summary>
        public double SpanStartTime {
            get { return span_start_time; }
        }

        /// <summary>The progress along the slider's path at which this event occurs.</summary>
        public double PathProgress {
            get { return path_progress; }
        }

        /// <summary>The position on the path at <see cref="PathProgress"/>, resolved once when the
        /// descriptor is built.</summary>
        public Vector2 Position {
            get { return position; }
        }
    }

    public sealed class Slider {
        internal Slider(int slides, double expectedLength, uint[] nodeSounds, Vector2[] controlPoints,
                        Vector2[] path, double[] cumulativeLengths, Vector2[] undecimatedPath,
                        double[] undecimatedCumulativeLengths, SliderEventDescriptor[] events,
                        SliderEventDescriptor[] catchEvents) {
            Slides = slides;
            ExpectedLength = expectedLength;
            NodeSounds = nodeSounds;
            ControlPoints = controlPoints;
            Path = path;
            CumulativeLengths = cumulativeLengths;
            UndecimatedPath = undecimatedPath;
            UndecimatedCumulativeLengths = undecimatedCumulativeLengths;
            Events = events;
            CatchEvents = catchEvents;
        }

        public int Slides { get; private set; }
        public double ExpectedLength { get; private set; }
        public uint[] NodeSounds { get; private set; }
        public Vector2[] ControlPoints { get; private set; }
        public Vector2[] Path { get; private set; }
        public double[] CumulativeLengths { get; private set; }
        public Vector2[] UndecimatedPath { get; private set; }
        public double[] UndecimatedCumulativeLengths { get; private set; }
        public SliderEventDescriptor[] Events { get; private set; }
        public SliderEventDescriptor[] CatchEvents { get; private set; }
    }
}
