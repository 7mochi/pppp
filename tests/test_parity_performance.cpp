#include "common.h"
#include "maps.h"
#include "parity.h"
#include <doctest.h>
#include <pppp/difficulty.h>
#include <pppp/performance.h>
#include <string>

namespace common = ::pppp::common;

namespace {
    const pppp::DifficultyAttributes difficulty(const char* map, pppp::Ruleset::Value ruleset,
                                                const char* mods) {
        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, map);

        return pppp_test::calculate(pppp::Difficulty().mods(pppp_test::parse_mods(mods)).ruleset(ruleset),
                                    beatmap);
    }

    pppp::PerformanceAttributes performance(const char* map, pppp::Ruleset::Value ruleset, const char* mods,
                                            const common::ScoreInfo& score) {
        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, map);

        const pppp::DifficultyAttributes attributes = difficulty(map, ruleset, mods);
        return pppp_test::calculate(
            pppp::Performance(beatmap, attributes).mods(pppp_test::parse_mods(mods)).score(score));
    }

    bool matches(const nonstd::optional<double>& value, double expected) {
        return value.has_value() && value.value() == pppp_test::pp_approx(expected);
    }

    bool absent(double expected) { return expected != expected; }
} // namespace

TEST_CASE("osu! performance matches upstream") {
    const parity::Table rows("performance", "osu");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + map;
        common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(mods);
        score.statistics[common::HIT_RESULT_GREAT] = given.integer("great");
        score.statistics[common::HIT_RESULT_OK] = given.integer("ok");
        score.statistics[common::HIT_RESULT_MEH] = given.integer("meh");
        score.statistics[common::HIT_RESULT_MISS] = given.integer("miss");
        const pppp::DifficultyAttributes attributes = difficulty(path.c_str(), pppp::Ruleset::OSU, mods);
        const pppp::osu::difficulty::OsuDifficultyAttributes* attrs = attributes.osu();
        REQUIRE_UNARY(attrs != 0);
        // A perfect play completes every slider tail, which is the same value osu-native's own tests
        // pass for a score that drops no slider ends.
        score.statistics[common::HIT_RESULT_SLIDER_TAIL_HIT] = attrs->slider_count;
        score.max_combo = given.integer("combo");
        score.accuracy = given.number("accuracy") < 0.0
                             ? pppp_test::classic_accuracy(given.integer("great"), given.integer("ok"),
                                                           given.integer("meh"), given.integer("miss"))
                             : given.number("accuracy");
        if (given.integer("legacy_total_score") != 0) {
            score.legacy_total_score = given.integer("legacy_total_score");
        }

        const pppp::PerformanceAttributes result = performance(path.c_str(), pppp::Ruleset::OSU, mods, score);
        const pppp::osu::difficulty::OsuPerformanceAttributes* pp = result.osu();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(expected.number("total")));
        CHECK(pp->aim == pppp_test::pp_approx(expected.number("aim")));
        CHECK(pp->speed == pppp_test::pp_approx(expected.number("speed")));
        CHECK(pp->accuracy == pppp_test::pp_approx(expected.number("accuracy")));
        CHECK(pp->flashlight == pppp_test::pp_approx(expected.number("flashlight")));
        CHECK(pp->reading == pppp_test::pp_approx(expected.number("reading")));
        CHECK(pp->effective_miss_count == pppp_test::pp_approx(expected.number("effective_miss_count")));
        CHECK(pp->combo_based_estimated_miss_count ==
              pppp_test::pp_approx(expected.number("combo_based_estimated_miss_count")));
        if (absent(expected.number("score_based_estimated_miss_count"))) {
            CHECK(!pp->score_based_estimated_miss_count.has_value());
        } else {
            CHECK(matches(pp->score_based_estimated_miss_count,
                          expected.number("score_based_estimated_miss_count")));
        }
        CHECK(pp->aim_estimated_slider_breaks ==
              pppp_test::pp_approx(expected.number("aim_estimated_slider_breaks")));
        CHECK(pp->speed_estimated_slider_breaks ==
              pppp_test::pp_approx(expected.number("speed_estimated_slider_breaks")));
        if (absent(expected.number("speed_deviation"))) {
            CHECK(!pp->speed_deviation.has_value());
        } else {
            CHECK(matches(pp->speed_deviation, expected.number("speed_deviation")));
        }
    }
}

