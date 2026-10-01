// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "common.h"
#include "maps.h"
#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_performance_calculator.h"
#include "pppp/mania/difficulty/mania_performance_calculator.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/osu_performance_attributes.h"
#include "pppp/osu/difficulty/osu_performance_calculator.h"
#include "pppp/performance.h"
#include "pppp/taiko/difficulty/taiko_performance_calculator.h"
#include <cstddef>
#include <doctest.h>

namespace {
    struct OsuRow {
        const char* mods;
        int great;
        int ok;
        int meh;
        int miss;
        int combo;
        pppp_int64 legacy_total_score;
        double accuracy;
        double total;
        double aim;
        double speed;
        double accuracy_pp;
        double flashlight;
        double reading;
        double effective_miss_count;
        double combo_based_estimated_miss_count;
        double score_based_estimated_miss_count;
        double aim_estimated_slider_breaks;
        double speed_estimated_slider_breaks;
        double speed_deviation;
    };

    struct TaikoRow {
        const char* mods;
        int great;
        int ok;
        int miss;
        int combo;
        double total;
        double difficulty;
        double accuracy;
        double estimated_unstable_rate;
    };

    struct CatchRow {
        const char* mods;
        int fruits;
        int droplets;
        int tiny_droplets;
        int tiny_misses;
        int miss;
        int combo;
        double total;
    };

    struct ManiaRow {
        const char* mods;
        int perfect;
        int great;
        int good;
        int ok;
        int meh;
        int miss;
        int combo;
        double total;
    };

    void check(const pppp::beatmaps::Beatmap& beatmap, const OsuRow& row) {
        const pppp_test::Mods mods(row.mods);
        pppp::common::ScoreInfo score;

        INFO(row.mods);
        CAPTURE(row.great);
        CAPTURE(row.miss);

        const pppp::DifficultyAttributes difficulty = pppp::Difficulty()
                                                          .mods(mods.list, mods.count)
                                                          .ruleset(pppp::Ruleset::RULESET_OSU)
                                                          .calculate(beatmap);
        const pppp::osu::difficulty::OsuDifficultyAttributes& attrs = difficulty.osu;

        score.mods = mods.list;
        score.mod_count = mods.count;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.great;
        score.statistics[pppp::common::HIT_RESULT_OK] = row.ok;
        score.statistics[pppp::common::HIT_RESULT_MEH] = row.meh;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        score.statistics[pppp::common::HIT_RESULT_SLIDER_TAIL_HIT] = attrs.slider_count;
        score.max_combo = row.combo;
        score.accuracy = row.accuracy < 0.0
                             ? pppp_test::classic_accuracy(row.great, row.ok, row.meh, row.miss)
                             : row.accuracy;
        if (row.legacy_total_score != 0) {
            score.legacy_total_score = row.legacy_total_score;
        }

        const pppp::PerformanceAttributes performance =
            pppp::Performance(beatmap, difficulty).mods(mods.list, mods.count).state(score).calculate();
        const pppp::osu::difficulty::OsuPerformanceAttributes& pp = performance.osu;
        CHECK(pp.total == pppp_test::pp_approx(row.total));
        CHECK(pp.aim == pppp_test::pp_approx(row.aim));
        CHECK(pp.speed == pppp_test::pp_approx(row.speed));
        CHECK(pp.accuracy == pppp_test::pp_approx(row.accuracy_pp));
        CHECK(pp.flashlight == pppp_test::pp_approx(row.flashlight));
        CHECK(pp.reading == pppp_test::pp_approx(row.reading));
        CHECK(pp.effective_miss_count == pppp_test::pp_approx(row.effective_miss_count));
        CHECK(pp.combo_based_estimated_miss_count ==
              pppp_test::pp_approx(row.combo_based_estimated_miss_count));
        REQUIRE(pp.score_based_estimated_miss_count.has_value() ==
                (row.score_based_estimated_miss_count >= 0.0));
        if (pp.score_based_estimated_miss_count.has_value()) {
            CHECK(pp.score_based_estimated_miss_count.value() ==
                  pppp_test::pp_approx(row.score_based_estimated_miss_count));
        }
        CHECK(pp.aim_estimated_slider_breaks == pppp_test::pp_approx(row.aim_estimated_slider_breaks));
        CHECK(pp.speed_estimated_slider_breaks == pppp_test::pp_approx(row.speed_estimated_slider_breaks));
        REQUIRE(pp.speed_deviation.has_value());
        CHECK(pp.speed_deviation.value() == pppp_test::pp_approx(row.speed_deviation));
    }

