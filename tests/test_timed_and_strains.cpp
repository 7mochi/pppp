// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "common.h"
#include "maps.h"
#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/osu_difficulty_calculator.h"
#include "pppp/performance.h"
#include "pppp/taiko/difficulty/taiko_difficulty_calculator.h"
#include <cstddef>
#include <doctest.h>
#include <vector>

TEST_CASE("an empty beatmap has default attributes") {
    const pppp::beatmaps::Beatmap empty;
    pppp::osu::difficulty::OsuDifficultyAttributes attrs;

    REQUIRE(pppp::osu::difficulty::calculate_difficulty(attrs, empty, 0, 0).ok());
    CHECK(attrs.star_rating == 0.0);
    CHECK(attrs.slider_factor == 0.0);
}

TEST_CASE("every ruleset rejects a null mod list with a count") {
    const pppp::beatmaps::Beatmap empty;
    pppp::osu::difficulty::OsuDifficultyAttributes osu;
    pppp::taiko::difficulty::TaikoDifficultyAttributes taiko;
    pppp::fruits::difficulty::CatchDifficultyAttributes fruits;
    pppp::mania::difficulty::ManiaDifficultyAttributes mania;

    CHECK(pppp::osu::difficulty::calculate_difficulty(osu, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(pppp::taiko::difficulty::calculate_difficulty(taiko, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(pppp::fruits::difficulty::calculate_difficulty(fruits, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(pppp::mania::difficulty::calculate_difficulty(mania, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
}

TEST_CASE("the facade reports a calculation that could not run") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    beatmap.slider_path.recompute = 0;

    const pppp::Mods mirror = pppp_test::parse_mods("MR");
    const pppp::Difficulty difficulty = pppp::Difficulty().mods(mirror);
    pppp::DifficultyAttributes attributes;
    std::vector<pppp::TimedDifficultyAttributes> timed;
    pppp::Strains strains;
    CHECK(difficulty.calculate(beatmap, attributes).code() == pppp::StatusCode::NO_SLIDER_PATH_BACKEND);
    CHECK(difficulty.calculate_timed(beatmap, timed).code() == pppp::StatusCode::NO_SLIDER_PATH_BACKEND);
    CHECK(difficulty.strains(beatmap, strains).code() == pppp::StatusCode::NO_SLIDER_PATH_BACKEND);

    pppp::ScoreInfo score;
    score.mods = mirror;
    pppp::PerformanceAttributes performance;
    CHECK(pppp::Performance(beatmap).score(score).calculate(performance).code() ==
          pppp::StatusCode::NO_SLIDER_PATH_BACKEND);

    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const pppp::Difficulty unknown = pppp::Difficulty().ruleset(static_cast<pppp::Ruleset::Value>(4));
    CHECK(unknown.calculate(beatmap, attributes).code() == pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(unknown.calculate_timed(beatmap, timed).code() == pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(unknown.strains(beatmap, strains).code() == pppp::StatusCode::INVALID_ARGUMENT);
}

namespace {
    struct TimedRow {
        const char* map;
        const char* mods;
        int ruleset;
        size_t count;
        double middle_time;
        double middle_stars;
        int middle_combo;
        double last_time;
        double last_stars;
        int last_combo;
    };
} // namespace

TEST_CASE("the timed attributes match upstream's CalculateTimed") {
    static const TimedRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "NM", 0, 124, 18000, 5.767545351317129, 63, 103000,
         6.5880619069592505, 239},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "HD,DT", 0, 601, 61309.235290751734, 8.243851306944014, 433,
         115486.23529075173, 8.97136919085591, 909},
        {PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", "DT", 1, 238, 21624, 4.403686852094672, 118, 53000,
         4.462215604030145, 200},
        {PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", "DT", 2, 93, 14500, 4.27412411307246, 47, 45250,
         5.073647828613076, 127},
        {PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", "NM", 3, 137, 16250, 1.8228476946125378, 69, 30500,
         2.3493769750220914, 242},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "4K", 3, 838, 61662, 2.5718820820666117, 530, 115486,
         2.653795415351293, 1072},
    };

    for (size_t r = 0; r < sizeof(rows) / sizeof(rows[0]); r++) {
        const TimedRow& row = rows[r];
        INFO(row.map);
        INFO(row.mods);
        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, row.map);

        const std::vector<pppp::TimedDifficultyAttributes> timed =
            pppp_test::calculate_timed(pppp::Difficulty()
                                           .mods(pppp_test::parse_mods(row.mods))
                                           .ruleset(static_cast<pppp::Ruleset::Value>(row.ruleset)),
                                       beatmap);

        REQUIRE(timed.size() == row.count);
        const pppp::TimedDifficultyAttributes& middle = timed[row.count / 2];
        const pppp::TimedDifficultyAttributes& last = timed[row.count - 1];
        CHECK(middle.time == row.middle_time);
        CHECK(middle.attributes.star_rating() == pppp_test::difficulty_approx(row.middle_stars));
        CHECK(middle.attributes.max_combo() == row.middle_combo);
        CHECK(last.time == row.last_time);
        CHECK(last.attributes.star_rating() == pppp_test::difficulty_approx(row.last_stars));
        CHECK(last.attributes.max_combo() == row.last_combo);
        CHECK(last.attributes.ruleset() == static_cast<pppp::Ruleset::Value>(row.ruleset));
    }
}

TEST_CASE("the first timed taiko entries keep upstream's NaN") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu");

    const std::vector<pppp::TimedDifficultyAttributes> timed =
        pppp_test::calculate_timed(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), beatmap);

    REQUIRE(!timed.empty());
    REQUIRE(timed[0].attributes.taiko());
    CHECK(timed[0].time == 0.0);
    CHECK(timed[0].attributes.star_rating() == 0.0);
    CHECK(timed[0].attributes.max_combo() == 1);
    const double mechanical = timed[0].attributes.taiko()->mechanical_difficulty;
    CHECK(mechanical != mechanical);
}

TEST_CASE("a beatmap without hit objects has no timed attributes") {
    const pppp::beatmaps::Beatmap empty;
    CHECK(pppp_test::calculate_timed(pppp::Difficulty(), empty).empty());
}

TEST_CASE("the osu! strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu");

    const pppp::Strains strains =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("HD,FL")), beatmap);
    REQUIRE(strains.osu());
    const pppp::OsuStrains& osu = *strains.osu();
    CHECK(strains.start_time() == 800.0);
    CHECK(strains.section_length() == 400.0);
    REQUIRE(osu.aim.size() == 254);
    REQUIRE(osu.aim_no_sliders.size() == 254);
    REQUIRE(osu.speed.size() == 254);
    REQUIRE(osu.reading.size() == 254);
    REQUIRE(osu.flashlight.size() == 254);
    CHECK(osu.aim[46] == pppp_test::difficulty_approx(532.2658451798917));
    CHECK(osu.aim_no_sliders[46] == pppp_test::difficulty_approx(532.2658451798917));
    CHECK(osu.speed[19] == pppp_test::difficulty_approx(59.421754881990346));
    CHECK(osu.reading[46] == pppp_test::difficulty_approx(193.38785580639302));
    CHECK(osu.reading[253] == pppp_test::difficulty_approx(12.417219091126125));
    CHECK(osu.flashlight[18] == pppp_test::difficulty_approx(41.02690817661656));

    pppp::Beatmap rated;
    pppp_test::load(rated, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    const pppp::Strains dt = pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), rated);
    REQUIRE(dt.osu());
    CHECK(dt.start_time() == 2400.0);
    CHECK(dt.section_length() == 600.0);
    REQUIRE(dt.osu()->aim.size() == 189);
    CHECK(dt.osu()->aim[161] == pppp_test::difficulty_approx(572.4806692488332));
    CHECK(dt.osu()->aim[188] == pppp_test::difficulty_approx(356.7442881431748));
    CHECK(dt.osu()->speed[130] == pppp_test::difficulty_approx(262.07748853668267));
    CHECK(dt.osu()->flashlight.empty());
}

