#include "common.h"
#include "maps.h"
#include "parity/performance.h"
#include <doctest.h>
#include <pppp/difficulty.h>
#include <pppp/performance.h>
#include <string>

namespace {
    const pppp::DifficultyAttributes difficulty(const char* map, pppp::Ruleset::Value ruleset,
                                                const char* mods) {
        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, map);

        return pppp_test::calculate(pppp::Difficulty().mods(pppp_test::parse_mods(mods)).ruleset(ruleset),
                                    beatmap);
    }

    pppp::PerformanceAttributes performance(const char* map, pppp::Ruleset::Value ruleset, const char* mods,
                                            const pppp::common::ScoreInfo& score) {
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
    const size_t rows = sizeof(parity::osu_pp_rows) / sizeof(parity::osu_pp_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::OsuPpRow& row = parity::osu_pp_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods << " " << row.score);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + row.map;
        pppp::common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(row.mods);
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.great;
        score.statistics[pppp::common::HIT_RESULT_OK] = row.ok;
        score.statistics[pppp::common::HIT_RESULT_MEH] = row.meh;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        const pppp::DifficultyAttributes attributes = difficulty(path.c_str(), pppp::Ruleset::OSU, row.mods);
        const pppp::osu::difficulty::OsuDifficultyAttributes* attrs = attributes.osu();
        REQUIRE_UNARY(attrs != 0);
        // A perfect play completes every slider tail, which is the same value osu-native's own tests
        // pass for a score that drops no slider ends.
        score.statistics[pppp::common::HIT_RESULT_SLIDER_TAIL_HIT] = attrs->slider_count;
        score.max_combo = row.combo;
        score.accuracy = row.accuracy < 0.0
                             ? pppp_test::classic_accuracy(row.great, row.ok, row.meh, row.miss)
                             : row.accuracy;
        if (row.legacy_total_score != 0) {
            score.legacy_total_score = row.legacy_total_score;
        }

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::OSU, row.mods, score);
        const pppp::osu::difficulty::OsuPerformanceAttributes* pp = result.osu();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(row.pp_total));
        CHECK(pp->aim == pppp_test::pp_approx(row.pp_aim));
        CHECK(pp->speed == pppp_test::pp_approx(row.pp_speed));
        CHECK(pp->accuracy == pppp_test::pp_approx(row.pp_accuracy));
        CHECK(pp->flashlight == pppp_test::pp_approx(row.pp_flashlight));
        CHECK(pp->reading == pppp_test::pp_approx(row.pp_reading));
        CHECK(pp->effective_miss_count == pppp_test::pp_approx(row.pp_effective_miss_count));
        CHECK(pp->combo_based_estimated_miss_count ==
              pppp_test::pp_approx(row.pp_combo_based_estimated_miss_count));
        if (absent(row.pp_score_based_estimated_miss_count)) {
            CHECK(!pp->score_based_estimated_miss_count.has_value());
        } else {
            CHECK(matches(pp->score_based_estimated_miss_count, row.pp_score_based_estimated_miss_count));
        }
        CHECK(pp->aim_estimated_slider_breaks == pppp_test::pp_approx(row.pp_aim_estimated_slider_breaks));
        CHECK(pp->speed_estimated_slider_breaks ==
              pppp_test::pp_approx(row.pp_speed_estimated_slider_breaks));
        if (absent(row.pp_speed_deviation)) {
            CHECK(!pp->speed_deviation.has_value());
        } else {
            CHECK(matches(pp->speed_deviation, row.pp_speed_deviation));
        }
    }
}

