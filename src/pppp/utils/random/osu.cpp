// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/utils/random/osu.h"

namespace pppp { namespace utils { namespace random {
    LegacyRandom::LegacyRandom(int seed)
        : x(static_cast<unsigned>(seed) & MASK32),
          y(842502087u),
          z(3579807591u),
          w(273326509u),
          bit_buffer(0),
          bit_index(32) {}

    unsigned LegacyRandom::next_uint() {
        unsigned t = (x ^ ((x << 11) & MASK32)) & MASK32;
        x = y;
        y = z;
        z = w;
        w = (w ^ (w >> 19) ^ t ^ (t >> 8)) & MASK32;
        return w;
    }

    int LegacyRandom::next() { return static_cast<int>(INT_MASK & next_uint()); }

    double LegacyRandom::next_double() { return INT_TO_REAL * next(); }

    int LegacyRandom::next(int upper_bound) { return static_cast<int>(next_double() * upper_bound); }

    int LegacyRandom::next(int lower_bound, int upper_bound) {
        return static_cast<int>(lower_bound + next_double() * (upper_bound - lower_bound));
    }

    int LegacyRandom::next(double lower_bound, double upper_bound) {
        return static_cast<int>(lower_bound + next_double() * (upper_bound - lower_bound));
    }

    bool LegacyRandom::next_bool() {
        if (bit_index == 32) {
            bit_buffer = next_uint();
            bit_index = 1;
            return (bit_buffer & 1) == 1;
        }

        bit_index++;
        bit_buffer >>= 1;
        return (bit_buffer & 1) == 1;
    }
}}} // namespace pppp::utils::random
