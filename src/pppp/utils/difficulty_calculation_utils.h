// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_UTILS_DIFFICULTY_CALCULATION_UTILS_H
#define PPPP_UTILS_DIFFICULTY_CALCULATION_UTILS_H

namespace pppp { namespace utils {
    /// Converts BPM value into milliseconds
    /// @param bpm Beats per minute
    /// @param delimiter Which rhythm delimiter to use, default is 1/4
    /// @returns BPM converted to milliseconds
    double bpm_to_milliseconds(double bpm, int delimiter = 4);

    /// Converts milliseconds value into a BPM value
    /// @param ms Milliseconds
    /// @param delimiter Which rhythm delimiter to use, default is 1/4
    /// @returns Milliseconds converted to beats per minute
    double milliseconds_to_bpm(double ms, int delimiter = 4);

    /// Calculates a S-shaped logistic function (https://en.wikipedia.org/wiki/Logistic_function)
    /// @param x Value to calculate the function for
    /// @param midpoint How much the function midpoint is offset from zero `x`
    /// @param multiplier Growth rate of the function
    /// @param max_value Maximum value returnable by the function
    /// @returns The output of logistic function of `x`
    double logistic(double x, double midpoint, double multiplier, double max_value = 1.0);

    /// Calculates a S-shaped logistic function (https://en.wikipedia.org/wiki/Logistic_function)
    /// @param exponent Exponent
    /// @param max_value Maximum value returnable by the function
    /// @returns The output of logistic function
    double logistic(double exponent, double max_value = 1.0);

    /// Returns the <i>p</i>-norm of an <i>n</i>-dimensional vector
    /// (https://en.wikipedia.org/wiki/Norm_(mathematics))
    /// @param p The value of <i>p</i> to calculate the norm for.
    /// @param v1 A coefficient of the vector.
    /// @param v2 A coefficient of the vector.
    /// @returns The <i>p</i>-norm of the vector.
    double norm(double p, double v1, double v2);

    /// Returns the <i>p</i>-norm of an <i>n</i>-dimensional vector
    /// (https://en.wikipedia.org/wiki/Norm_(mathematics))
    /// @param p The value of <i>p</i> to calculate the norm for.
    /// @param v1 A coefficient of the vector.
    /// @param v2 A coefficient of the vector.
    /// @param v3 A coefficient of the vector.
    /// @returns The <i>p</i>-norm of the vector.
    double norm(double p, double v1, double v2, double v3);

    /// Returns the <i>p</i>-norm of an <i>n</i>-dimensional vector
    /// (https://en.wikipedia.org/wiki/Norm_(mathematics))
    /// @param p The value of <i>p</i> to calculate the norm for.
    /// @param v1 A coefficient of the vector.
    /// @param v2 A coefficient of the vector.
    /// @param v3 A coefficient of the vector.
    /// @param v4 A coefficient of the vector.
    /// @returns The <i>p</i>-norm of the vector.
    double norm(double p, double v1, double v2, double v3, double v4);

    /// Calculates a Gaussian-based bell curve function (https://en.wikipedia.org/wiki/Gaussian_function)
    /// @param x Value to calculate the function for
    /// @param mean The mean (center) of the bell curve
    /// @param width The width (spread) of the curve
    /// @param multiplier Multiplier to adjust the curve's height
    /// @returns The output of the bell curve function of `x`
    double bell_curve(double x, double mean, double width, double multiplier = 1.0);

    /// Calculates a Smoothstep bell curve that returns 1 for x = mean, and smoothly reducing it's
    /// value to 0 over width
    /// @param x Value to calculate the function for
    /// @param mean Value of x, for which return value will be the highest (=1)
    /// @param width Range [mean - width, mean + width] where function will change values
    /// @returns The output of the smoothstep bell curve function of `x`
    double smoothstep_bell_curve(double x, double mean, double width);

    /// Smoothstep function (https://en.wikipedia.org/wiki/Smoothstep)
    /// @param x Value to calculate the function for
    /// @param start Value at which function returns 0
    /// @param end_val Value at which function returns 1
    double smoothstep(double x, double start, double end_val);

    /// Smootherstep function (https://en.wikipedia.org/wiki/Smoothstep#Variations)
    /// @param x Value to calculate the function for
    /// @param start Value at which function returns 0
    /// @param end_val Value at which function returns 1
    double smootherstep(double x, double start, double end_val);

    /// Reverse linear interpolation function (https://en.wikipedia.org/wiki/Linear_interpolation)
    /// @param x Value to calculate the function for
    /// @param start Value at which function returns 0
    /// @param end_val Value at which function returns 1
    double reverse_lerp(double x, double start, double end_val);

    /// Error function (https://en.wikipedia.org/wiki/Error_function)
    /// @param x Value to calculate the function for
    double erf(double x);

    /// Inverse error function (https://en.wikipedia.org/wiki/Error_function)
    /// @param x Value to calculate the function for
    double erf_inv(double x);

    double pow(double x, double exponent);
    double pow(double x, int exponent);

}} // namespace pppp::utils

#endif
