// Mods applied while parsing: Easy and Hard Rock change difficulty settings
// and positions; Double Time, Nightcore and Half Time change the clock rate
// (upstream tests/test_mods.cc).
#include "test_support.h"

#include <fosu/mods.h>

using namespace fosu;

namespace {
    ParseOptions with_mods(fosu::u32 mods) {
        ParseOptions options = lazer_options();
        options.mods = mods;
        return options;
    }
} // namespace

TEST_CASE("hard rock raises difficulty and flips vertically") {
    const std::string input = "osu file format v14\n[General]\nMode:0\n"
                              "[Difficulty]\nHPDrainRate:4\nCircleSize:4\nOverallDifficulty:4\n"
                              "ApproachRate:4\nSliderMultiplier:1\n[TimingPoints]\n0,500\n"
                              "[HitObjects]\n100,100,1000,2,0,L|200:150,1,100\n";
    const Beatmap map = parse_str(input, with_mods(Mods::HardRock));
    CHECK(map.hp == 4 * 1.4);
    CHECK(map.cs == 4 * 1.3);
    CHECK(map.od == 4 * 1.4);
    CHECK(map.ar == 4 * 1.4);
    CHECK(map.hit_objects[0].y == 384 - 100);
    CHECK(map.slider_points[0].y == 384 - 150);
}

TEST_CASE("hard rock caps difficulty at 10") {
    const std::string input = "osu file format "
                              "v14\n[General]\nMode:0\n[Difficulty]\nHPDrainRate:8\nCircleSize:8\n"
                              "OverallDifficulty:8\nApproachRate:8\n";
    const Beatmap map = parse_str(input, with_mods(Mods::HardRock));
    CHECK(map.hp == 10);
    CHECK(map.cs == 10);
    CHECK(map.od == 10);
    CHECK(map.ar == 10);
}

TEST_CASE("easy halves difficulty") {
    const std::string input = "osu file format "
                              "v14\n[General]\nMode:0\n[Difficulty]\nHPDrainRate:4\nCircleSize:4\n"
                              "OverallDifficulty:4\nApproachRate:4\nSliderMultiplier:1\n";
    const Beatmap map = parse_str(input, with_mods(Mods::Easy));
    CHECK(map.hp == 2);
    CHECK(map.cs == 2);
    CHECK(map.od == 2);
    CHECK(map.ar == 2);
    CHECK(map.slider_multiplier == 1);
}

// osu!taiko has no circle size or approach rate to change, and scales
// slider velocity instead of flipping positions.
TEST_CASE("taiko difficulty mods follow taiko rules") {
    const std::string input = "osu file format "
                              "v14\n[General]\nMode:1\n[Difficulty]\nHPDrainRate:4\nCircleSize:4\n"
                              "OverallDifficulty:4\nApproachRate:4\nSliderMultiplier:1\n"
                              "[HitObjects]\n100,100,1000,1,0\n";
    const Beatmap easy = parse_str(input, with_mods(Mods::Easy));
    CHECK(easy.hp == 2);
    CHECK(easy.od == 2);
    CHECK_UNARY(easy.slider_multiplier == 0.8);
    const Beatmap hard_rock = parse_str(input, with_mods(Mods::HardRock));
    CHECK(hard_rock.hp == 4 * 1.4);
    CHECK(hard_rock.cs == 4);
    CHECK(hard_rock.od == 4 * 1.4);
    CHECK(hard_rock.ar == 4);
    CHECK(hard_rock.slider_multiplier == 1.4 * 4 / 3);
    CHECK(hard_rock.hit_objects[0].y == 100);
}

TEST_CASE("catch and mania reject difficulty mods") {
    static const char* const modes[] = {"2", "3"};
    for (size_t i = 0; i < 2; i++) {
        const std::string input = std::string("osu file format v14\n[General]\nMode:") + modes[i] +
                                  "\n[Difficulty]\nCircleSize:4\n";
        Parser parser;
        CHECK(!parser.parse(input.data(), input.size(), with_mods(Mods::Easy)));
        CHECK(!parser.parse(input.data(), input.size(), with_mods(Mods::HardRock)));
    }
}

