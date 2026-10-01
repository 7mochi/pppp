// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_UTILS_DIFFICULTY_RANGE_H
#define PPPP_UTILS_DIFFICULTY_RANGE_H

namespace pppp { namespace utils {
    /// Represents a piecewise-linear difficulty curve for a given gameplay quantity.
    struct DifficultyRange {
        /// Minimum of the resulting range which will be achieved by a difficulty value of 0.
        double min;
        /// Midpoint of the resulting range which will be achieved by a difficulty value of 5.
        double mid;
        /// Maximum of the resulting range which will be achieved by a difficulty value of 10.
        double max;

        DifficultyRange(double min_in, double mid_in, double max_in)
            : min(min_in),
              mid(mid_in),
              max(max_in) {}
    };

    /// Maps a difficulty value [0, 10] to a two-piece linear range of values.
    /// @param difficulty The difficulty value to be mapped.
    /// @param min_val Minimum of the resulting range which will be achieved by a difficulty value of 0.
    /// @param mid Midpoint of the resulting range which will be achieved by a difficulty value of 5.
    /// @param max_val Maximum of the resulting range which will be achieved by a difficulty value of 10.
    /// @returns Value to which the difficulty value maps in the specified range.
    double difficulty_range(double difficulty, double min_val, double mid, double max_val);

    /// Maps a difficulty value [0, 10] to a linear range of [-1, 1].
    /// @param difficulty The difficulty value to be mapped.
    /// @returns Value to which the difficulty value maps in the specified range.
    double difficulty_range(double difficulty);

    /// Maps a difficulty value [0, 10] to a two-piece linear range of values.
    /// @param difficulty The difficulty value to be mapped.
    /// @param range The values that define the two linear ranges, where min is the minimum of the
    /// resulting range which will be achieved by a difficulty value of 0, mid is the midpoint
    /// achieved by a difficulty value of 5 and max is the maximum achieved by a difficulty value of
    /// 10.
    /// @returns Value to which the difficulty value maps in the specified range.
    double difficulty_range(double difficulty, const DifficultyRange& range);

    /// Maps a difficulty value [0, 10] to a two-piece linear range of values. Floors the value to
    /// `int`, usually to match osu!stable spec.
    /// @param difficulty The difficulty value to be mapped.
    /// @param range The values that define the two linear ranges, where min is the minimum of the
    /// resulting range which will be achieved by a difficulty value of 0, mid is the midpoint
    /// achieved by a difficulty value of 5 and max is the maximum achieved by a difficulty value of
    /// 10.
    /// @returns Value to which the difficulty value maps in the specified range.
    int difficulty_range_int(double difficulty, const DifficultyRange& range);

    /// Inverse function to difficulty_range(double,double,double,double).
    /// Maps a value returned by the function above back to the difficulty that produced it.
    /// @param difficulty_value The difficulty-dependent value to be unmapped.
    /// @param diff0 Minimum of the resulting range which will be achieved by a difficulty value of 0.
    /// @param diff5 Midpoint of the resulting range which will be achieved by a difficulty value of 5.
    /// @param diff10 Maximum of the resulting range which will be achieved by a difficulty value of 10.
    /// @returns Value to which the difficulty value maps in the specified range.
    double inverse_difficulty_range(double difficulty_value, double diff0, double diff5, double diff10);
}} // namespace pppp::utils

#endif
