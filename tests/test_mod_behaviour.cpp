// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "common.h"
#include "maps.h"
#include "pppp/difficulty.h"
#include "pppp/performance.h"
#include <cstddef>
#include <doctest.h>

TEST_CASE("the score's mods reach the difficulty when the calculator is given none") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/1028484.osu");

    const pppp::Mods mods = pppp_test::parse_mods("HR");

    pppp::ScoreInfo score;
    score.mods = mods;
    score.statistics[pppp::HIT_RESULT_GREAT] = 289;
    score.max_combo = pppp_test::calculate(pppp::Difficulty().mods(mods), beatmap).max_combo();
    score.accuracy = 1.0;

    const pppp::PerformanceAttributes from_score =
        pppp_test::calculate(pppp::Performance(beatmap).score(score));
    const pppp::DifficultyAttributes attributes =
        pppp_test::calculate(pppp::Difficulty().mods(mods).ruleset(pppp::Ruleset::TAIKO), beatmap);
    const pppp::PerformanceAttributes given =
        pppp_test::calculate(pppp::Performance(beatmap, attributes).mods(mods).score(score));

    CHECK(from_score.total() == given.total());
}

TEST_CASE("adding classic to legacy mods matches the classic specification") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");

    pppp::mods::Mods from_bits = pppp::mods::Mods::from_legacy(0u);
    REQUIRE_FALSE(from_bits.contains(pppp::mods::MOD_CL));

    from_bits.add_classic();
    REQUIRE(from_bits.contains(pppp::mods::MOD_CL));

    const size_t size = from_bits.size();
    from_bits.add_classic();
    CHECK(from_bits.size() == size);

    const pppp::DifficultyAttributes attributes =
        pppp_test::calculate(pppp::Difficulty().mods(from_bits).ruleset(pppp::Ruleset::OSU), beatmap);

    pppp::common::ScoreInfo score;
    score.statistics[pppp::common::HIT_RESULT_GREAT] = 601;
    score.statistics[pppp::common::HIT_RESULT_SLIDER_TAIL_HIT] = attributes.osu()->slider_count;
    score.max_combo = 909;
    score.accuracy = pppp_test::classic_accuracy(601, 0, 0, 0);

    const pppp::PerformanceAttributes actual =
        pppp_test::calculate(pppp::Performance(beatmap, attributes).mods(from_bits).score(score));
    CHECK(actual.osu()->total == pppp_test::pp_approx(290.84046231334827));

    const pppp::PerformanceAttributes expected = pppp_test::calculate(
        pppp::Performance(beatmap, attributes).mods(pppp_test::parse_mods("CL")).score(score));
    CHECK(actual.osu()->total == expected.osu()->total);
}
