// Slider end times and events. Expected values match osu!'s legacy decoder
// (ConvertHitObjectParser, LegacyBeatmapDecoder) and SliderEventGenerator
// (upstream tests/test_slider_timing.cc).
#include "test_support.h"

#include <fosu/slider_event.h>
#include <fosu/slider_path.h>

#include <cmath>
#include <vector>

using namespace fosu;

namespace {
    // SliderMultiplier 1 at a 500 ms beat moves sliders at 0.2 px/ms, so each
    // 100 px span takes 500 ms.
    std::string beatmap(const std::string& timing_points, const std::string& hit_objects) {
        return "osu file format v14\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n" + timing_points +
               "[HitObjects]\n" + hit_objects;
    }

    double end_time(const std::string& input, size_t object = 0) {
        ParseOptions options = lazer_options();
        options.calculate_slider_end_times = true;
        return parse_str(input, options).hit_objects[object].end_time;
    }

    struct TimedEvent {
        SliderEventType::Value type;
        double time;

        bool operator==(const TimedEvent& other) const { return type == other.type && time == other.time; }
    };

    typedef std::vector<TimedEvent> Events;

    TimedEvent make_event(SliderEventType::Value type, double time) {
        TimedEvent event;
        event.type = type;
        event.time = time;
        return event;
    }

    Events timed_events(const std::string& input) {
        ParseOptions options = lazer_options();
        options.calculate_slider_events = true;
        const Beatmap map = parse_str(input, options);
        Events events;
        const nonstd::span<SliderEvent> span = map.slider_events[map.hit_objects[0].slider];
        for (size_t i = 0; i < span.size(); i++) {
            events.push_back(make_event(span[i].type, span[i].time));
        }
        return events;
    }

    Events events3(SliderEventType::Value t0, double x0, SliderEventType::Value t1, double x1,
                   SliderEventType::Value t2, double x2) {
        Events v;
        v.push_back(make_event(t0, x0));
        v.push_back(make_event(t1, x1));
        v.push_back(make_event(t2, x2));
        return v;
    }

    Events events6(SliderEventType::Value t0, double x0, SliderEventType::Value t1, double x1,
                   SliderEventType::Value t2, double x2, SliderEventType::Value t3, double x3,
                   SliderEventType::Value t4, double x4, SliderEventType::Value t5, double x5) {
        Events v;
        v.push_back(make_event(t0, x0));
        v.push_back(make_event(t1, x1));
        v.push_back(make_event(t2, x2));
        v.push_back(make_event(t3, x3));
        v.push_back(make_event(t4, x4));
        v.push_back(make_event(t5, x5));
        return v;
    }

    bool close_to(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance; }
} // namespace

TEST_CASE("the end time covers every span") {
    const std::string input = beatmap("0,500\n", "0,0,1000,2,0,L|100:0,2,100\n");
    CHECK(end_time(input) == 2000);
}

TEST_CASE("an omitted or nonpositive length uses the path distance") {
    static const char* const tails[] = {"", ",0", ",-5"};
    for (size_t i = 0; i < 3; i++) {
        const std::string input = beatmap("0,500\n", std::string("0,0,1000,2,0,L|200:0,1") + tails[i] + "\n");
        CHECK(end_time(input) == 2000);
    }
}

TEST_CASE("a slide count below one is one slide") {
    const std::string input = beatmap("0,500\n", "0,0,1000,2,0,L|100:0,0,100\n");
    CHECK(end_time(input) == 1500);
    CHECK(timed_events(input) == events3(SliderEventType::Head, 1000, SliderEventType::LegacyLastTick, 1464,
                                         SliderEventType::Tail, 1500));
}

// The generic decoder's end time; rulesets convert sliders afterwards.
TEST_CASE("the end time does not depend on the game mode") {
    static const char* const modes[] = {"0", "1", "2"};
    for (size_t i = 0; i < 3; i++) {
        const std::string input = std::string("osu file format v14\n[General]\nMode:") + modes[i] +
                                  "\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n"
                                  "0,500\n[HitObjects]\n0,0,1000,2,0,L|100:0,2,100\n";
        CHECK(end_time(input) == 2000);
    }
}

