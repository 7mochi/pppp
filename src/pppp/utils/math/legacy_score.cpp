#include "pppp/utils/math/legacy_score.h"
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

namespace pppp { namespace utils { namespace math {
    pppp_uint64 decimal15_scaled(double x, bool* negative) {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::setprecision(15) << x;
        const std::string text = out.str();

        *negative = false;
        const char* s = text.c_str();
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