TEST_CASE("taiko performance matches upstream") {
    const parity::Table rows("performance", "taiko");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + map;
        const int great = given.integer("great");
        const int ok = given.integer("ok");
        const int miss = given.integer("miss");
        common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(mods);
        score.statistics[common::HIT_RESULT_GREAT] = great;
        score.statistics[common::HIT_RESULT_OK] = ok;
        score.statistics[common::HIT_RESULT_MISS] = miss;
        // The map's own maximum combo stands in when the table names none, and this mode's accuracy is
        // always the one its judgements give, never a number read from the table.
        const pppp::DifficultyAttributes attributes = difficulty(path.c_str(), pppp::Ruleset::TAIKO, mods);
        const pppp::taiko::difficulty::TaikoDifficultyAttributes* attrs = attributes.taiko();
        REQUIRE_UNARY(attrs != 0);
        score.max_combo = given.integer("combo") != 0 ? given.integer("combo") : attrs->max_combo;
        score.accuracy = (great * 300.0 + ok * 150.0) / ((great + ok + miss) * 300.0);

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::TAIKO, mods, score);
        const pppp::taiko::difficulty::TaikoPerformanceAttributes* pp = result.taiko();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(expected.number("total")));
        CHECK(pp->difficulty == pppp_test::pp_approx(expected.number("difficulty")));
        CHECK(pp->accuracy == pppp_test::pp_approx(expected.number("accuracy")));
        if (absent(expected.number("estimated_unstable_rate"))) {
            CHECK(!pp->estimated_unstable_rate.has_value());
        } else {
            CHECK(matches(pp->estimated_unstable_rate, expected.number("estimated_unstable_rate")));
        }
    }
}

TEST_CASE("osu!catch performance matches upstream") {
    const parity::Table rows("performance", "catch");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + map;
        const int fruits = given.integer("fruits");
        const int droplets = given.integer("droplets");
        const int tiny_droplets = given.integer("tiny_droplets");
        const int tiny_misses = given.integer("tiny_misses");
        const int miss = given.integer("miss");
        common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(mods);
        score.statistics[common::HIT_RESULT_GREAT] = fruits;
        score.statistics[common::HIT_RESULT_LARGE_TICK_HIT] = droplets;
        score.statistics[common::HIT_RESULT_SMALL_TICK_HIT] = tiny_droplets;
        score.statistics[common::HIT_RESULT_SMALL_TICK_MISS] = tiny_misses;
        score.statistics[common::HIT_RESULT_MISS] = miss;
        const pppp::DifficultyAttributes attributes = difficulty(path.c_str(), pppp::Ruleset::CATCH, mods);
        const pppp::fruits::difficulty::CatchDifficultyAttributes* attrs = attributes.fruits();
        REQUIRE_UNARY(attrs != 0);
        score.max_combo = given.integer("combo") != 0 ? given.integer("combo") : attrs->max_combo;
        score.accuracy = static_cast<double>(fruits + droplets + tiny_droplets) /
                         (fruits + droplets + tiny_droplets + tiny_misses + miss);

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::CATCH, mods, score);
        const pppp::fruits::difficulty::CatchPerformanceAttributes* pp = result.fruits();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(expected.number("total")));
    }
}

TEST_CASE("osu!mania performance matches upstream") {
    const parity::Table rows("performance", "mania");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + map;
        const int perfect = given.integer("perfect");
        const int great = given.integer("great");
        const int good = given.integer("good");
        const int ok = given.integer("ok");
        const int meh = given.integer("meh");
        const int miss = given.integer("miss");
        common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(mods);
        score.statistics[common::HIT_RESULT_PERFECT] = perfect;
        score.statistics[common::HIT_RESULT_GREAT] = great;
        score.statistics[common::HIT_RESULT_GOOD] = good;
        score.statistics[common::HIT_RESULT_OK] = ok;
        score.statistics[common::HIT_RESULT_MEH] = meh;
        score.statistics[common::HIT_RESULT_MISS] = miss;
        const pppp::DifficultyAttributes attributes = difficulty(path.c_str(), pppp::Ruleset::MANIA, mods);
        const pppp::mania::difficulty::ManiaDifficultyAttributes* attrs = attributes.mania();
        REQUIRE_UNARY(attrs != 0);
        score.max_combo = given.integer("combo") != 0 ? given.integer("combo") : attrs->max_combo;
        score.accuracy = (perfect * 305.0 + great * 300.0 + good * 200.0 + ok * 100.0 + meh * 50.0) /
                         ((perfect + great + good + ok + meh + miss) * 305.0);

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::MANIA, mods, score);
        const pppp::mania::difficulty::ManiaPerformanceAttributes* pp = result.mania();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(expected.number("total")));
        CHECK(pp->difficulty == pppp_test::pp_approx(expected.number("difficulty")));
    }
}
