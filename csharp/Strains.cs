namespace Pppp {
    public abstract class Strains {
        private readonly double startTime;
        private readonly double sectionLength;

        internal Strains(double startTime, double sectionLength) {
            this.startTime = startTime;
            this.sectionLength = sectionLength;
        }

        public abstract Ruleset Ruleset { get; }

        public double StartTime {
            get { return startTime; }
        }

        public double SectionLength {
            get { return sectionLength; }
        }
    }

    public sealed class OsuStrains : Strains {
        private readonly double[] aim;
        private readonly double[] aimNoSliders;
        private readonly double[] speed;
        private readonly double[] reading;
        private readonly double[] flashlight;

        internal OsuStrains(double startTime, double sectionLength, double[] aim, double[] aimNoSliders,
                            double[] speed, double[] reading, double[] flashlight)
            : base(startTime, sectionLength) {
            this.aim = aim;
            this.aimNoSliders = aimNoSliders;
            this.speed = speed;
            this.reading = reading;
            this.flashlight = flashlight;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Osu; }
        }

        public double[] Aim {
            get { return aim; }
        }

        public double[] AimNoSliders {
            get { return aimNoSliders; }
        }

        public double[] Speed {
            get { return speed; }
        }

        public double[] Reading {
            get { return reading; }
        }

        public double[] Flashlight {
            get { return flashlight; }
        }
    }

    public sealed class TaikoStrains : Strains {
        private readonly double[] colour;
        private readonly double[] reading;
        private readonly double[] rhythm;
        private readonly double[] stamina;
        private readonly double[] singleColourStamina;

        internal TaikoStrains(double startTime, double sectionLength, double[] colour, double[] reading,
                              double[] rhythm, double[] stamina, double[] singleColourStamina)
            : base(startTime, sectionLength) {
            this.colour = colour;
            this.reading = reading;
            this.rhythm = rhythm;
            this.stamina = stamina;
            this.singleColourStamina = singleColourStamina;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Taiko; }
        }

        public double[] Colour {
            get { return colour; }
        }

        public double[] Reading {
            get { return reading; }
        }

        public double[] Rhythm {
            get { return rhythm; }
        }

        public double[] Stamina {
            get { return stamina; }
        }

        public double[] SingleColourStamina {
            get { return singleColourStamina; }
        }
    }

    public sealed class CatchStrains : Strains {
        private readonly double[] movement;

        internal CatchStrains(double startTime, double sectionLength, double[] movement)
            : base(startTime, sectionLength) {
            this.movement = movement;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Catch; }
        }

        public double[] Movement {
            get { return movement; }
        }
    }

    public sealed class ManiaStrains : Strains {
        private readonly double[] strain;

        internal ManiaStrains(double startTime, double sectionLength, double[] strain)
            : base(startTime, sectionLength) {
            this.strain = strain;
        }

        public override Ruleset Ruleset {
            get { return Ruleset.Mania; }
        }

        public double[] Strain {
            get { return strain; }
        }
    }
}
