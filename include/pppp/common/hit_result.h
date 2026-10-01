// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_COMMON_HIT_RESULT_H
#define PPPP_COMMON_HIT_RESULT_H

namespace pppp { namespace common {
    enum HitResult {
        /// Indicates that the object has not been judged yet.
        HIT_RESULT_NONE = 0,

        /// Indicates that the object has been judged as a miss.
        /// @remarks This miss window should determine how early a hit can be before it is considered for
        /// judgement (as opposed to being ignored as "too far in the future"). It should also define when a
        /// forced miss should be triggered (as a result of no user input in time).
        HIT_RESULT_MISS,

        HIT_RESULT_MEH,
        HIT_RESULT_OK,
        HIT_RESULT_GOOD,
        HIT_RESULT_GREAT,

        /// This is an optional timing window tighter than great. By default it gives no bonus
        /// accuracy or score; to have it affect scoring, consider adding a nested bonus object.
        HIT_RESULT_PERFECT,

        /// Indicates small tick miss.
        HIT_RESULT_SMALL_TICK_MISS,

        /// Indicates a small tick hit.
        HIT_RESULT_SMALL_TICK_HIT,

        /// Indicates a large tick miss.
        HIT_RESULT_LARGE_TICK_MISS,

        /// Indicates a large tick hit.
        HIT_RESULT_LARGE_TICK_HIT,

        /// Indicates a small bonus.
        HIT_RESULT_SMALL_BONUS,

        /// Indicates a large bonus.
        HIT_RESULT_LARGE_BONUS,

        /// Indicates a miss that should be ignored for scoring purposes.
        HIT_RESULT_IGNORE_MISS,

        /// Indicates a hit that should be ignored for scoring purposes.
        HIT_RESULT_IGNORE_HIT,

        /// Indicates that a combo break should occur, but does not otherwise affect score.
        /// @remarks May be paired with `HIT_RESULT_IGNORE_HIT`.
        HIT_RESULT_COMBO_BREAK,

        /// A special tick judgement to increase the valuation of the final tick of a slider.
        /// The default minimum result is `HIT_RESULT_IGNORE_MISS`, but may be overridden to
        /// `HIT_RESULT_LARGE_TICK_MISS`.
        HIT_RESULT_SLIDER_TAIL_HIT,

        /// A special result used as a padding value for legacy rulesets. It is a hit type and affects
        /// combo, but does not affect the base score, so it does not affect accuracy.
        /// DO NOT USE FOR ANYTHING EVER.
        /// @remarks This is used when dealing with legacy scores, which historically only have counts
        /// stored for 300/100/50/miss.
        /// For these scores, we pad the hit statistics with `LegacyComboIncrease` to meet the correct max
        /// combo for the score.
        HIT_RESULT_LEGACY_COMBO_INCREASE
    };

    const int HIT_RESULT_COUNT = 18;
}} // namespace pppp::common

#endif