    void check(const pppp::beatmaps::Beatmap& beatmap, const TaikoRow& row) {
        const pppp_test::Mods mods(row.mods);
        pppp::common::ScoreInfo score;
        const int total = row.great + row.ok + row.miss;

        INFO(row.mods);
        CAPTURE(row.great);

        const pppp::DifficultyAttributes difficulty = pppp::Difficulty()
                                                          .mods(mods.list, mods.count)
                                                          .ruleset(pppp::Ruleset::RULESET_TAIKO)
                                                          .calculate(beatmap);
        const pppp::taiko::difficulty::TaikoDifficultyAttributes& attrs = difficulty.taiko;

        score.mods = mods.list;
        score.mod_count = mods.count;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.great;
        score.statistics[pppp::common::HIT_RESULT_OK] = row.ok;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        score.max_combo = row.combo != 0 ? row.combo : attrs.max_combo;
        score.accuracy = (row.great * 300.0 + row.ok * 150.0) / (total * 300.0);

        const pppp::PerformanceAttributes performance =
            pppp::Performance(beatmap, difficulty).mods(mods.list, mods.count).state(score).calculate();
        const pppp::taiko::difficulty::TaikoPerformanceAttributes& pp = performance.taiko;
        CHECK(pp.total == pppp_test::pp_approx(row.total));
        CHECK(pp.difficulty == pppp_test::pp_approx(row.difficulty));
        CHECK(pp.accuracy == pppp_test::pp_approx(row.accuracy));
        REQUIRE(pp.estimated_unstable_rate.has_value());
        CHECK(pp.estimated_unstable_rate.value() == pppp_test::pp_approx(row.estimated_unstable_rate));
    }

    void check(const pppp::beatmaps::Beatmap& beatmap, const CatchRow& row) {
        const pppp_test::Mods mods(row.mods);
        pppp::common::ScoreInfo score;
        const int total = row.fruits + row.droplets + row.tiny_droplets + row.tiny_misses + row.miss;

        INFO(row.mods);
        CAPTURE(row.fruits);

        const pppp::DifficultyAttributes difficulty = pppp::Difficulty()
                                                          .mods(mods.list, mods.count)
                                                          .ruleset(pppp::Ruleset::RULESET_CATCH)
                                                          .calculate(beatmap);
        const pppp::fruits::difficulty::CatchDifficultyAttributes& attrs = difficulty.fruits;

        score.mods = mods.list;
        score.mod_count = mods.count;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.fruits;
        score.statistics[pppp::common::HIT_RESULT_LARGE_TICK_HIT] = row.droplets;
        score.statistics[pppp::common::HIT_RESULT_SMALL_TICK_HIT] = row.tiny_droplets;
        score.statistics[pppp::common::HIT_RESULT_SMALL_TICK_MISS] = row.tiny_misses;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        score.max_combo = row.combo != 0 ? row.combo : attrs.max_combo;
        score.accuracy = static_cast<double>(row.fruits + row.droplets + row.tiny_droplets) / total;

        const pppp::PerformanceAttributes performance =
            pppp::Performance(beatmap, difficulty).mods(mods.list, mods.count).state(score).calculate();
        const pppp::fruits::difficulty::CatchPerformanceAttributes& pp = performance.fruits;
        CHECK(pp.total == pppp_test::pp_approx(row.total));
    }

