namespace Pppp {
    public struct BeatmapDifficulty {
        private readonly double drainRate;
        private readonly double circleSize;
        private readonly double overallDifficulty;
        private readonly double approachRate;
        private readonly double sliderMultiplier;
        private readonly double sliderTickRate;

        internal BeatmapDifficulty(double drainRate, double circleSize, double overallDifficulty,
                                   double approachRate, double sliderMultiplier, double sliderTickRate) {
            this.drainRate = drainRate;
            this.circleSize = circleSize;
            this.overallDifficulty = overallDifficulty;
            this.approachRate = approachRate;
            this.sliderMultiplier = sliderMultiplier;
            this.sliderTickRate = sliderTickRate;
        }

        public double DrainRate {
            get { return drainRate; }
        }

        public double CircleSize {
            get { return circleSize; }
        }

        public double OverallDifficulty {
            get { return overallDifficulty; }
        }

        public double ApproachRate {
            get { return approachRate; }
        }

        public double SliderMultiplier {
            get { return sliderMultiplier; }
        }

        public double SliderTickRate {
            get { return sliderTickRate; }
        }
    }
}
