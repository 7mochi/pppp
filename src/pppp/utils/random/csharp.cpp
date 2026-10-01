#include "pppp/utils/random/csharp.h"
#include "pppp/config.h"
#include <climits>

namespace pppp { namespace utils { namespace random {
    DotNetRandom::DotNetRandom(int seed)
        : inext(0),
          inextp(21) {
        for (int i = 0; i < 56; i++) {
            seed_array[i] = 0;
        }

        int subtraction = (seed == INT_MIN) ? INT_MAX : (seed < 0 ? -seed : seed);
        int mj = MSEED - subtraction;
        seed_array[55] = mj;
        int mk = 1;
        int ii = 0;
        for (int i = 1; i < 55; i++) {
            if ((ii += 21) >= 55) {
                ii -= 55;
            }
            seed_array[ii] = mk;
            mk = mj - mk;
            if (mk < 0) {
                mk += MBIG;
            }
            mj = seed_array[ii];
        }
        for (int k = 1; k < 5; k++) {
            for (int i = 1; i < 56; i++) {
                int n = i + 30;
                if (n >= 55) {
                    n -= 55;
                }
                seed_array[i] -= seed_array[1 + n];
                if (seed_array[i] < 0) {
                    seed_array[i] += MBIG;
                }
            }
        }
    }

    int DotNetRandom::internal_sample() {
        int loc_inext = inext;
        int loc_inextp = inextp;

        if (++loc_inext >= 56) {
            loc_inext = 1;
        }
        if (++loc_inextp >= 56) {
            loc_inextp = 1;
        }

        int ret = seed_array[loc_inext] - seed_array[loc_inextp];
        if (ret == MBIG) {
            ret--;
        }
        if (ret < 0) {
            ret += MBIG;
        }

        seed_array[loc_inext] = ret;
        inext = loc_inext;
        inextp = loc_inextp;
        return ret;
    }

    double DotNetRandom::sample() { return internal_sample() * (1.0 / MBIG); }

    double DotNetRandom::sample_for_large_range() {
        int result = internal_sample();
        bool negative = internal_sample() % 2 == 0;
        if (negative) {
            result = -result;
        }
        double d = result;
        d += INT_MAX - 1;
        d /= 2u * (unsigned)INT_MAX - 1;
        return d;
    }

    int DotNetRandom::next() { return internal_sample(); }

    int DotNetRandom::next(int max_value) { return static_cast<int>(sample() * max_value); }

    int DotNetRandom::next(int min_value, int max_value) {
        pppp_int64 range = static_cast<pppp_int64>(max_value) - min_value;
        if (range <= INT_MAX) {
            return static_cast<int>(sample() * static_cast<double>(range)) + min_value;
        }
        return static_cast<int>(
            static_cast<pppp_int64>(sample_for_large_range() * static_cast<double>(range)) + min_value);
    }

    double DotNetRandom::next_double() { return sample(); }
}}} // namespace pppp::utils::random