    void check(const pppp::beatmaps::Beatmap& beatmap, const ManiaRow& row) {
        const pppp_test::Mods mods(row.mods);
        pppp::common::ScoreInfo score;
        const int total = row.perfect + row.great + row.good + row.ok + row.meh + row.miss;

        INFO(row.mods);
        CAPTURE(row.perfect);

        const pppp::DifficultyAttributes difficulty = pppp::Difficulty()
                                                          .mods(mods.list, mods.count)
                                                          .ruleset(pppp::Ruleset::RULESET_MANIA)
                                                          .calculate(beatmap);
        const pppp::mania::difficulty::ManiaDifficultyAttributes& attrs = difficulty.mania;

        score.mods = mods.list;
        score.mod_count = mods.count;
        score.statistics[pppp::common::HIT_RESULT_PERFECT] = row.perfect;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.great;
        score.statistics[pppp::common::HIT_RESULT_GOOD] = row.good;
        score.statistics[pppp::common::HIT_RESULT_OK] = row.ok;
        score.statistics[pppp::common::HIT_RESULT_MEH] = row.meh;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        score.max_combo = row.combo != 0 ? row.combo : attrs.max_combo;
        score.accuracy =
            (row.perfect * 305.0 + row.great * 300.0 + row.good * 200.0 + row.ok * 100.0 + row.meh * 50.0) /
            (total * 305.0);

        const pppp::PerformanceAttributes performance =
            pppp::Performance(beatmap, difficulty).mods(mods.list, mods.count).state(score).calculate();
        const pppp::mania::difficulty::ManiaPerformanceAttributes& pp = performance.mania;
        CHECK(pp.total == pppp_test::pp_approx(row.total));
    }
} // namespace

