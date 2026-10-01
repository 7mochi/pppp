#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <cmath>

namespace pppp { namespace utils { namespace math {
    double clamp(double x, double lo, double hi) {
        if (x < lo) {
            return lo;
        }
        if (x > hi) {
            return hi;
        }
        return x;
    }

    double lerp(double a, double b, double t) { return a + (b - a) * t; }

    double lerp_dotnet(double a, double b, double t) { return (a * (1.0 - t)) + (b * t); }

    double degrees_to_radians(double deg) { return deg * PI_D / 180.0; }

    double cbrt(double x) {
        // Zero, the infinities and NaN come back unchanged; `frexp` leaves their exponent unspecified.
        if (x == 0.0 || !(x - x == 0.0)) {
            return x + x;
        }

        int xe = 0;
        const double xm = std::frexp(std::fabs(x), &xe);

        const double u =
            (0.354895765043919860 +
             ((1.50819193781584896 +
               ((-2.11499494167371287 +
                 ((2.44693122563534430 +
                   ((-1.83469277483613086 + (0.784932344976639262 - 0.145263899385486377 * xm) * xm) * xm)) *
                  xm)) *
                xm)) *
              xm));

        const double t2 = u * u * u;

        // glibc divides a negative exponent towards zero, which C++98 leaves to the implementation.
        const int xe_div3 = xe >= 0 ? xe / 3 : -(-xe / 3);
        const int xe_mod3 = xe >= 0 ? xe % 3 : -(-xe % 3);

        const double ym = u * (t2 + 2.0 * xm) / (2.0 * t2 + xm) * CBRT_FACTOR[2 + xe_mod3];

        return std::ldexp(x > 0.0 ? ym : -ym, xe_div3);
    }
}}} // namespace pppp::utils::math
