#include "common.h"
#include "maps.h"
#include "parity/difficulty.h"
#include <doctest.h>
#include <pppp/difficulty.h>
#include <string>

namespace {
    bool calculate(const char* map, pppp::Ruleset::Value ruleset, const char* mods,
                   pppp::DifficultyAttributes& out) {
        const std::string path = std::string(PPPP_TEST_RESOURCES) + map;

        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, path.c_str());

        return pppp::Difficulty()
            .mods(pppp_test::parse_mods(mods))
            .ruleset(ruleset)
            .calculate(beatmap, out)
            .ok();
    }

    bool matches(const nonstd::optional<double>& value, double expected) {
        return value.has_value() && value.value() == pppp_test::difficulty_approx(expected);
    }

    bool absent(double expected) { return expected != expected; }
} // namespace

TEST_CASE("osu! difficulty attributes match upstream") {
    const size_t rows = sizeof(parity::osu_diff_rows) / sizeof(parity::osu_diff_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::OsuDiffRow& row = parity::osu_diff_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(row.map, pppp::Ruleset::OSU, row.mods, attributes));

        const pppp::OsuDifficultyAttributes* a = attributes.osu();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(row.star_rating));
        CHECK(a->max_combo == row.max_combo);
        CHECK(a->aim_difficulty == pppp_test::difficulty_approx(row.aim_difficulty));
        CHECK(a->aim_difficult_slider_count == pppp_test::difficulty_approx(row.aim_difficult_slider_count));
        CHECK(a->speed_difficulty == pppp_test::difficulty_approx(row.speed_difficulty));
        CHECK(a->speed_note_count == pppp_test::difficulty_approx(row.speed_note_count));
        if (absent(row.flashlight_difficulty)) {
            CHECK_FALSE(a->flashlight_difficulty.has_value());
        } else {
            CHECK(matches(a->flashlight_difficulty, row.flashlight_difficulty));
        }
        CHECK(a->reading_difficulty == pppp_test::difficulty_approx(row.reading_difficulty));
        CHECK(a->slider_factor == pppp_test::difficulty_approx(row.slider_factor));
        CHECK(a->aim_top_weighted_slider_factor ==
              pppp_test::difficulty_approx(row.aim_top_weighted_slider_factor));
        CHECK(a->speed_top_weighted_slider_factor ==
              pppp_test::difficulty_approx(row.speed_top_weighted_slider_factor));
        CHECK(a->aim_difficult_strain_count == pppp_test::difficulty_approx(row.aim_difficult_strain_count));
        CHECK(a->speed_difficult_strain_count ==
              pppp_test::difficulty_approx(row.speed_difficult_strain_count));
        CHECK(a->reading_difficult_note_count ==
              pppp_test::difficulty_approx(row.reading_difficult_note_count));
        CHECK(a->hit_circle_count == row.hit_circle_count);
        CHECK(a->slider_count == row.slider_count);
        CHECK(a->spinner_count == row.spinner_count);
        CHECK(a->nested_score_per_object == pppp_test::difficulty_approx(row.nested_score_per_object));
        CHECK(a->legacy_score_base_multiplier ==
              pppp_test::difficulty_approx(row.legacy_score_base_multiplier));
        CHECK(a->maximum_legacy_combo_score == pppp_test::difficulty_approx(row.maximum_legacy_combo_score));
    }
}

TEST_CASE("taiko difficulty attributes match upstream") {
    const size_t rows = sizeof(parity::taiko_diff_rows) / sizeof(parity::taiko_diff_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::TaikoDiffRow& row = parity::taiko_diff_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(row.map, pppp::Ruleset::TAIKO, row.mods, attributes));

        const pppp::TaikoDifficultyAttributes* a = attributes.taiko();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(row.star_rating));
        CHECK(a->max_combo == row.max_combo);
        CHECK(a->mechanical_difficulty == pppp_test::difficulty_approx(row.mechanical_difficulty));
        CHECK(a->rhythm_difficulty == pppp_test::difficulty_approx(row.rhythm_difficulty));
        CHECK(a->reading_difficulty == pppp_test::difficulty_approx(row.reading_difficulty));
        CHECK(a->colour_difficulty == pppp_test::difficulty_approx(row.colour_difficulty));
        CHECK(a->stamina_difficulty == pppp_test::difficulty_approx(row.stamina_difficulty));
        CHECK(a->mono_stamina_factor == pppp_test::difficulty_approx(row.mono_stamina_factor));
        CHECK(a->consistency_factor == pppp_test::difficulty_approx(row.consistency_factor));
        CHECK(a->stamina_top_strains == pppp_test::difficulty_approx(row.stamina_top_strains));
    }
}

TEST_CASE("osu!catch difficulty attributes match upstream") {
    const size_t rows = sizeof(parity::catch_diff_rows) / sizeof(parity::catch_diff_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::CatchDiffRow& row = parity::catch_diff_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(row.map, pppp::Ruleset::CATCH, row.mods, attributes));

        const pppp::CatchDifficultyAttributes* a = attributes.fruits();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(row.star_rating));
        CHECK(a->max_combo == row.max_combo);
    }
}

TEST_CASE("osu!mania difficulty attributes match upstream") {
    const size_t rows = sizeof(parity::mania_diff_rows) / sizeof(parity::mania_diff_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::ManiaDiffRow& row = parity::mania_diff_rows[i];
        INFO(row.name << " " << row.map << " " << row.mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(row.map, pppp::Ruleset::MANIA, row.mods, attributes));

        const pppp::ManiaDifficultyAttributes* a = attributes.mania();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(row.star_rating));
        CHECK(a->max_combo == row.max_combo);
    }
}
