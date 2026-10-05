#include "common.h"
#include "maps.h"
#include "pppp/capi.h"
#include "pppp/pppp.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <doctest.h>
#include <vector>

namespace {
    struct ModsHandle {
        pppp_mods* handle;

        explicit ModsHandle(const char* spec)
            : handle(0) {
            INFO(spec);
            REQUIRE(pppp_mods_parse(spec, &handle) == PPPP_OK);
        }

        ~ModsHandle() { pppp_mods_free(handle); }

    private:
        ModsHandle(const ModsHandle&);
        ModsHandle& operator=(const ModsHandle&);
    };

    struct StarsRow {
        const char* map;
        const char* mods;
        int ruleset;
        double star_rating;
        int max_combo;
    };

    void check_stars(const StarsRow& row) {
        pppp_beatmap* map = 0;

        INFO(row.map);
        INFO(row.mods);
        REQUIRE(pppp_beatmap_from_file(row.map, &map) == PPPP_OK);
        REQUIRE(map);

        const ModsHandle mods(row.mods);
        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = mods.handle;
        options.ruleset = row.ruleset;
        options.has_ruleset = 1;

        pppp_difficulty_attributes attributes;
        REQUIRE(pppp_calculate_difficulty(map, &options, &attributes) == PPPP_OK);
        pppp_beatmap_free(map);

        CHECK(attributes.ruleset == row.ruleset);
        CHECK(pppp_test::distance(attributes.star_rating, row.star_rating) <= 0.00001);
        CHECK(attributes.max_combo == row.max_combo);
    }

    void check_difficulty_round_trip(const char* path, pppp_int32 ruleset) {
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, path);

        const pppp::DifficultyAttributes expected =
            pppp_test::calculate(pppp::Difficulty()
                                     .mods(pppp_test::parse_mods("HD,DT"))
                                     .ruleset(static_cast<pppp::Ruleset::Value>(ruleset))
                                     .clock_rate(1.5),
                                 reference);
        const pppp::OsuDifficultyAttributes osu =
            expected.osu() ? *expected.osu() : pppp::OsuDifficultyAttributes();
        const pppp::TaikoDifficultyAttributes taiko =
            expected.taiko() ? *expected.taiko() : pppp::TaikoDifficultyAttributes();
        const pppp::CatchDifficultyAttributes fruits =
            expected.fruits() ? *expected.fruits() : pppp::CatchDifficultyAttributes();
        const pppp::ManiaDifficultyAttributes mania =
            expected.mania() ? *expected.mania() : pppp::ManiaDifficultyAttributes();

        const ModsHandle mods("HD,DT");
        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = mods.handle;
        options.ruleset = ruleset;
        options.has_ruleset = 1;
        options.clock_rate = 1.5;
        options.has_clock_rate = 1;

        pppp_difficulty_attributes attributes;
        REQUIRE(pppp_calculate_difficulty(map, &options, &attributes) == PPPP_OK);
        pppp_beatmap_free(map);

        CHECK(attributes.ruleset == static_cast<pppp_int32>(expected.ruleset()));
        CHECK(attributes.star_rating == pppp_test::difficulty_approx(expected.star_rating()));
        CHECK(attributes.max_combo == expected.max_combo());
        switch (ruleset) {
        case PPPP_RULESET_OSU:
            CHECK(attributes.osu.aim_difficulty == pppp_test::difficulty_approx(osu.aim_difficulty));
            CHECK(attributes.osu.speed_difficulty == pppp_test::difficulty_approx(osu.speed_difficulty));
            CHECK(attributes.osu.aim_difficult_slider_count ==
                  pppp_test::difficulty_approx(osu.aim_difficult_slider_count));
            CHECK(attributes.osu.hit_circle_count == osu.hit_circle_count);
            CHECK(attributes.osu.slider_count == osu.slider_count);
            break;
        case PPPP_RULESET_TAIKO:
            CHECK(attributes.taiko.mechanical_difficulty ==
                  pppp_test::difficulty_approx(taiko.mechanical_difficulty));
            CHECK(attributes.taiko.stamina_top_strains ==
                  pppp_test::difficulty_approx(taiko.stamina_top_strains));
            break;
        case PPPP_RULESET_CATCH:
            CHECK(attributes.fruits.star_rating == pppp_test::difficulty_approx(fruits.star_rating));
            break;
        default:
            CHECK(attributes.mania.star_rating == pppp_test::difficulty_approx(mania.star_rating));
            break;
        }
    }
} // namespace

