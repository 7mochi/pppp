#ifndef PPPP_TESTS_SUPPORT_H
#define PPPP_TESTS_SUPPORT_H

#include "pppp/mods/mod.h"
#include <cstddef>
#include <doctest.h>

namespace pppp_test {
    struct Mods {
        pppp::mods::Mod list[8];
        size_t count;

        explicit Mods(const char* spec)
            : count(0) {
            const int parsed = pppp::mods::mod_from_acronyms(list, 8, spec);

            INFO(spec);
            REQUIRE(parsed >= 0);
            count = static_cast<size_t>(parsed);
        }
    };

    inline doctest::Approx difficulty_approx(double value) { return doctest::Approx(value).epsilon(1e-6); }

    inline doctest::Approx pp_approx(double value) { return doctest::Approx(value).epsilon(1e-9); }

    inline double distance(double a, double b) { return a > b ? a - b : b - a; }

    inline double classic_accuracy(int great, int ok, int meh, int miss) {
        const int total = great + ok + meh + miss;

        if (total <= 0) {
            return 1.0;
        }
        return (great * 300.0 + ok * 100.0 + meh * 50.0) / (total * 300.0);
    }
} // namespace pppp_test

#endif
