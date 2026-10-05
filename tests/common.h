#ifndef PPPP_TESTS_SUPPORT_H
#define PPPP_TESTS_SUPPORT_H

#include "pppp/mods/mods.h"
#include "pppp/performance.h"
#include <doctest.h>
#include <vector>

namespace pppp_test {
    inline pppp::mods::Mods parse_mods(const char* spec) {
        pppp::mods::Mods out;

        INFO(spec);
        REQUIRE(out.parse(spec).ok());
        return out;
    }

    inline pppp::DifficultyAttributes calculate(const pppp::Difficulty& difficulty,
                                                const pppp::beatmaps::Beatmap& beatmap) {
        pppp::DifficultyAttributes out;
        REQUIRE(difficulty.calculate(beatmap, out).ok());
        return out;
    }

    inline std::vector<pppp::TimedDifficultyAttributes>
    calculate_timed(const pppp::Difficulty& difficulty, const pppp::beatmaps::Beatmap& beatmap) {
        std::vector<pppp::TimedDifficultyAttributes> out;
        REQUIRE(difficulty.calculate_timed(beatmap, out).ok());
        return out;
    }

    inline pppp::Strains strains(const pppp::Difficulty& difficulty, const pppp::beatmaps::Beatmap& beatmap) {
        pppp::Strains out;
        REQUIRE(difficulty.strains(beatmap, out).ok());
        return out;
    }

    inline pppp::PerformanceAttributes calculate(const pppp::Performance& performance) {
        pppp::PerformanceAttributes out;
        REQUIRE(performance.calculate(out).ok());
        return out;
    }

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