TEST_SUITE("CapiTest") {
    TEST_CASE("the version is a version") {
        const char* version = pppp_version();

        REQUIRE(version);
        int dots = 0;
        for (const char* character = version; *character != '\0'; character++) {
            if (*character == '.') {
                dots++;
            }
        }
        CHECK(dots == 2);
    }

    TEST_CASE("every ruleset reproduces the pinned star ratings") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "NM", PPPP_RULESET_OSU, 6.5243170265483581, 239},
            {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "DT", PPPP_RULESET_OSU, 9.4677607900646308, 239},
            {PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", "NM", PPPP_RULESET_TAIKO, 3.3190848563395079,
             200},
            {PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", "DT", PPPP_RULESET_TAIKO, 4.4551414906554987,
             200},
            {PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", "NM", PPPP_RULESET_CATCH, 4.039861734717169,
             127},
            {PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", "DT", PPPP_RULESET_CATCH, 5.1527173897800873,
             127},
            {PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", "NM", PPPP_RULESET_MANIA, 2.3493769750220914,
             242},
            {PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", "DT", PPPP_RULESET_MANIA, 2.797245912537965,
             242},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_stars(rows[i]);
        }
    }

    TEST_CASE("a converted map matches the library for every ruleset") {
        check_difficulty_round_trip(PPPP_TEST_RESOURCES "/osu/2785319.osu", PPPP_RULESET_OSU);
        check_difficulty_round_trip(PPPP_TEST_RESOURCES "/osu/2785319.osu", PPPP_RULESET_TAIKO);
        check_difficulty_round_trip(PPPP_TEST_RESOURCES "/osu/2785319.osu", PPPP_RULESET_CATCH);
        check_difficulty_round_trip(PPPP_TEST_RESOURCES "/osu/2785319.osu", PPPP_RULESET_MANIA);
    }

    TEST_CASE("null options mean no mods and the map's own ruleset") {
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu");
        const pppp::DifficultyAttributes expected = pppp_test::calculate(pppp::Difficulty(), reference);

        pppp_difficulty_attributes attributes;
        REQUIRE(pppp_calculate_difficulty(map, 0, &attributes) == PPPP_OK);
        pppp_beatmap_free(map);

        CHECK(attributes.ruleset == static_cast<pppp_int32>(expected.ruleset()));
        CHECK(attributes.star_rating == pppp_test::difficulty_approx(expected.star_rating()));
        CHECK(attributes.max_combo == expected.max_combo());
    }

    TEST_CASE("the beatmap data crosses the ABI") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, path);

        CHECK(pppp_beatmap_format_version(map) == reference.format_version);
        CHECK(pppp_beatmap_mode(map) == reference.mode);
        CHECK(pppp_beatmap_stack_leniency(map) == reference.stack_leniency);

        pppp_beatmap_difficulty settings;
        REQUIRE(pppp_beatmap_get_difficulty(map, &settings) == PPPP_OK);
        CHECK(settings.drain_rate == reference.difficulty.drain_rate);
        CHECK(settings.circle_size == reference.difficulty.circle_size);
        CHECK(settings.overall_difficulty == reference.difficulty.overall_difficulty);
        CHECK(settings.approach_rate == reference.difficulty.approach_rate);
        CHECK(settings.slider_multiplier == reference.difficulty.slider_multiplier);
        CHECK(settings.slider_tick_rate == reference.difficulty.slider_tick_rate);

        const pppp_hit_object* hit_objects = 0;
        size_t hit_object_count = 0;
        REQUIRE(pppp_beatmap_hit_objects(map, &hit_objects, &hit_object_count) == PPPP_OK);
        REQUIRE(hit_object_count == reference.hit_objects.size());
        REQUIRE(hit_object_count > 0);
        CHECK(hit_objects[0].position.x == reference.hit_objects[0].position.x);
        CHECK(hit_objects[0].position.y == reference.hit_objects[0].position.y);
        CHECK(hit_objects[0].type == reference.hit_objects[0].type);
        CHECK(hit_objects[0].hitsound == reference.hit_objects[0].hitsound);
        CHECK(hit_objects[0].start_time == reference.hit_objects[0].start_time);
        CHECK(hit_objects[0].end_time == reference.hit_objects[0].end_time);
        CHECK(hit_objects[0].new_combo == (reference.hit_objects[0].new_combo ? 1 : 0));
        CHECK(hit_objects[0].combo_offset == reference.hit_objects[0].combo_offset);
        CHECK(hit_objects[0].slider == reference.hit_objects[0].slider);

        const pppp_timing_point* timing_points = 0;
        size_t timing_point_count = 0;
        REQUIRE(pppp_beatmap_timing_points(map, &timing_points, &timing_point_count) == PPPP_OK);
        REQUIRE(timing_point_count == reference.timing_points.size());
        REQUIRE(timing_point_count > 0);
        CHECK(timing_points[0].time == reference.timing_points[0].time);
        CHECK(timing_points[0].beat_length == reference.timing_points[0].beat_length);
        CHECK(timing_points[0].meter == reference.timing_points[0].meter);
        CHECK(timing_points[0].uninherited == (reference.timing_points[0].uninherited ? 1 : 0));
        CHECK(timing_points[0].effects == reference.timing_points[0].effects);

        const pppp_break_period* breaks = 0;
        size_t break_count = 0;
        REQUIRE(pppp_beatmap_breaks(map, &breaks, &break_count) == PPPP_OK);
        REQUIRE(break_count == reference.breaks.size());

        pppp_beatmap_free(map);
    }

    TEST_CASE("every slider array crosses with its full contents") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, path);

        const pppp_slider* sliders = 0;
        size_t slider_count = 0;
        REQUIRE(pppp_beatmap_sliders(map, &sliders, &slider_count) == PPPP_OK);
        REQUIRE(slider_count == reference.sliders.size());
        REQUIRE(slider_count > 0);

        for (size_t i = 0; i < slider_count; i++) {
            const pppp::beatmaps::Slider& expected = reference.sliders[i];

            CHECK(sliders[i].slides == expected.slides);
            CHECK(sliders[i].expected_length == expected.expected_length);
            REQUIRE(sliders[i].node_sound_count == expected.node_sounds.size());
            REQUIRE(sliders[i].control_point_count == expected.control_points.size());
            REQUIRE(sliders[i].path_count == expected.path.size());
            REQUIRE(sliders[i].cumulative_length_count == expected.cumulative_lengths.size());
            REQUIRE(sliders[i].undecimated_path_count == expected.undecimated_path.size());
            REQUIRE(sliders[i].undecimated_cumulative_length_count ==
                    expected.undecimated_cumulative_lengths.size());
            REQUIRE(sliders[i].event_count == expected.events.size());
            REQUIRE(sliders[i].catch_event_count == expected.catch_events.size());

            for (size_t j = 0; j < sliders[i].path_count; j++) {
                CHECK(sliders[i].path[j].x == expected.path[j].x);
                CHECK(sliders[i].path[j].y == expected.path[j].y);
            }
            for (size_t j = 0; j < sliders[i].cumulative_length_count; j++) {
                CHECK(sliders[i].cumulative_lengths[j] == expected.cumulative_lengths[j]);
            }
            for (size_t j = 0; j < sliders[i].event_count; j++) {
                CHECK(sliders[i].events[j].type == static_cast<pppp_int32>(expected.events[j].type));
                CHECK(sliders[i].events[j].time == expected.events[j].time);
                CHECK(sliders[i].events[j].span_index == expected.events[j].span_index);
                CHECK(sliders[i].events[j].span_start_time == expected.events[j].span_start_time);
                CHECK(sliders[i].events[j].path_progress == expected.events[j].path_progress);
                CHECK(sliders[i].events[j].position.x == expected.events[j].position.x);
            }
        }

        pppp_beatmap_free(map);
    }

    TEST_CASE("the performance attributes match the library, optionals included") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, path);

        pppp::common::ScoreInfo score;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = 300;
        score.statistics[pppp::common::HIT_RESULT_OK] = 20;
        score.statistics[pppp::common::HIT_RESULT_MEH] = 3;
        score.statistics[pppp::common::HIT_RESULT_MISS] = 1;
        score.max_combo = 500;
        score.accuracy = pppp_test::classic_accuracy(300, 20, 3, 1);

        const pppp::PerformanceAttributes performance = pppp_test::calculate(
            pppp::Performance(reference).score(score).mods(pppp_test::parse_mods("HD,DT")).combo(500));
        REQUIRE(performance.osu());
        const pppp::OsuPerformanceAttributes& expected = *performance.osu();

        const ModsHandle mods("HD,DT");
        pppp_performance_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = mods.handle;
        options.has_score = 1;
        for (int i = 0; i < PPPP_HIT_RESULT_COUNT; i++) {
            options.score.statistics[i] = score.statistics[i];
            options.score.maximum_statistics[i] = score.maximum_statistics[i];
        }
        options.score.max_combo = score.max_combo;
        options.score.accuracy = score.accuracy;
        options.has_combo = 1;
        options.combo = 500;

        pppp_performance_attributes attributes;
        REQUIRE(pppp_calculate_performance(map, &options, &attributes) == PPPP_OK);
        pppp_beatmap_free(map);

        CHECK(attributes.ruleset == static_cast<pppp_int32>(performance.ruleset()));
        CHECK(attributes.total == pppp_test::pp_approx(performance.total()));
        CHECK(attributes.osu.aim == pppp_test::pp_approx(expected.aim));
        CHECK(attributes.osu.speed == pppp_test::pp_approx(expected.speed));
        CHECK(attributes.osu.accuracy == pppp_test::pp_approx(expected.accuracy));
        CHECK(attributes.osu.effective_miss_count == pppp_test::pp_approx(expected.effective_miss_count));
        CHECK(attributes.osu.has_score_based_estimated_miss_count ==
              (expected.score_based_estimated_miss_count.has_value() ? 1 : 0));
        CHECK(attributes.osu.has_speed_deviation == (expected.speed_deviation.has_value() ? 1 : 0));
    }

    TEST_CASE("performance from attributes skips the difficulty calculation") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, path);

        pppp::common::ScoreInfo score;
        score.statistics[pppp::common::HIT_RESULT_GREAT] = 300;
        score.statistics[pppp::common::HIT_RESULT_OK] = 20;
        score.statistics[pppp::common::HIT_RESULT_MEH] = 3;
        score.statistics[pppp::common::HIT_RESULT_MISS] = 1;
        score.max_combo = 500;
        score.accuracy = pppp_test::classic_accuracy(300, 20, 3, 1);

        const ModsHandle mods("HD,DT");
        pppp_performance_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = mods.handle;
        options.has_score = 1;
        for (int i = 0; i < PPPP_HIT_RESULT_COUNT; i++) {
            options.score.statistics[i] = score.statistics[i];
            options.score.maximum_statistics[i] = score.maximum_statistics[i];
        }
        options.score.max_combo = score.max_combo;
        options.score.accuracy = score.accuracy;
        options.has_combo = 1;
        options.combo = 500;

        pppp_performance_attributes from_map;
        REQUIRE(pppp_calculate_performance(map, &options, &from_map) == PPPP_OK);

        pppp_difficulty_attributes difficulty;
        {
            pppp_difficulty_options difficulty_options;
            std::memset(&difficulty_options, 0, sizeof(difficulty_options));
            difficulty_options.mods = mods.handle;
            REQUIRE(pppp_calculate_difficulty(map, &difficulty_options, &difficulty) == PPPP_OK);
        }

        pppp_performance_options with_attributes = options;
        with_attributes.difficulty = &difficulty;
        with_attributes.has_difficulty = 1;

        pppp_performance_attributes from_attributes;
        REQUIRE(pppp_calculate_performance(map, &with_attributes, &from_attributes) == PPPP_OK);

        CHECK(from_attributes.ruleset == from_map.ruleset);
        CHECK(from_attributes.total == from_map.total);
        CHECK(from_attributes.osu.aim == from_map.osu.aim);
        CHECK(from_attributes.osu.speed == from_map.osu.speed);
        CHECK(from_attributes.osu.accuracy == from_map.osu.accuracy);
        CHECK(from_attributes.osu.reading == from_map.osu.reading);
        CHECK(from_attributes.osu.effective_miss_count == from_map.osu.effective_miss_count);
        CHECK(from_attributes.osu.has_speed_deviation == from_map.osu.has_speed_deviation);
        CHECK(from_attributes.osu.speed_deviation == from_map.osu.speed_deviation);

        pppp_beatmap_free(map);
    }

    TEST_CASE("attributes from a converted map carry their ruleset") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp::Beatmap reference;
        pppp_test::load(reference, path);

        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.ruleset = PPPP_RULESET_TAIKO;
        options.has_ruleset = 1;

        pppp_difficulty_attributes difficulty;
        REQUIRE(pppp_calculate_difficulty(map, &options, &difficulty) == PPPP_OK);
        REQUIRE(difficulty.ruleset == PPPP_RULESET_TAIKO);

        const pppp::DifficultyAttributes expected =
            pppp_test::calculate(pppp::Difficulty().ruleset(pppp::Ruleset::TAIKO), reference);
        const pppp::PerformanceAttributes expected_performance =
            pppp_test::calculate(pppp::Performance(reference, expected));

        pppp_performance_options performance_options;
        std::memset(&performance_options, 0, sizeof(performance_options));
        performance_options.difficulty = &difficulty;
        performance_options.has_difficulty = 1;

        pppp_performance_attributes attributes;
        REQUIRE(pppp_calculate_performance(map, &performance_options, &attributes) == PPPP_OK);

        CHECK(attributes.ruleset == PPPP_RULESET_TAIKO);
        CHECK(attributes.total == pppp_test::pp_approx(expected_performance.total()));
        REQUIRE(expected_performance.taiko());
        CHECK(attributes.taiko.difficulty == pppp_test::pp_approx(expected_performance.taiko()->difficulty));

        pppp_beatmap_free(map);
    }

    TEST_CASE("provided attributes with an out-of-range ruleset are rejected") {
        const char* path = PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        pppp_difficulty_attributes difficulty;
        std::memset(&difficulty, 0, sizeof(difficulty));
        difficulty.ruleset = 4;

        pppp_performance_options options;
        std::memset(&options, 0, sizeof(options));
        options.difficulty = &difficulty;
        options.has_difficulty = 1;

        pppp_performance_attributes attributes;
        CHECK(pppp_calculate_performance(map, &options, &attributes) == PPPP_INVALID_ARGUMENT);

        pppp_beatmap_free(map);
    }

    TEST_CASE("a missing file fails and leaves the handle untouched") {
        pppp_beatmap* map = 0;

        CHECK(pppp_beatmap_from_file(PPPP_TEST_RESOURCES "/osu/does-not-exist.osu", &map) != PPPP_OK);
        CHECK_FALSE(map);
    }

    TEST_CASE("a null beatmap is rejected instead of crashing") {
        pppp_difficulty_attributes attributes;

        CHECK(pppp_beatmap_from_file(0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_beatmap_format_version(0) == 0);
        CHECK(pppp_beatmap_mode(0) == 0);
        CHECK(pppp_beatmap_stack_leniency(0) == 0.0);
        CHECK(pppp_beatmap_get_difficulty(0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_beatmap_hit_objects(0, 0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_beatmap_sliders(0, 0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_beatmap_timing_points(0, 0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_beatmap_breaks(0, 0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_calculate_difficulty(0, 0, &attributes) == PPPP_INVALID_ARGUMENT);
        pppp_beatmap_free(0);
    }

    TEST_CASE("an out-of-range ruleset and a broken mod string are rejected") {
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == PPPP_OK);

        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.ruleset = 4;
        options.has_ruleset = 1;
        pppp_difficulty_attributes attributes;
        CHECK(pppp_calculate_difficulty(map, &options, &attributes) == PPPP_INVALID_ARGUMENT);

        pppp_mods* mods = 0;
        CHECK(pppp_mods_parse("NO_SUCH_MOD", &mods) == PPPP_INVALID_ARGUMENT);
        CHECK(!mods);
        CHECK(pppp_mods_parse(0, &mods) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_mods_from_legacy(0, 0) == PPPP_INVALID_ARGUMENT);
        pppp_mods_free(0);

        pppp_beatmap_free(map);
    }

    TEST_CASE("legacy mods match their acronyms") {
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == PPPP_OK);

        const ModsHandle acronyms("HD,DT");
        pppp_mods* legacy = 0;
        REQUIRE(pppp_mods_from_legacy(8u | 64u, &legacy) == PPPP_OK);

        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = acronyms.handle;
        pppp_difficulty_attributes from_acronyms;
        REQUIRE(pppp_calculate_difficulty(map, &options, &from_acronyms) == PPPP_OK);

        options.mods = legacy;
        pppp_difficulty_attributes from_legacy;
        REQUIRE(pppp_calculate_difficulty(map, &options, &from_legacy) == PPPP_OK);
        pppp_mods_free(legacy);
        pppp_beatmap_free(map);

        CHECK(from_legacy.star_rating == from_acronyms.star_rating);
        CHECK(from_legacy.osu.reading_difficulty == from_acronyms.osu.reading_difficulty);
    }

    TEST_CASE("a beatmap from memory matches the one from its file") {
        const char* path = PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu";
        std::FILE* file = std::fopen(path, "rb");
        REQUIRE(file);
        std::vector<char> data;
        char buffer[4096];
        size_t read = 0;
        while ((read = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
            data.insert(data.end(), buffer, buffer + read);
        }
        std::fclose(file);

        pppp_beatmap* from_bytes = 0;
        REQUIRE(pppp_beatmap_from_bytes(&data[0], data.size(), &from_bytes) == PPPP_OK);
        pppp_beatmap* from_file = 0;
        REQUIRE(pppp_beatmap_from_file(path, &from_file) == PPPP_OK);

        pppp_difficulty_attributes a;
        pppp_difficulty_attributes b;
        REQUIRE(pppp_calculate_difficulty(from_bytes, 0, &a) == PPPP_OK);
        REQUIRE(pppp_calculate_difficulty(from_file, 0, &b) == PPPP_OK);
        CHECK(a.star_rating == b.star_rating);
        CHECK(a.osu.aim_difficulty == b.osu.aim_difficulty);
        pppp_beatmap_free(from_bytes);
        pppp_beatmap_free(from_file);

        pppp_beatmap* untouched = 0;
        CHECK(pppp_beatmap_from_bytes(0, 1, &untouched) == PPPP_INVALID_ARGUMENT);
        CHECK(!untouched);
    }

    TEST_CASE("every result has a description") {
        CHECK(std::strcmp(pppp_result_message(PPPP_OK), "no error") == 0);
        CHECK(std::strcmp(pppp_result_message(PPPP_PARSE), "cannot parse the beatmap") == 0);
    }
}

TEST_SUITE("CapiTest") {
    TEST_CASE("the timed attributes match the library's") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        const ModsHandle mods("HD,DT");
        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = mods.handle;

        pppp_timed_difficulty* timed = 0;
        REQUIRE(pppp_calculate_timed_difficulty(map, &options, &timed) == PPPP_OK);
        const pppp_timed_difficulty_attributes* entries = 0;
        size_t count = 0;
        REQUIRE(pppp_timed_difficulty_entries(timed, &entries, &count) == PPPP_OK);

        REQUIRE(count == 601);
        CHECK(entries[300].time == 61309.235290751734);
        CHECK(entries[300].attributes.star_rating == pppp_test::difficulty_approx(8.272126401328721));
        CHECK(entries[300].attributes.max_combo == 433);
        CHECK(entries[600].attributes.ruleset == PPPP_RULESET_OSU);
        CHECK(entries[600].attributes.osu.max_combo == 909);
        CHECK(entries[600].attributes.star_rating == pppp_test::difficulty_approx(8.898179651893287));

        pppp_timed_difficulty_free(timed);
        pppp_beatmap_free(map);
    }

    TEST_CASE("timed arguments are checked") {
        pppp_timed_difficulty* timed = 0;
        CHECK(pppp_calculate_timed_difficulty(0, 0, &timed) == PPPP_INVALID_ARGUMENT);
        CHECK(!timed);
        CHECK(pppp_timed_difficulty_entries(0, 0, 0) == PPPP_INVALID_ARGUMENT);
        pppp_timed_difficulty_free(0);
    }

    TEST_CASE("the strain graph matches the library's") {
        const char* path = PPPP_TEST_RESOURCES "/osu/2785319.osu";
        pppp_beatmap* map = 0;
        REQUIRE(pppp_beatmap_from_file(path, &map) == PPPP_OK);

        const ModsHandle mods("DT");
        pppp_difficulty_options options;
        std::memset(&options, 0, sizeof(options));
        options.mods = mods.handle;

        pppp_strains* strains = 0;
        REQUIRE(pppp_calculate_strains(map, &options, &strains) == PPPP_OK);
        pppp_int32 ruleset = -1;
        double start_time = 0.0;
        double section_length = 0.0;
        REQUIRE(pppp_strains_info(strains, &ruleset, &start_time, &section_length) == PPPP_OK);
        CHECK(ruleset == PPPP_RULESET_OSU);
        CHECK(start_time == 2400.0);
        CHECK(section_length == 600.0);

        pppp_osu_strains osu;
        REQUIRE(pppp_strains_osu(strains, &osu) == PPPP_OK);
        REQUIRE(osu.aim_count == 189);
        CHECK(osu.aim[161] == pppp_test::difficulty_approx(549.3785675344396));
        CHECK(osu.speed[130] == pppp_test::difficulty_approx(264.03121219093333));
        CHECK(osu.reading_count == 189);
        CHECK(!osu.flashlight);
        CHECK(osu.flashlight_count == 0);

        pppp_mania_strains mania;
        CHECK(pppp_strains_mania(strains, &mania) == PPPP_INVALID_ARGUMENT);

        pppp_strains_free(strains);

        options.ruleset = PPPP_RULESET_MANIA;
        options.has_ruleset = 1;
        const ModsHandle keys("4K");
        options.mods = keys.handle;
        REQUIRE(pppp_calculate_strains(map, &options, &strains) == PPPP_OK);
        REQUIRE(pppp_strains_mania(strains, &mania) == PPPP_OK);
        REQUIRE(mania.strain_count == 282);
        CHECK(mania.strain[195] == pppp_test::difficulty_approx(15.725755965531633));
        pppp_strains_free(strains);

        pppp_beatmap_free(map);
    }

    TEST_CASE("strain arguments are checked") {
        pppp_strains* strains = 0;
        CHECK(pppp_calculate_strains(0, 0, &strains) == PPPP_INVALID_ARGUMENT);
        CHECK(!strains);
        CHECK(pppp_strains_info(0, 0, 0, 0) == PPPP_INVALID_ARGUMENT);
        CHECK(pppp_strains_osu(0, 0) == PPPP_INVALID_ARGUMENT);
        pppp_strains_free(0);
    }
}
