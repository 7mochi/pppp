#include "maps.h"
#include "pppp/beatmap.h"
#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/osu/difficulty/osu_difficulty_calculator.h"
#include "pppp/taiko/difficulty/taiko_difficulty_calculator.h"
#include <cstddef>
#include <doctest.h>
#include <fosu/parser.h>
#include <vector>

TEST_CASE("fosu parses 2785319.osu with every calculation on") {
    ::fosu::Parser parser;
    ::fosu::ParseOptions options;

    options.sections = ::fosu::kAllSections;
    options.calculate_slider_end_times = true;
    options.calculate_slider_paths = true;
    options.calculate_slider_events = true;
    options.apply_stacking = true;

    const ::fosu::Result< ::fosu::Beatmap*> result =
        parser.parse_file(PPPP_TEST_RESOURCES "/osu/2785319.osu", options);

    REQUIRE(result.ok());

    const ::fosu::Beatmap& map = *result.value();

    CHECK(map.hit_objects.size() == 601);
    CHECK_FALSE(map.timing_points.empty());
    CHECK_FALSE(map.sliders.empty());
    CHECK_FALSE(map.stacking.empty());
    CHECK_FALSE(map.slider_paths.empty());
    CHECK_FALSE(map.slider_events.empty());
    CHECK_FALSE(map.raw_positions.empty());
    CHECK_FALSE(map.raw_slider_points.empty());
    CHECK(map.ar > 0.0);
    CHECK(map.od > 0.0);
    CHECK(map.cs > 0.0);
}

TEST_CASE("from_file fills a beatmap and clear() empties it") {
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");

    CHECK(beatmap.hit_objects.size() == 601);
    CHECK(beatmap.difficulty.approach_rate > 0.0);
    CHECK(beatmap.difficulty.overall_difficulty > 0.0);
    CHECK(beatmap.difficulty.circle_size > 0.0);
    CHECK(beatmap.slider_path.recompute);
    CHECK(beatmap.slider_path.release);
    CHECK_FALSE(beatmap.timing_points.empty());
    CHECK(beatmap.hit_objects[0].position.x >= 0.0f);
    CHECK(beatmap.hit_objects[0].position.y >= 0.0f);
    CHECK(beatmap.hit_objects[0].start_time >= 0.0);

    beatmap.clear();
    CHECK_FALSE(beatmap.slider_path.recompute);
    CHECK(beatmap.hit_objects.empty());
}

TEST_CASE("the mode and the hit sounds survive the adapter") {
    pppp::beatmaps::Beatmap beatmap;

    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");

    CHECK(beatmap.mode == 0);
    REQUIRE(beatmap.hit_objects.size() > 8);

    CHECK(beatmap.hit_objects[0].hitsound == 6);

    const pppp::beatmaps::HitObject& first_slider = beatmap.hit_objects[8];

    REQUIRE(first_slider.slider >= 0);

    const pppp::beatmaps::Slider& slider = beatmap.sliders[first_slider.slider];

    REQUIRE(slider.node_sounds.size() == 2);
    CHECK(slider.node_sounds[0] == 2);
    CHECK(slider.node_sounds[1] == 0);
}

TEST_CASE("from_parsed copies what the parser produced") {
    ::fosu::Parser parser;
    ::fosu::ParseOptions options;

    options.sections = ::fosu::kAllSections;
    options.calculate_slider_end_times = true;
    options.calculate_slider_paths = true;
    options.calculate_slider_events = true;
    options.apply_stacking = true;

    const ::fosu::Result< ::fosu::Beatmap*> result =
        parser.parse_file(PPPP_TEST_RESOURCES "/osu/2785319.osu", options);

    REQUIRE(result.ok());

    const ::fosu::Beatmap* parsed = result.value();
    pppp::beatmaps::Beatmap beatmap;

    REQUIRE(pppp::beatmaps::from_parsed(beatmap, parsed) == pppp::Result::OK);
    CHECK(beatmap.hit_objects.size() == parsed->hit_objects.size());
    CHECK(beatmap.sliders.size() == parsed->sliders.size());
    REQUIRE(beatmap.slider_path.recompute);

    SUBCASE("every slider carries a path") {
        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            if (!(beatmap.hit_objects[i].type & 2)) {
                continue;
            }

            const int s = beatmap.hit_objects[i].slider;

            CAPTURE(i);
            REQUIRE(s >= 0);
            REQUIRE(static_cast<size_t>(s) < beatmap.sliders.size());
            CHECK_FALSE(beatmap.sliders[s].path.empty());
        }
    }

    SUBCASE("recomputing a path reproduces the parsed one") {
        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            const int s = beatmap.hit_objects[i].slider;

            if (s < 0) {
                continue;
            }

            const pppp::beatmaps::Slider& slider = beatmap.sliders[s];
            std::vector<pppp::utils::Vector2> relative(slider.control_points.size());
            std::vector<pppp::utils::Vector2> path;
            std::vector<double> lengths;

            for (size_t k = 0; k < relative.size(); k++) {
                relative[k].x = slider.control_points[k].x - beatmap.hit_objects[i].position.x;
                relative[k].y = slider.control_points[k].y - beatmap.hit_objects[i].position.y;
            }

            CAPTURE(i);
            REQUIRE(beatmap.slider_path.recompute(beatmap.slider_path.ctx, static_cast<unsigned>(i),
                                                  relative.empty() ? 0 : &relative[0], relative.size(), &path,
                                                  &lengths) == 0);
            REQUIRE(path.size() == slider.path.size());
            CHECK(lengths == slider.cumulative_lengths);
            for (size_t k = 0; k < path.size(); k++) {
                CAPTURE(k);
                CHECK(path[k].x == slider.path[k].x);
                CHECK(path[k].y == slider.path[k].y);
            }
        }
    }
}