TEST_CASE("taiko performance matches upstream") {
    const size_t rows = sizeof(parity::taiko_pp_rows) / sizeof(parity::taiko_pp_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::TaikoPpRow& row = parity::taiko_pp_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods << " " << row.score);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + row.map;
        pppp::common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(row.mods);
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.great;
        score.statistics[pppp::common::HIT_RESULT_OK] = row.ok;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        // The map's own maximum combo stands in when the table names none, and this mode's accuracy is
        // always the one its judgements give, never a number read from the table.
        const pppp::DifficultyAttributes attributes =
            difficulty(path.c_str(), pppp::Ruleset::TAIKO, row.mods);
        const pppp::taiko::difficulty::TaikoDifficultyAttributes* attrs = attributes.taiko();
        REQUIRE_UNARY(attrs != 0);
        score.max_combo = row.combo != 0 ? row.combo : attrs->max_combo;
        score.accuracy = (row.great * 300.0 + row.ok * 150.0) / ((row.great + row.ok + row.miss) * 300.0);

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::TAIKO, row.mods, score);
        const pppp::taiko::difficulty::TaikoPerformanceAttributes* pp = result.taiko();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(row.pp_total));
        CHECK(pp->difficulty == pppp_test::pp_approx(row.pp_difficulty));
        CHECK(pp->accuracy == pppp_test::pp_approx(row.pp_accuracy));
        if (absent(row.pp_estimated_unstable_rate)) {
            CHECK(!pp->estimated_unstable_rate.has_value());
        } else {
            CHECK(matches(pp->estimated_unstable_rate, row.pp_estimated_unstable_rate));
        }
    }
}

TEST_CASE("osu!catch performance matches upstream") {
    const size_t rows = sizeof(parity::catch_pp_rows) / sizeof(parity::catch_pp_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::CatchPpRow& row = parity::catch_pp_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods << " " << row.score);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + row.map;
        pppp::common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(row.mods);
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.fruits;
        score.statistics[pppp::common::HIT_RESULT_LARGE_TICK_HIT] = row.droplets;
        score.statistics[pppp::common::HIT_RESULT_SMALL_TICK_HIT] = row.tiny_droplets;
        score.statistics[pppp::common::HIT_RESULT_SMALL_TICK_MISS] = row.tiny_misses;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        const pppp::DifficultyAttributes attributes =
            difficulty(path.c_str(), pppp::Ruleset::CATCH, row.mods);
        const pppp::fruits::difficulty::CatchDifficultyAttributes* attrs = attributes.fruits();
        REQUIRE_UNARY(attrs != 0);
        score.max_combo = row.combo != 0 ? row.combo : attrs->max_combo;
        score.accuracy = static_cast<double>(row.fruits + row.droplets + row.tiny_droplets) /
                         (row.fruits + row.droplets + row.tiny_droplets + row.tiny_misses + row.miss);

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::CATCH, row.mods, score);
        const pppp::fruits::difficulty::CatchPerformanceAttributes* pp = result.fruits();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(row.pp_total));
    }
}

TEST_CASE("osu!mania performance matches upstream") {
    const size_t rows = sizeof(parity::mania_pp_rows) / sizeof(parity::mania_pp_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::ManiaPpRow& row = parity::mania_pp_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods << " " << row.score);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + row.map;
        pppp::common::ScoreInfo score;
        score.mods = pppp_test::parse_mods(row.mods);
        score.statistics[pppp::common::HIT_RESULT_PERFECT] = row.perfect;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = row.great;
        score.statistics[pppp::common::HIT_RESULT_GOOD] = row.good;
        score.statistics[pppp::common::HIT_RESULT_OK] = row.ok;
        score.statistics[pppp::common::HIT_RESULT_MEH] = row.meh;
        score.statistics[pppp::common::HIT_RESULT_MISS] = row.miss;
        const pppp::DifficultyAttributes attributes =
            difficulty(path.c_str(), pppp::Ruleset::MANIA, row.mods);
        const pppp::mania::difficulty::ManiaDifficultyAttributes* attrs = attributes.mania();
        REQUIRE_UNARY(attrs != 0);
        score.max_combo = row.combo != 0 ? row.combo : attrs->max_combo;
        score.accuracy =
            (row.perfect * 305.0 + row.great * 300.0 + row.good * 200.0 + row.ok * 100.0 + row.meh * 50.0) /
            ((row.perfect + row.great + row.good + row.ok + row.meh + row.miss) * 305.0);

        const pppp::PerformanceAttributes result =
            performance(path.c_str(), pppp::Ruleset::MANIA, row.mods, score);
        const pppp::mania::difficulty::ManiaPerformanceAttributes* pp = result.mania();
        REQUIRE_UNARY(pp != 0);

        CHECK(pp->total == pppp_test::pp_approx(row.pp_total));
        CHECK(pp->difficulty == pppp_test::pp_approx(row.pp_difficulty));
    }
}
