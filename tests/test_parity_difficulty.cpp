#include "common.h"
#include "maps.h"
#include "parity.h"
#include <doctest.h>
#include <pppp/difficulty.h>
#include <string>

namespace {
    bool calculate(const char* map, pppp::Ruleset::Value ruleset, const char* mods,
                   pppp::DifficultyAttributes& out) {
        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + map;

        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, path.c_str());

        return pppp::Difficulty()
            .mods(pppp_test::parse_mods(mods))
            .ruleset(ruleset)
            .calculate(beatmap, out)
            .ok();
    }
} // namespace

TEST_CASE("osu! difficulty attributes match upstream") {
    const parity::Table rows("difficulty", "osu");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(map, pppp::Ruleset::OSU, mods, attributes));

        const pppp::OsuDifficultyAttributes* a = attributes.osu();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
        CHECK(a->max_combo == expected.integer("max_combo"));
        CHECK(a->aim_difficulty == pppp_test::difficulty_approx(expected.number("aim_difficulty")));
        CHECK(a->aim_difficult_slider_count ==
              pppp_test::difficulty_approx(expected.number("aim_difficult_slider_count")));
        CHECK(a->speed_difficulty == pppp_test::difficulty_approx(expected.number("speed_difficulty")));
        CHECK(a->speed_note_count == pppp_test::difficulty_approx(expected.number("speed_note_count")));
        CHECK(a->flashlight_difficulty ==
              pppp_test::difficulty_approx(expected.number("flashlight_difficulty")));
        CHECK(a->reading_difficulty == pppp_test::difficulty_approx(expected.number("reading_difficulty")));
        CHECK(a->slider_factor == pppp_test::difficulty_approx(expected.number("slider_factor")));
        CHECK(a->aim_top_weighted_slider_factor ==
              pppp_test::difficulty_approx(expected.number("aim_top_weighted_slider_factor")));
        CHECK(a->speed_top_weighted_slider_factor ==
              pppp_test::difficulty_approx(expected.number("speed_top_weighted_slider_factor")));
        CHECK(a->aim_difficult_strain_count ==
              pppp_test::difficulty_approx(expected.number("aim_difficult_strain_count")));
        CHECK(a->speed_difficult_strain_count ==
              pppp_test::difficulty_approx(expected.number("speed_difficult_strain_count")));
        CHECK(a->reading_difficult_note_count ==
              pppp_test::difficulty_approx(expected.number("reading_difficult_note_count")));
        CHECK(a->hit_circle_count == expected.integer("hit_circle_count"));
        CHECK(a->slider_count == expected.integer("slider_count"));
        CHECK(a->spinner_count == expected.integer("spinner_count"));
        CHECK(a->nested_score_per_object ==
              pppp_test::difficulty_approx(expected.number("nested_score_per_object")));
        CHECK(a->legacy_score_base_multiplier ==
              pppp_test::difficulty_approx(expected.number("legacy_score_base_multiplier")));
        CHECK(a->maximum_legacy_combo_score ==
              pppp_test::difficulty_approx(expected.number("maximum_legacy_combo_score")));
    }
}

TEST_CASE("taiko difficulty attributes match upstream") {
    const parity::Table rows("difficulty", "taiko");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(map, pppp::Ruleset::TAIKO, mods, attributes));

        const pppp::TaikoDifficultyAttributes* a = attributes.taiko();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
        CHECK(a->max_combo == expected.integer("max_combo"));
        CHECK(a->mechanical_difficulty ==
              pppp_test::difficulty_approx(expected.number("mechanical_difficulty")));
        CHECK(a->rhythm_difficulty == pppp_test::difficulty_approx(expected.number("rhythm_difficulty")));
        CHECK(a->reading_difficulty == pppp_test::difficulty_approx(expected.number("reading_difficulty")));
        CHECK(a->colour_difficulty == pppp_test::difficulty_approx(expected.number("colour_difficulty")));
        CHECK(a->stamina_difficulty == pppp_test::difficulty_approx(expected.number("stamina_difficulty")));
        CHECK(a->mono_stamina_factor == pppp_test::difficulty_approx(expected.number("mono_stamina_factor")));
        CHECK(a->consistency_factor == pppp_test::difficulty_approx(expected.number("consistency_factor")));
        CHECK(a->stamina_top_strains == pppp_test::difficulty_approx(expected.number("stamina_top_strains")));
    }
}

TEST_CASE("osu!catch difficulty attributes match upstream") {
    const parity::Table rows("difficulty", "catch");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(map, pppp::Ruleset::CATCH, mods, attributes));

        const pppp::CatchDifficultyAttributes* a = attributes.fruits();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
        CHECK(a->max_combo == expected.integer("max_combo"));
    }
}

TEST_CASE("osu!mania difficulty attributes match upstream") {
    const parity::Table rows("difficulty", "mania");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map = row.text("map");
        const char* mods = row.text("mods");
        INFO(name << " " << map << " " << mods);

        pppp::DifficultyAttributes attributes;
        REQUIRE(calculate(map, pppp::Ruleset::MANIA, mods, attributes));

        const pppp::ManiaDifficultyAttributes* a = attributes.mania();
        REQUIRE_UNARY(a != 0);

        CHECK(a->star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
        CHECK(a->max_combo == expected.integer("max_combo"));
    }
}
