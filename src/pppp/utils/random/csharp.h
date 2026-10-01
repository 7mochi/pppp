#ifndef PPPP_UTILS_RANDOM_CSHARP_H
#define PPPP_UTILS_RANDOM_CSHARP_H

#include <climits>

namespace pppp { namespace utils { namespace random {
    /// The seeded `.NET` pseudo-random generator the game reaches for when a mod or a difficulty
    /// setting seeds a random: the algorithm of
    /// https://github.com/dotnet/runtime/blob/5535e31a712343a63f5d7d796cd874e563e5ac14/src/libraries/System.Private.CoreLib/src/System/Random.cs#L13,
    /// reproduced bit for bit. A seeded draw here has to land on the same value the game produced,
    /// because one wrong value moves every later draw and rearranges a beatmap's hit objects.
    ///
    /// The sequence comes from a subtractive generator, called `CompatPrng` in the framework and
    /// seeded through
    /// https://github.com/dotnet/runtime/blob/15872212c29cecc8d82da4548c3060f2614665f7/src/libraries/System.Private.CoreLib/src/System/Random.CompatImpl.cs#L256:
    /// the seed primes a 56-entry array off the golden-ratio constant, and the array is then wrung
    /// out four times over before the first draw.
    class DotNetRandom {
    public:
        /// The upper bound of the internal sample.
        static const int MBIG = INT_MAX;
        /// The constant the seed array's derivation starts from.
        static const int MSEED = 161803398;

        int seed_array[56];
        int inext;
        int inextp;

        /// Seeds the generator's 56-entry state array.
        /// @param seed The seed.
        explicit DotNetRandom(int seed);

        /// Draws the next value of the raw sequence, a value in `[1, MBIG)`.
        /// @returns The random value.
        int internal_sample();

        /// Draws a value in `[0, 1)`.
        /// @returns The random value.
        double sample();

        /// Draws a value in `[0, MBIG)`.
        /// @returns The random value.
        int next();

        /// Draws a value in `[0, max_value)`.
        /// @param max_value The exclusive upper bound.
        /// @returns The random value.
        int next(int max_value);

        /// Draws a value in `[min_value, max_value)`.
        /// @param min_value The inclusive lower bound.
        /// @param max_value The exclusive upper bound.
        /// @returns The random value.
        int next(int min_value, int max_value);

        /// Draws a value in `[0, 1)`.
        /// @returns The random value.
        double next_double();

    private:
        double sample_for_large_range();
    };
}}} // namespace pppp::utils::random

// The generators are seeded across every ruleset, so the type is re-exported under its short name.
namespace pppp { namespace utils {
    typedef random::DotNetRandom DotNetRandom;
}} // namespace pppp::utils

#endif