// Times divide by the clock rate; beat lengths of uninherited points too,
// while inherited multipliers stay.
TEST_CASE("rate mods scale every time") {
    struct RateCase {
        fosu::u32 mods;
        double rate;
    };
    static const RateCase rate_cases[] = {
        {Mods::DoubleTime, 1.5}, {Mods::Nightcore, 1.5}, {Mods::HalfTime, 0.75}};
    for (size_t r = 0; r < 3; r++) {
        for (int mode = 0; mode < 4; mode++) {
            const char* object = mode == 3 ? "0,0,900,128,0,1500:0:0:0:0:" : "0,0,900,8,0,1500,0:0:0:0:";
            const std::string input = "osu file format v14\n[General]\nMode:" + to_str(mode) +
                                      "\n[Events]\n2,1500,2000\n[TimingPoints]\n300,600\n"
                                      "600,-50,4,0,0,100,0,0\n[HitObjects]\n" +
                                      object + "\n";
            const Beatmap map = parse_str(input, with_mods(rate_cases[r].mods));
            CHECK(map.hit_objects[0].time == 900 / rate_cases[r].rate);
            CHECK(map.hit_objects[0].end_time == 1500 / rate_cases[r].rate);
            CHECK(map.timing_points[0].time == 300 / rate_cases[r].rate);
            CHECK(map.timing_points[1].time == 600 / rate_cases[r].rate);
            CHECK(map.timing_points[0].beat_length == 600 / rate_cases[r].rate);
            CHECK(map.timing_points[1].beat_length == -50);
            CHECK(map.breaks[0].start == 1500 / rate_cases[r].rate);
            CHECK(map.breaks[0].end == 2000 / rate_cases[r].rate);
        }
    }
}

// Calculated values come from the unscaled map, then scale with it.
TEST_CASE("rate mods scale calculated slider values") {
    const std::string input = "osu file format v14\n[Difficulty]\nSliderMultiplier:1\n"
                              "[TimingPoints]\n0,500\n[HitObjects]\n100,100,1000,2,0,L|200:150,1,100\n";
    ParseOptions options = lazer_options();
    options.calculate_slider_events = true;
    options.mods = static_cast<fosu::u32>(Mods::HardRock) | static_cast<fosu::u32>(Mods::DoubleTime);
    const Beatmap map = parse_str(input, options);
    CHECK(map.hit_objects[0].end_time == 1500 / 1.5);
    const nonstd::span<SliderEvent> events = map.slider_events[0];
    CHECK(events.size() == 3u);
    CHECK(events[0].time == 1000 / 1.5);
    CHECK(events[1].time == 1464 / 1.5);
    CHECK(events[2].time == 1500 / 1.5);
    for (size_t i = 0; i < events.size(); i++) {
        CHECK(events[i].span_start_time == 1000 / 1.5);
    }
}

TEST_CASE("invalid mod options fail the parse") {
    const std::string input = "osu file format "
                              "v14\n[General]\nMode:0\n[Difficulty]\nCircleSize:4\n[HitObjects]\n0,0,0,"
                              "1,"
                              "0\n";
    Parser parser;
    static const fosu::u32 invalid[] = {
        static_cast<fosu::u32>(Mods::Easy) | static_cast<fosu::u32>(Mods::HardRock),
        static_cast<fosu::u32>(Mods::DoubleTime) | static_cast<fosu::u32>(Mods::HalfTime),
        static_cast<fosu::u32>(Mods::Nightcore) | static_cast<fosu::u32>(Mods::HalfTime), 1u << 30};
    for (size_t i = 0; i < 4; i++) {
        CHECK(!parser.parse(input.data(), input.size(), with_mods(invalid[i])));
    }
    // Difficulty mods need the General and Difficulty sections they adjust.
    ParseOptions hit_objects_only = with_mods(Mods::HardRock);
    hit_objects_only.sections = kSectionHitObjects;
    CHECK(!parser.parse(input.data(), input.size(), hit_objects_only));
    // A failed parse leaves the parser usable.
    CHECK(parser.parse(input.data(), input.size(), with_mods(Mods::HardRock)));
}