TEST_CASE("the osu!taiko strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu");

    const pppp::Strains strains =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), beatmap);
    REQUIRE(strains.taiko());
    const pppp::TaikoStrains& taiko = *strains.taiko();
    CHECK(strains.start_time() == 0.0);
    CHECK(strains.section_length() == 600.0);
    REQUIRE(taiko.colour.size() == 89);
    REQUIRE(taiko.reading.size() == 89);
    REQUIRE(taiko.rhythm.size() == 89);
    REQUIRE(taiko.stamina.size() == 89);
    REQUIRE(taiko.single_colour_stamina.size() == 89);
    CHECK(taiko.colour[29] == pppp_test::difficulty_approx(2.6337298882145888));
    CHECK(taiko.reading[32] == pppp_test::difficulty_approx(0.0001525556772378032));
    CHECK(taiko.rhythm[40] == pppp_test::difficulty_approx(0.1353227417658808));
    CHECK(taiko.stamina[32] == pppp_test::difficulty_approx(8.807433788330115));
    CHECK(taiko.single_colour_stamina[6] == pppp_test::difficulty_approx(4.473586847131765));
    CHECK(taiko.stamina[88] == pppp_test::difficulty_approx(1.324675633775491));
}

TEST_CASE("the osu!catch strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu");

    const pppp::Strains strains =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), beatmap);
    REQUIRE(strains.fruits());
    CHECK(strains.start_time() == 0.0);
    CHECK(strains.section_length() == 1125.0);
    REQUIRE(strains.fruits()->movement.size() == 41);
    CHECK(strains.fruits()->movement[11] == pppp_test::difficulty_approx(0.35301054964344125));
    CHECK(strains.fruits()->movement[40] == pppp_test::difficulty_approx(0.028384444121763497));
}