TEST_CASE("basic_osu") {
    static const OsuRow rows[] = {
        {"NM",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         316.5901855625614,
         148.75278891878943,
         61.34653468094172,
         98.99847982709288,
         0,
         2.2291238201795176,
         0,
         0,
         -1.0,
         0,
         0,
         11.70045116819282},
        {"HD",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         349.9881115302272,
         148.75278891878943,
         61.34653468094172,
         98.99847982709288,
         0,
         41.59660781351789,
         0,
         0,
         -1.0,
         0,
         0,
         11.70045116819282},
        {"EZ,HD",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         330.5430804957736,
         88.9845069859366,
         40.67344998245687,
         16.05545397996135,
         0,
         179.54117243675987,
         0,
         0,
         -1.0,
         0,
         0,
         23.04067406810845},
        {"HR",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         468.21934604774174,
         231.45791599856506,
         61.86844909389746,
         161.55575439788055,
         0,
         3.0588804345706087,
         0,
         0,
         -1.0,
         0,
         0,
         8.609766678538842},
        {"DT",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         861.1999380363726,
         436.40604835642193,
         198.35289926936036,
         183.66566616694254,
         0,
         33.10777055891541,
         0,
         0,
         -1.0,
         0,
         0,
         7.66444640194172},
        {"FL",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         444.5824653333625,
         148.75278891878943,
         61.34653468094172,
         98.99847982709288,
         137.54700871774983,
         2.2291238201795176,
         0,
         0,
         -1.0,
         0,
         0,
         11.70045116819282},
        {"HD,FL",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         512.2069389717595,
         148.75278891878943,
         61.34653468094172,
         98.99847982709288,
         172.5399803247157,
         41.59660781351789,
         0,
         0,
         -1.0,
         0,
         0,
         11.70045116819282},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("basic_taiko") {
    static const TaikoRow rows[] = {
        {"NM", 289, 0, 0, 0, 130.26636361095524, 33.48488833057447, 96.78147528038076, 146.3238357972284},
        {"HD", 289, 0, 0, 0, 138.19467202359527, 34.15458609718596, 104.04008592640932, 146.3238357972284},
        {"HR", 289, 0, 0, 0, 166.70246243922367, 36.50275483295865, 130.199707606265, 120.87621218031913},
        {"DT", 289, 0, 0, 0, 265.5965861527287, 92.42487686635559, 173.17170928637316, 97.54922386481894},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/1028484.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("convert_taiko") {
    static const TaikoRow rows[] = {
        {"NM", 908, 0, 0, 0, 372.4458760719575, 153.2164827606566, 219.22939331130095, 81.74165086164196},
        {"HD", 908, 0, 0, 0, 373.2119584857609, 153.9825651744599, 219.22939331130095, 81.74165086164196},
        {"HR", 908, 0, 0, 0, 452.2496503558973, 194.56688580907107, 257.68276454682626, 70.84276408008971},
        {"DT", 908, 0, 0, 0, 769.13039895928, 385.1562322213426, 383.9741667379375, 54.49443390776131},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("basic_catch") {
    static const CatchRow rows[] = {
        {"NM", 728, 2, 263, 0, 0, 0, 112.72215339177879},
        {"HD", 728, 2, 263, 0, 0, 0, 135.26658407013454},
        {"HD,HR", 728, 2, 263, 0, 0, 0, 231.1954012763412},
        {"DT", 728, 2, 263, 0, 0, 0, 245.48176596381523},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/fruits/2118524.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("convert_catch") {
    static const CatchRow rows[] = {
        {"NM", 908, 0, 159, 0, 0, 0, 232.34402311853054},
        {"HD", 908, 0, 159, 0, 0, 0, 256.159282164472},
        {"HD,HR", 908, 0, 159, 0, 0, 0, 327.3523805137957},
        {"DT", 908, 0, 159, 0, 0, 0, 502.8408227990554},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("basic_mania") {
    static const ManiaRow rows[] = {
        {"NM", 715, 0, 0, 0, 0, 0, 0, 108.92297471705167},
        {"EZ", 715, 0, 0, 0, 0, 0, 0, 54.46148735852584},
        {"DT", 715, 0, 0, 0, 0, 0, 0, 224.52717042937203},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/mania/1638954.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("convert_mania") {
    static const ManiaRow rows[] = {
        {"NM", 1312, 0, 0, 0, 0, 0, 0, 101.39189449271568},
        {"EZ", 1312, 0, 0, 0, 0, 0, 0, 50.69594724635784},
        {"DT", 1312, 0, 0, 0, 0, 0, 0, 198.46891237015896},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("osu!standard scores with misses on 2785319.osu") {
    static const OsuRow rows[] = {
        {"NM",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         316.5901855625614,
         148.75278891878943,
         61.34653468094172,
         98.99847982709288,
         0,
         2.2291238201795176,
         0,
         0,
         -1.0,
         0,
         0,
         11.70045116819282},
        {"NM",
         598,
         2,
         0,
         1,
         561,
         5409000,
         -1.0,
         281.23685107410796,
         131.0158437846765,
         53.44333276016617,
         90.16030098202059,
         0,
         1.9143392287156948,
         1,
         1,
         -1.0,
         0,
         0,
         12.84569412425561},
        {"NM",
         600,
         1,
         0,
         0,
         909,
         0,
         0.9995,
         313.5931111664516,
         148.67841252433004,
         61.044586857635174,
         96.39197991665763,
         0,
         2.2257818060134733,
         0,
         0,
         -1.0,
         0,
         0,
         12.330239528599414},
        {"HD,HR,DT,FL",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         1903.9380064848701,
         673.0817238043575,
         198.40984801835927,
         254.57978510253744,
         562.5546054607017,
         256.3336670922647,
         0,
         0,
         -1.0,
         0,
         0,
         5.639875916042211},
        {"HD,HR,DT,FL",
         598,
         2,
         0,
         1,
         561,
         5409000,
         -1.0,
         1556.6503306219697,
         593.450454755154,
         174.72923708817956,
         231.85194448310463,
         367.5800885557813,
         224.60891250014552,
         1,
         1,
         -1.0,
         0,
         0,
         6.172601589845095},
        {"CL",
         601,
         0,
         0,
         0,
         909,
         0,
         -1.0,
         298.5325815982227,
         148.75278891878943,
         61.34653468094172,
         80.97009771045249,
         0,
         2.2291238201795176,
         0,
         0,
         -1.0,
         0,
         0,
         11.70045116819282},
        {"CL",
         598,
         2,
         0,
         1,
         561,
         5409000,
         -1.0,
         240.17950868231355,
         118.65978693331824,
         48.25834074745435,
         67.42209392326264,
         0,
         1.596150907083617,
         3,
         1.5609625668449199,
         3,
         1.0325757091930874,
         0.902726298511645,
         12.84569412425561},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("osu!taiko scores on v9.osu") {
    static const TaikoRow rows[] = {
        {"NM", 716, 0, 0, 0, 198.288506328797, 77.05812472448943, 121.23038160430758, 129.00468373694073},
        {"NM", 705, 10, 1, 400, 160.18804104568687, 75.06349237847733, 85.12454866720955, 159.26353298023395},
        {"HD,FL", 716, 0, 0, 0, 219.32479424324907, 89.00213401861843, 130.32266022463062,
         129.00468373694073},
        {"HD,FL", 705, 10, 1, 400, 178.20722347721272, 86.69833365996243, 91.50888981725028,
         159.26353298023395},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/v9.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("osu!catch scores on v12.osu") {
    static const CatchRow rows[] = {
        {"NM", 292, 0, 0, 0, 0, 0, 33.96188592231199},
        {"NM", 281, 10, 20, 2, 1, 300, 23.454358461534294},
        {"HD,HR", 292, 0, 0, 0, 0, 0, 105.49742830397298},
        {"HD,HR", 281, 10, 20, 2, 1, 300, 72.85739389948904},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/fruits/v12.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("osu!mania scores on keys_4_holds.osu") {
    static const ManiaRow rows[] = {
        {"NM", 520, 0, 0, 0, 0, 0, 0, 86.27265617167986},
        {"NM", 499, 10, 5, 3, 2, 1, 2000, 80.25845658278872},
        {"NF,DT", 520, 0, 0, 0, 0, 0, 0, 126.28339731995449},
        {"NF,DT", 499, 10, 5, 3, 2, 1, 2000, 117.47998741063074},
    };
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/mania/keys_4_holds.osu");
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(beatmap, rows[i]);
    }
}

TEST_CASE("every ruleset rejects a null mod list with a count") {
    const pppp::beatmaps::Beatmap empty;
    pppp::common::ScoreInfo score;

    score.mods = 0;
    score.mod_count = 2;

    SUBCASE("osu!standard") {
        const pppp::osu::difficulty::OsuDifficultyAttributes attrs;
        pppp::osu::difficulty::OsuPerformanceAttributes pp;

        CHECK(pppp::osu::difficulty::calculate_performance(pp, score, attrs, empty) ==
              pppp::Result::INVALID_ARGUMENT);
    }

    SUBCASE("osu!taiko") {
        const pppp::taiko::difficulty::TaikoDifficultyAttributes attrs;
        pppp::taiko::difficulty::TaikoPerformanceAttributes pp;

        CHECK(pppp::taiko::difficulty::calculate_performance(pp, score, attrs, empty) ==
              pppp::Result::INVALID_ARGUMENT);
    }

    SUBCASE("osu!catch") {
        pppp::fruits::difficulty::CatchDifficultyAttributes attrs;
        pppp::fruits::difficulty::CatchPerformanceAttributes pp;

        CHECK(pppp::fruits::difficulty::calculate_performance(pp, score, attrs, empty) ==
              pppp::Result::INVALID_ARGUMENT);
    }

    SUBCASE("osu!mania") {
        const pppp::mania::difficulty::ManiaDifficultyAttributes attrs;
        pppp::mania::difficulty::ManiaPerformanceAttributes pp;

        CHECK(pppp::mania::difficulty::calculate_performance(pp, score, attrs, empty) ==
              pppp::Result::INVALID_ARGUMENT);
    }
}