TEST_CASE("an inherited point scales velocity") {
    const std::string input = beatmap("0,500\n0,-50,4,1,0,100,0,0\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1250);
}

TEST_CASE("the velocity multiplier is clamped from a tenth to tenfold") {
    // -1 asks for 100x and -10000 for 0.01x.
    const std::string fast = beatmap("0,500\n0,-1,4,1,0,100,0,0\n", "0,0,1000,2,0,L|100:0,1,100\n");
    const std::string slow = beatmap("0,500\n0,-10000,4,1,0,100,0,0\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(fast) == 1050);
    CHECK(end_time(slow) == 6000);
}

TEST_CASE("a NaN inherited point keeps the default velocity") {
    const std::string input = beatmap("0,500\n0,NaN,4,1,0,100,0,0\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1500);
}

TEST_CASE("the beat length is clamped from 6 to 60000 ms") {
    const std::string short_beat = beatmap("0,1\n", "0,0,1000,2,0,L|100:0,1,100\n");
    const std::string long_beat = beatmap("0,100000\n", "0,0,1000,2,0,L|100:0,1,1\n");
    CHECK(end_time(short_beat) == 1006);
    CHECK(end_time(long_beat) == 1600);
}

TEST_CASE("an uninherited point resets velocity") {
    const std::string input =
        beatmap("0,500\n0,-50,4,1,0,100,0,0\n1000,500\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1500);
}

// Consecutive lines at one time form a group. Within it, the first
// uninherited line sets the beat length and the last inherited line sets the
// velocity, overriding the uninherited lines' 1x.
TEST_CASE("the first uninherited point in a group wins") {
    const std::string input = beatmap("0,500\n0,250\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1500);
}

TEST_CASE("the last inherited point in a group wins") {
    const std::string input =
        beatmap("0,500\n0,-50,4,1,0,100,0,0\n0,-25,4,1,0,100,0,0\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1125);
}

TEST_CASE("an inherited point overrides a reset in its group") {
    const std::string inherited_first =
        beatmap("0,-50,4,1,0,100,0,0\n0,500\n", "0,0,1000,2,0,L|100:0,1,100\n");
    const std::string inherited_last =
        beatmap("0,500\n1000,-50,4,1,0,100,0,0\n1000,500\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(inherited_first) == 1250);
    CHECK(end_time(inherited_last) == 1250);
}

TEST_CASE("points apply in time order, not file order") {
    const std::string input =
        beatmap("0,500\n2000,-50,4,1,0,100,0,0\n1000,-25,4,1,0,100,0,0\n", "0,0,1500,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1625);
}

// osu! drops an inherited point that matches the velocity in effect when it
// is read. Here the 2x at 1000 matches the 2x at 0, so the 4x line read later
// for 500 still applies at 1000.
TEST_CASE("an unchanged inherited point is dropped when read") {
    const std::string input = beatmap("0,500\n0,-50,4,1,0,100,0,0\n1000,-50,4,1,0,100,0,0\n"
                                      "500,-25,4,1,0,100,0,0\n",
                                      "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 1125);
}

// Unlike a repeat within one group, a separate later group replaces it.
TEST_CASE("a later group at the same time replaces the earlier one") {
    const std::string input =
        beatmap("0,500\n1000,-50,4,1,0,100,0,0\n0,250\n", "0,0,500,2,0,L|100:0,1,100\n");
    CHECK(end_time(input) == 750);
}

// The first uninherited point's beat length applies before it; inherited
// points never apply before their time.
TEST_CASE("objects before the first point use its beat length") {
    const std::string uninherited = beatmap("1000,250\n", "0,0,500,2,0,L|100:0,1,100\n");
    const std::string inherited =
        beatmap("1000,250\n1000,-50,4,1,0,100,0,0\n", "0,0,500,2,0,L|100:0,1,100\n");
    CHECK(end_time(uninherited) == 750);
    CHECK(end_time(inherited) == 750);
}

TEST_CASE("without timing points sliders use 60 BPM") {
    // The default slider multiplier is 1.4 and the default beat 1000 ms.
    const std::string input = "osu file format v14\n[Difficulty]\nSliderMultiplier:1\n"
                              "[TimingPoints]\n0,500\n[HitObjects]\n0,0,0,2,0,L|100:0,1,140\n";
    ParseOptions options = lazer_options();
    options.sections = kSectionHitObjects;
    options.calculate_slider_end_times = true;
    const Beatmap map = parse_str(input, options);
    CHECK(map.hit_objects[0].end_time == 140 / (100 * 1.4 / 1000));
}

// lazer removes the repeats of a path no longer than 1e-7 px. Stable creates
// them but never judges them.
TEST_CASE("a zero length path drops repeats") {
    static const char* const sliders[] = {"B|0:0,3,200",     "L|0:0,2,100", "C|0:0,2,100",
                                          "P|0:0|0:0,2,100", "B,3072,-1",   "L,1,100"};
    for (size_t i = 0; i < 6; i++) {
        const std::string input = beatmap("0,500\n", std::string("0,0,1000,2,0,") + sliders[i] + "\n");
        CHECK(end_time(input) == 1000);
        CHECK(timed_events(input) == events3(SliderEventType::Head, 1000, SliderEventType::LegacyLastTick,
                                             1000, SliderEventType::Tail, 1000));
    }
    // The slider still reports the slide count as written.
    const Beatmap map = parse_str(beatmap("0,500\n", "0,0,1000,2,0,B,3072,-1\n"));
    CHECK(map.sliders[0].slides == 3072);
}

TEST_CASE("repeats need a path longer than 1e-7 px") {
    const std::string short_len = beatmap("0,500\n", "0,0,1000,2,0,L|100:0,5,0.00000001\n");
    const std::string long_len = beatmap("0,500\n", "0,0,1000,2,0,L|100:0,5,0.000001\n");
    size_t short_repeats = 0;
    const Events short_events = timed_events(short_len);
    for (size_t i = 0; i < short_events.size(); i++) {
        short_repeats += short_events[i].type == SliderEventType::Repeat;
    }
    size_t long_repeats = 0;
    const Events long_events = timed_events(long_len);
    for (size_t i = 0; i < long_events.size(); i++) {
        long_repeats += long_events[i].type == SliderEventType::Repeat;
    }
    CHECK_UNARY(short_repeats == 0u);
    CHECK(long_repeats == 4u);
}

// Stacking needs end times, so it calculates them; paths alone do not.
TEST_CASE("end times are calculated only when needed") {
    const std::string input = beatmap("0,500\n", "0,0,1000,2,0,L|100:0,1,100\n");
    CHECK_UNARY(parse_str(input, lazer_options()).hit_objects[0].end_time == 0);
    ParseOptions paths = lazer_options();
    paths.calculate_slider_paths = true;
    CHECK_UNARY(parse_str(input, paths).hit_objects[0].end_time == 0);
    ParseOptions stacked = lazer_options();
    stacked.apply_stacking = true;
    const Beatmap map = parse_str(input, stacked);
    CHECK(map.hit_objects[0].end_time == 1500);
    CHECK(map.slider_events.empty());
}

TEST_CASE("the end time is independent of the requested outputs") {
    static const char* const inputs[] = {"osu file format "
                                         "v14\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n0,-"
                                         "50,4,1,0,100,0,0\n[HitObjects]\n0,0,1000,2,0,L|100:0,2,100\n",
                                         "osu file format "
                                         "v14\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n["
                                         "HitObjects]\n0,0,1000,2,0,B|100:100|200:0|200:0,1,500\n",
                                         "osu file format "
                                         "v14\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n["
                                         "HitObjects]\n0,0,1000,2,0,P|100:100|200:0,1,0\n",
                                         "osu file format "
                                         "v14\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n["
                                         "HitObjects]\n0,0,1000,2,0,C|100:100|200:0,1,0\n",
                                         "osu file format "
                                         "v14\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n["
                                         "HitObjects]\n0,0,1000,2,0,B,3072,-1\n",
                                         "osu file format v128\n[Difficulty]\nSliderMultiplier:1.4\n"
                                         "[TimingPoints]\n0,500\n[HitObjects]\n"
                                         "0,0,1000,2,0,B2|100:0|100:100|100:100,1,300\n"
                                         "0,0,3000,2,0,B2|100:0|100:100|100:100,1,0\n"
                                         "0,0,5000,2,0,B2|100:0|100:100|100:100,1\n"};
    for (size_t i = 0; i < 6; i++) {
        const std::string input = inputs[i];
        ParseOptions expected_options = lazer_options();
        expected_options.calculate_slider_end_times = true;
        const Beatmap expected_map = parse_str(input, expected_options);
        std::vector<double> expected;
        for (size_t j = 0; j < expected_map.hit_objects.size(); j++) {
            expected.push_back(expected_map.hit_objects[j].end_time);
        }

        ParseOptions variant1 = lazer_options();
        variant1.calculate_slider_end_times = true;
        variant1.calculate_slider_paths = true;
        ParseOptions variant2 = lazer_options();
        variant2.calculate_slider_events = true;
        ParseOptions variant3 = lazer_options();
        variant3.calculate_slider_events = true;
        variant3.apply_stacking = true;
        ParseOptions variants[3] = {variant1, variant2, variant3};

        for (size_t v = 0; v < 3; v++) {
            const Beatmap map = parse_str(input, variants[v]);
            for (size_t j = 0; j < expected.size(); j++) {
                CHECK(map.hit_objects[j].end_time == expected[j]);
            }
        }
    }
}

TEST_CASE("events cover head, ticks, repeats and tail") {
    const std::string input = "osu file format v8\n[Difficulty]\nSliderMultiplier:1\n"
                              "[TimingPoints]\n0,500\n0,-50,4,0,0,100,0,0\n"
                              "[HitObjects]\n0,0,1000,2,0,L|400:0,2,400\n";
    CHECK(timed_events(input) == events6(SliderEventType::Head, 1000, SliderEventType::Tick, 1500,
                                         SliderEventType::Repeat, 2000, SliderEventType::Tick, 2500,
                                         SliderEventType::LegacyLastTick, 2964, SliderEventType::Tail, 3000));
    ParseOptions options = lazer_options();
    options.calculate_slider_events = true;
    const Beatmap map = parse_str(input, options);
    const nonstd::span<SliderEvent> events = map.slider_events[0];
    CHECK(events[2].position == PathPoint::make(400, 0));
    CHECK(events[5].position == PathPoint::make(0, 0));
    // 36 ms before the end, on the way back: 3.6% of the path from the head.
    CHECK(close_to(events[4].path_progress, 0.036, 1e-9));
    CHECK(close_to(events[4].position.x, 14.4f, 1e-4f));
    for (size_t i = 0; i < events.size(); i++) {
        const PathPoint position = slider_position_at(map.slider_paths[0], events[i].path_progress);
        CHECK(close_to(events[i].position.x, position.x, 1e-4f));
        CHECK(close_to(events[i].position.y, position.y, 1e-4f));
    }
}

// Before v8, tick spacing ignores the inherited velocity multiplier.
TEST_CASE("ticks before v8 ignore the velocity multiplier") {
    const std::string input = "osu file format v7\n[Difficulty]\nSliderMultiplier:1\n"
                              "[TimingPoints]\n0,500\n0,-50,4,0,0,100,0,0\n"
                              "[HitObjects]\n0,0,1000,2,0,L|400:0,2,400\n";
    std::vector<double> ticks;
    const Events events = timed_events(input);
    for (size_t i = 0; i < events.size(); i++) {
        if (events[i].type == SliderEventType::Tick) {
            ticks.push_back(events[i].time);
        }
    }
    static const double expected[] = {1250, 1500, 1750, 2250, 2500, 2750};
    CHECK(ticks == std::vector<double>(expected, expected + 6));
}

TEST_CASE("the legacy last tick is at least halfway") {
    const std::string input = beatmap("0,500\n", "0,0,1000,2,0,L|10:0,1,10\n");
    ParseOptions options = lazer_options();
    options.calculate_slider_events = true;
    const Beatmap map = parse_str(input, options);
    const SliderEvent& legacy = map.slider_events[0][1];
    CHECK(legacy.type == SliderEventType::LegacyLastTick);
    CHECK(legacy.time == 1025);
    CHECK_UNARY(legacy.path_progress == 0.5);
    CHECK(legacy.position.x == 5);
}

TEST_CASE("a NaN inherited point suppresses ticks") {
    const std::string input = beatmap("0,500\n0,NaN,4,0,0,100,0,0\n", "0,0,0,2,0,L|300:0,1,300\n");
    const std::string zero_length = beatmap("0,500\n0,NaN,4,0,0,100,0,0\n", "0,0,0,2,0,B|0:0,1,0\n");
    CHECK(timed_events(input) == events3(SliderEventType::Head, 0, SliderEventType::LegacyLastTick, 1464,
                                         SliderEventType::Tail, 1500));
    ParseOptions options = lazer_options();
    options.calculate_slider_events = true;
    const Beatmap map = parse_str(zero_length, options);
    CHECK(map.slider_events[0].size() == 3u);
    for (size_t i = 0; i < map.slider_events[0].size(); i++) {
        CHECK_UNARY(std::isfinite(map.slider_events[0][i].time) &&
                    std::isfinite(map.slider_events[0][i].path_progress));
    }
}
