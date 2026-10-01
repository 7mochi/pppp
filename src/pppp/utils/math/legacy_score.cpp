#include "pppp/utils/math/legacy_score.h"
#include <cstdio>
#include <cstdlib>

namespace pppp { namespace utils { namespace math {
    pppp_uint64 decimal15_scaled(double x, bool* negative) {
        char buf[64];
        std::sprintf(buf, "%.15g", x);

        *negative = false;
        const char* s = buf;
        if (*s == '-') {
            *negative = true;
            s++;
        }
        pppp_uint64 value = 0;
        int frac_digits = 0;
        bool in_frac = false;
        int exp10 = 0;
        for (; *s; s++) {
            if (*s == '.') {
                in_frac = true;
                continue;
            }
            if (*s == 'e' || *s == 'E') {
                exp10 = std::atoi(s + 1);
                break;
            }
            value = value * 10 + static_cast<pppp_uint64>(*s - '0');
            if (in_frac) {
                frac_digits++;
            }
        }
        int shift = 14 - frac_digits + exp10;
        while (shift > 0) {
            value *= 10;
            shift--;
        }
        while (shift < 0) {
            value /= 10;
            shift++;
        }
        return value;
    }
}}} // namespace pppp::utils::math
