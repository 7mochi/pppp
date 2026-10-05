namespace Pppp {
    public enum HitResult {
        /// <summary>
        /// Indicates that the object has not been judged yet.
        /// </summary>
        None = 0,

        /// <summary>
        /// Indicates that the object has been judged as a miss.
        /// </summary>
        /// <remarks>
        /// This miss window should determine how early a hit can be before it is considered for judgement (as opposed to being ignored as
        /// "too far in the future"). It should also define when a forced miss should be triggered (as a result of no user input in time).
        /// </remarks>
        Miss,

        Meh,
        Ok,
        Good,
        Great,

        /// <summary>
        /// This is an optional timing window tighter than <see cref="Great"/>.
        /// </summary>
        /// <remarks>
        /// By default, this does not give any bonus accuracy or score.
        /// To have it affect scoring, consider adding a nested bonus object.
        /// </remarks>
        Perfect,

        /// <summary>
        /// Indicates small tick miss.
        /// </summary>
        SmallTickMiss,

        /// <summary>
        /// Indicates a small tick hit.
        /// </summary>
        SmallTickHit,

        /// <summary>
        /// Indicates a large tick miss.
        /// </summary>
        LargeTickMiss,

        /// <summary>
        /// Indicates a large tick hit.
        /// </summary>
        LargeTickHit,

        /// <summary>
        /// Indicates a small bonus.
        /// </summary>
        SmallBonus,

        /// <summary>
        /// Indicates a large bonus.
        /// </summary>
        LargeBonus,

        /// <summary>
        /// Indicates a miss that should be ignored for scoring purposes.
        /// </summary>
        IgnoreMiss,

        /// <summary>
        /// Indicates a hit that should be ignored for scoring purposes.
        /// </summary>
        IgnoreHit,

        /// <summary>
        /// Indicates that a combo break should occur, but does not otherwise affect score.
        /// </summary>
        /// <remarks>
        /// May be paired with <see cref="IgnoreHit"/>.
        /// </remarks>
        ComboBreak,

        /// <summary>
        /// A special tick judgement to increase the valuation of the final tick of a slider.
        /// The default minimum result is <see cref="IgnoreMiss"/>, but may be overridden to <see cref="LargeTickMiss"/>.
        /// </summary>
        SliderTailHit,

        /// <summary>
        /// A special result used as a padding value for legacy rulesets. It is a hit type and affects combo, but does not affect the base score (does not affect accuracy).
        ///
        /// DO NOT USE FOR ANYTHING EVER.
        /// </summary>
        /// <remarks>
        /// This is used when dealing with legacy scores, which historically only have counts stored for 300/100/50/miss.
        /// For these scores, we pad the hit statistics with `LegacyComboIncrease` to meet the correct max combo for the score.
        /// </remarks>
        LegacyComboIncrease
    }
}