TEST_CASE("the osu!mania strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu");

    const pppp::Strains strains = pppp_test::strains(pppp::Difficulty(), beatmap);
    REQUIRE(strains.mania());
    CHECK(strains.start_time() == 400.0);
    CHECK(strains.section_length() == 400.0);
    REQUIRE(strains.mania()->strain.size() == 76);
    CHECK(strains.mania()->strain[70] == pppp_test::difficulty_approx(16.8859847732691));
    CHECK(strains.mania()->strain[75] == pppp_test::difficulty_approx(16.363963256682354));

    pppp::Beatmap convert;
    pppp_test::load(convert, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    const pppp::Strains keys = pppp_test::strains(
        pppp::Difficulty().mods(pppp_test::parse_mods("4K")).ruleset(pppp::Ruleset::MANIA), convert);
    REQUIRE(keys.mania());
    CHECK(keys.start_time() == 2800.0);
    REQUIRE(keys.mania()->strain.size() == 282);
    CHECK(keys.mania()->strain[195] == pppp_test::difficulty_approx(15.725755965531633));
}

TEST_CASE("a beatmap without hit objects has upstream's strain graph") {
    const pppp::beatmaps::Beatmap empty;

    const pppp::Strains osu = pppp_test::strains(pppp::Difficulty(), empty);
    REQUIRE(osu.osu());
    CHECK(osu.section_length() == 400.0);
    CHECK(osu.osu()->aim.empty());

    const pppp::Strains taiko = pppp_test::strains(pppp::Difficulty().ruleset(pppp::Ruleset::TAIKO), empty);
    REQUIRE(taiko.taiko());
    REQUIRE(taiko.taiko()->colour.size() == 1);
    CHECK(taiko.taiko()->colour[0] == 0.0);
}
