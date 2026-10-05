namespace Pppp {
    /// <summary>The result of calculating the strains on a map.
    ///
    /// Suitable to plot the difficulty of a map over time.</summary>
    public abstract class Strains {
        internal Strains(double startTime, double sectionLength) {
            StartTime = startTime;
            SectionLength = sectionLength;
        }

        public abstract Ruleset Ruleset { get; }

        public double StartTime { get; private set; }

        public double SectionLength { get; private set; }
    }

    /// <summary>The result of calculating the strains on a osu! map.
    ///
    /// Suitable to plot the difficulty of a map over time.</summary>
    public sealed class OsuStrains : Strains {
        internal OsuStrains(double startTime, double sectionLength, Native.OsuStrains value)
            : base(startTime, sectionLength) {
            Aim = Native.CopyDoubles(value.aim, value.aim_count);
            AimNoSliders = Native.CopyDoubles(value.aim_no_sliders, value.aim_no_sliders_count);
            Speed = Native.CopyDoubles(value.speed, value.speed_count);
            Reading = Native.CopyDoubles(value.reading, value.reading_count);
            Flashlight = Native.CopyDoubles(value.flashlight, value.flashlight_count);
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        public double[] Aim { get; private set; }

        public double[] AimNoSliders { get; private set; }

        public double[] Speed { get; private set; }

        public double[] Reading { get; private set; }

        public double[] Flashlight { get; private set; }
    }

    /// <summary>The result of calculating the strains on a osu!taiko map.
    ///
    /// Suitable to plot the difficulty of a map over time.</summary>
    public sealed class TaikoStrains : Strains {
        internal TaikoStrains(double startTime, double sectionLength, Native.TaikoStrains value)
            : base(startTime, sectionLength) {
            Colour = Native.CopyDoubles(value.colour, value.colour_count);
            Reading = Native.CopyDoubles(value.reading, value.reading_count);
            Rhythm = Native.CopyDoubles(value.rhythm, value.rhythm_count);
            Stamina = Native.CopyDoubles(value.stamina, value.stamina_count);
            SingleColourStamina = Native.CopyDoubles(value.single_colour_stamina, value.single_colour_stamina_count);
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        public double[] Colour { get; private set; }

        public double[] Reading { get; private set; }

        public double[] Rhythm { get; private set; }

        public double[] Stamina { get; private set; }

        public double[] SingleColourStamina { get; private set; }
    }

    /// <summary>The result of calculating the strains on a osu!catch map.
    ///
    /// Suitable to plot the difficulty of a map over time.</summary>
    public sealed class CatchStrains : Strains {
        internal CatchStrains(double startTime, double sectionLength, Native.CatchStrains value)
            : base(startTime, sectionLength) {
            Movement = Native.CopyDoubles(value.movement, value.movement_count);
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }

        public double[] Movement { get; private set; }
    }

    /// <summary>The result of calculating the strains on a osu!mania map.
    ///
    /// Suitable to plot the difficulty of a map over time.</summary>
    public sealed class ManiaStrains : Strains {
        internal ManiaStrains(double startTime, double sectionLength, Native.ManiaStrains value)
            : base(startTime, sectionLength) {
            Strain = Native.CopyDoubles(value.strain, value.strain_count);
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }

        public double[] Strain { get; private set; }
    }
}
