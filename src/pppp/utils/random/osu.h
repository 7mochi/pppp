// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_UTILS_RANDOM_OSU_H
#define PPPP_UTILS_RANDOM_OSU_H

namespace pppp { namespace utils { namespace random {
    const unsigned MASK32 = 0xFFFFFFFFu;
    const unsigned INT_MASK = 0x7FFFFFFFu;
    const double INT_TO_REAL = 1.0 / 2147483648.0; // 1 / 2^31

    /// A four-word xorshift pseudo-random generator, kept for the draws that have to reproduce the
    /// game's legacy sequence rather than a better one. The game knows it as `FastRandom`, and it is
    /// the only generator whose draws a legacy beatmap's object placement can be read back from.
    ///
    /// The state is 32 bits wide by contract, and `unsigned` is only guaranteed to be at least
    /// that, so every write back into it is masked.
    class LegacyRandom {
    public:
        unsigned x;
        unsigned y;
        unsigned z;
        unsigned w;

        explicit LegacyRandom(int seed);

        /// Generates a random unsigned integer within the range [0, 4294967295).
        /// @returns The random value.
        unsigned next_uint();

        /// Generates a random integer value within the range [0, 2147483647).
        /// @returns The random value.
        int next();

        /// Generates a random integer value within the range [0, `upper_bound`).
        /// @param upper_bound The upper bound.
        /// @returns The random value.
        int next(int upper_bound);

        /// Generates a random integer value within the range [`lower_bound`, `upper_bound`).
        /// @param lower_bound The lower bound of the range.
        /// @param upper_bound The upper bound of the range.
        /// @returns The random value.
        int next(int lower_bound, int upper_bound);

        /// Generates a random integer value within the range [`lower_bound`, `upper_bound`).
        /// @param lower_bound The lower bound of the range.
        /// @param upper_bound The upper bound of the range.
        /// @returns The random value.
        int next(double lower_bound, double upper_bound);

        /// Generates a random double value within the range [0, 1).
        /// @returns The random value.
        double next_double();

        /// Generates a reandom boolean value. Cached such that a random value is only generated once
        /// in every 32 calls.
        /// @returns The random value.
        bool next_bool();

    private:
        unsigned bit_buffer;
        int bit_index;
    };
}}} // namespace pppp::utils::random

// The generators are seeded across every ruleset, so the type is re-exported under its short name.
namespace pppp { namespace utils {
    typedef random::LegacyRandom LegacyRandom;
}} // namespace pppp::utils

#endif