TEST_CASE("bad input is rejected") {
    pppp::beatmaps::Beatmap beatmap;

    CHECK(pppp::beatmaps::from_file(beatmap, "nonexistent.osu") == pppp::Result::PARSE);
    CHECK(pppp::beatmaps::from_file(beatmap, 0) == pppp::Result::INVALID_ARGUMENT);
    CHECK(pppp::beatmaps::from_bytes(beatmap, 0, 1) == pppp::Result::PARSE);
    CHECK(pppp::beatmaps::from_parsed(beatmap, 0) == pppp::Result::INVALID_ARGUMENT);
}

namespace {
    struct Decoded {
        const char* map;
        int mode;
        int version;
        float approach_rate;
        float overall_difficulty;
        float circle_size;
        float drain_rate;
        float slider_multiplier;
        float slider_tick_rate;
        size_t hit_objects;
        int timing_points;
        float stack_leniency;
        size_t breaks;
    };

    int uninherited_lines(const pppp::beatmaps::Beatmap& beatmap) {
        int count = 0;

        for (size_t i = 0; i < beatmap.timing_points.size(); i++) {
            count += beatmap.timing_points[i].uninherited ? 1 : 0;
        }
        return count;
    }
    void check(const Decoded& row) {
        pppp::beatmaps::Beatmap beatmap;

        pppp_test::load(beatmap, row.map);
        INFO(row.map);
        CHECK(beatmap.mode == row.mode);
        CHECK(beatmap.format_version == row.version);
        CHECK(static_cast<float>(beatmap.difficulty.approach_rate) == row.approach_rate);
        CHECK(static_cast<float>(beatmap.difficulty.overall_difficulty) == row.overall_difficulty);
        CHECK(static_cast<float>(beatmap.difficulty.circle_size) == row.circle_size);
        CHECK(static_cast<float>(beatmap.difficulty.drain_rate) == row.drain_rate);
        CHECK(static_cast<float>(beatmap.difficulty.slider_multiplier) == row.slider_multiplier);
        CHECK(static_cast<float>(beatmap.difficulty.slider_tick_rate) == row.slider_tick_rate);
        CHECK(beatmap.hit_objects.size() == row.hit_objects);
        CHECK(uninherited_lines(beatmap) == row.timing_points);
        CHECK(static_cast<float>(beatmap.stack_leniency) == row.stack_leniency);
        CHECK(beatmap.breaks.size() == row.breaks);
    }

    void load_empty(pppp::beatmaps::Beatmap& beatmap) {
        ::fosu::Parser parser;
        const ::fosu::Result< ::fosu::Beatmap*> parsed = parser.parse("", 0, ::fosu::ParseOptions());

        REQUIRE(parsed.ok());
        REQUIRE(pppp::beatmaps::from_parsed(beatmap, parsed.value()) == pppp::Result::OK);
    }
} // namespace

TEST_CASE("osu") {
    const Decoded row = {
        PPPP_TEST_RESOURCES "/osu/2785319.osu", 0, 14, 9.3f, 8.8f, 4.5f, 5.0f, 1.7f, 1.0f, 601, 1, 0.5f, 1};

    check(row);
}

TEST_CASE("taiko") {
    const Decoded row = {
        PPPP_TEST_RESOURCES "/taiko/1028484.osu", 1, 14, 8.0f, 5.0f, 2.0f, 6.0f, 1.4f, 1.0f, 295, 1, 0.7f, 0};

    check(row);
}

TEST_CASE("catch") {
    const Decoded row = {PPPP_TEST_RESOURCES "/fruits/2118524.osu",
                         2,
                         14,
                         8.0f,
                         8.0f,
                         3.5f,
                         5.0f,
                         1.45f,
                         1.0f,
                         477,
                         1,
                         0.7f,
                         0};

    check(row);
}

TEST_CASE("mania") {
    const Decoded row = {
        PPPP_TEST_RESOURCES "/mania/1638954.osu", 3, 14, 5.0f, 8.0f, 4.0f, 8.0f, 1.4f, 1.0f, 594, 1, 0.7f, 0};

    check(row);
}

TEST_CASE("empty_osu") {
    pppp::beatmaps::Beatmap beatmap;

    load_empty(beatmap);
    pppp::osu::difficulty::OsuDifficultyAttributes attrs;

    CHECK(pppp::osu::difficulty::calculate_difficulty(attrs, beatmap, 0, 0) == pppp::Result::OK);
}

TEST_CASE("empty_taiko") {
    pppp::beatmaps::Beatmap beatmap;

    load_empty(beatmap);
    pppp::taiko::difficulty::TaikoDifficultyAttributes attrs;

    CHECK(pppp::taiko::difficulty::calculate_difficulty(attrs, beatmap, 0, 0) == pppp::Result::OK);
}

TEST_CASE("empty_catch") {
    pppp::beatmaps::Beatmap beatmap;

    load_empty(beatmap);
    pppp::fruits::difficulty::CatchDifficultyAttributes attrs;

    CHECK(pppp::fruits::difficulty::calculate_difficulty(attrs, beatmap, 0, 0) == pppp::Result::OK);
}

TEST_CASE("empty_mania") {
    pppp::beatmaps::Beatmap beatmap;

    load_empty(beatmap);
    pppp::mania::difficulty::ManiaDifficultyAttributes attrs;

    CHECK(pppp::mania::difficulty::calculate_difficulty(attrs, beatmap, 0, 0) == pppp::Result::OK);
}
