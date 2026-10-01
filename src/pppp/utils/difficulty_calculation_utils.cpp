// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <cmath>
#include <limits>

namespace pppp { namespace utils {
    double bpm_to_milliseconds(double bpm, int delimiter) { return 60000.0 / delimiter / bpm; }

    double milliseconds_to_bpm(double ms, int delimiter) { return 60000.0 / (ms * delimiter); }

    double logistic(double x, double midpoint, double multiplier, double max_value) {
        return max_value / (1.0 + std::exp(multiplier * (midpoint - x)));
    }

    double logistic(double exponent, double max_value) { return max_value / (1.0 + std::exp(exponent)); }

    double norm(double p, double v1, double v2) {
        double s = pow(v1, p) + pow(v2, p);
        return pow(s, 1.0 / p);
    }

    double norm(double p, double v1, double v2, double v3) {
        double s = pow(v1, p) + pow(v2, p) + pow(v3, p);
        return pow(s, 1.0 / p);
    }

    double norm(double p, double v1, double v2, double v3, double v4) {
        double s = pow(v1, p) + pow(v2, p) + pow(v3, p) + pow(v4, p);
        return pow(s, 1.0 / p);
    }

    double bell_curve(double x, double mean, double width, double multiplier) {
        return multiplier * std::exp(M_E * -(pow(x - mean, 2) / pow(width, 2)));
    }

    double smoothstep_bell_curve(double x, double mean, double width) {
        x -= mean;
        x = x > 0.0 ? (width - x) : (width + x);
        return smoothstep(x, 0.0, width);
    }

    double smoothstep(double x, double start, double end_val) {
        x = math::clamp((x - start) / (end_val - start), 0.0, 1.0);
        return x * x * (3.0 - 2.0 * x);
    }

    double smootherstep(double x, double start, double end_val) {
        x = math::clamp((x - start) / (end_val - start), 0.0, 1.0);
        return x * x * x * (x * (6.0 * x - 15.0) + 10.0);
    }

    double reverse_lerp(double x, double start, double end_val) {
        return math::clamp((x - start) / (end_val - start), 0.0, 1.0);
    }

    double erf(double x) {
        if (x == 0.0) {
            return 0.0;
        }
        if (x == std::numeric_limits<double>::infinity()) {
            return 1.0;
        }
        if (x == -std::numeric_limits<double>::infinity()) {
            return -1.0;
        }
        if (x != x) {
            return x;
        }

        // Constants for approximation (Abramowitz and Stegun formula 7.1.26)
        double t = 1.0 / (1.0 + 0.3275911 * std::fabs(x));
        double tau =
            t * (0.254829592 + t * (-0.284496736 + t * (1.421413741 + t * (-1.453152027 + t * 1.061405429))));
        double result = 1.0 - tau * std::exp(-x * x);
        return x >= 0.0 ? result : -result;
    }

    double erf_inv(double x) {
        if (x <= -1.0) {
            return -std::numeric_limits<double>::infinity();
        }
        if (x >= 1.0) {
            return std::numeric_limits<double>::infinity();
        }
        if (x == 0.0) {
            return 0.0;
        }

        const double a = 0.147;
        double sgn = (x > 0.0) ? 1.0 : -1.0;
        x = std::fabs(x);

        double ln = std::log(1 - x * x);
        double t1 = 2 / (PI_D * a) + ln / 2;
        double t2 = ln / a;
        double base_approx = std::sqrt(t1 * t1 - t2) - t1;

        // Correction reduces max error from -0.005 to -0.00045.
        double c = x >= 0.85 ? pow((x - 0.85) / 0.293, 8) : 0;
        return sgn * (std::sqrt(base_approx) + c);
    }

    // In actual debug testing it's very rare for a (double, double) call to end up with a rounded
    // int value in the first place. Making an explicit overload is slightly faster than running
    // the `switch` in such cases.
    double pow(double x, double exponent) { return std::pow(x, exponent); }

    double pow(double x, int exponent) {
        switch (exponent) {
        case 0: return 1.0;
        case 1: return x;
        case 2: return x * x;
        case 3: return x * x * x;
        case 4: return x * x * x * x;
        case 5: return x * x * x * x * x; // This is the largest value used in diffcalc right now.
        default: return std::pow(x, static_cast<double>(exponent));
        }
    }
}} // namespace pppp::utils
