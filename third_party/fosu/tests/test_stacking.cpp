// osu!standard stacking. Expected values match osu!'s OsuBeatmapProcessor,
// including the pre-v6 algorithm (upstream tests/test_stacking.cc).
#include "test_support.h"

#include <fosu/slider_path.h>

#include <vector>

using namespace fosu;

namespace {
    ParseOptions stacking_options() {
        ParseOptions options = lazer_options();
        options.apply_stacking = true;
        return options;
    }

    std::vector<int> heights(const std::string& input) {
        std::vector<int> result;
        const Beatmap map = parse_str(input, stacking_options());
        for (size_t i = 0; i < map.stacking.size(); i++) {
            result.push_back(map.stacking[i].stack_height);
        }
        return result;
    }

    std::string with_version(int version, const std::string& body) {
        return "osu file format v" + to_str(version) + "\n" + body;
    }

    std::vector<int> v2(int a, int b) {
        std::vector<int> v;
        v.push_back(a);
        v.push_back(b);
        return v;
    }

    std::vector<int> v3(int a, int b, int c) {
        std::vector<int> v;
        v.push_back(a);
        v.push_back(b);
        v.push_back(c);
        return v;
    }

    std::vector<int> v5(int a, int b, int c, int d, int e) {
        std::vector<int> v;
        v.push_back(a);
        v.push_back(b);
        v.push_back(c);
        v.push_back(d);
        v.push_back(e);
        return v;
    }

    // Each level moves 6.4 px times the circle scale (0.5 at circle size 5) and
    // stable's 1.00041 adjustment.
    const float kLevel = 3.2013118267059326f;
} // namespace

PPPP_TEST(stacking_circles_at_one_position_stack_up_and_left) {
    static const int versions[] = {5, 6, 14};
    for (size_t v = 0; v < 3; v++) {
        const std::string input = with_version(
            versions[v], "[HitObjects]\n100,100,1000,1,0\n100,100,1100,1,0\n100,100,1200,1,0\n");
        const Beatmap map = parse_str(input, stacking_options());
        CHECK(map.stacking[0].stack_height == 2);
        CHECK(map.stacking[1].stack_height == 1);
        CHECK(map.stacking[2].stack_height == 0);
        CHECK(map.stacking[1].stack_offset.x == -kLevel);
        CHECK(map.stacking[1].stack_offset.y == -kLevel);
        CHECK(map.stacking[0].stack_offset.x == -2 * kLevel);
        CHECK(map.hit_objects[0].x == 100 - 2 * kLevel);
        const std::pair<float, float> raw = map.hit_objects[0].raw_position(map.stacking[0].stack_offset);
        CHECK(raw.first == 100);
        CHECK(raw.second == 100);
    }
}

PPPP_TEST(stacking_stack_offset_scales_with_circle_size) {
    const std::string input =
        "osu file format v14\n[Difficulty]\nCircleSize:4.2\n"
        "[HitObjects]\n100,100,1000,1,0\n100,100,1100,1,0\n";
    CHECK(parse_str(input, stacking_options()).stacking[0].stack_offset.x == -3.5598587989807129f);
}

PPPP_TEST(stacking_objects_at_a_slider_end_stack_down_and_right) {
    static const int versions[] = {5, 6, 14};
    for (size_t v = 0; v < 3; v++) {
        const std::string input = with_version(
            versions[v],
            "[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n"
            "[HitObjects]\n0,0,1000,2,0,L|100:0,1,100\n100,0,1550,1,0\n"
            "100,0,1600,1,0\n");
        CHECK(heights(input) == v3(0, -1, -2));
        CHECK(parse_str(input, stacking_options()).stacking[1].stack_offset.x == kLevel);
    }
}

// After an even number of slides a slider ends at its head. Before v6, osu!
// still stacks objects at the far end of the path instead.
PPPP_TEST(stacking_old_formats_stack_at_the_path_end_after_repeats) {
    const std::string header =
        "[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n"
        "[HitObjects]\n200,200,3000,2,0,L|300:200,2,100\n";
    CHECK(heights(with_version(5, header + "300,200,4050,1,0\n300,200,4100,1,0\n")) == v3(0, -1, -2));
    CHECK(heights(with_version(14, header + "200,200,4050,1,0\n200,200,4100,1,0\n")) == v3(0, -1, -2));
    // Elsewhere the circles stack only on each other.
    CHECK(heights(with_version(14, header + "300,200,4050,1,0\n300,200,4100,1,0\n")) == v3(0, 1, 0));
}

PPPP_TEST(stacking_needs_close_time_and_position) {
    // At AR 10 objects stack within 450 ms * 0.7 leniency = 315 ms, and within
    // 3 px. Spinners neither stack nor separate others.
    const std::string input =
        "osu file format v14\n[Difficulty]\nApproachRate:10\n[HitObjects]\n"
        "100,100,0,1,0\n100,100,316,1,0\n103,100,400,1,0\n0,0,410,8,0,420\n"
        "103,100,500,1,0\n";
    CHECK(heights(input) == v5(0, 0, 1, 0, 0));
}

PPPP_TEST(stacking_zero_leniency_still_stacks_at_a_slider_end) {
    const std::string input =
        "osu file format v14\n[General]\nStackLeniency:0\n"
        "[Difficulty]\nCircleSize:4\nApproachRate:9.5\nSliderMultiplier:2.1\n"
        "[TimingPoints]\n0,375\n0,-55.5555555555556,4,2,0,100,0,0\n"
        "[HitObjects]\n"
        "275,205,0,6,0,B|192:304|44:252|44:252|156:192|128:112,1,"
        "377.999988464356\n130,166,375,5,2\n";
    CHECK(heights(input) == v2(0, -1));
}

// Stacking uses osu!standard's slider end times, which round the velocity
// multiplier through single precision.
PPPP_TEST(stacking_uses_osu_standard_slider_end_times) {
    const std::string input =
        "osu file format v14\n[General]\nStackLeniency:0.5\n"
        "[Difficulty]\nCircleSize:3.8\nApproachRate:9\nSliderMultiplier:1.7\n"
        "[TimingPoints]\n390,300\n143790,-142.857142857143,4,2,1,50,0,0\n"
        "153990,-111.111111111111,4,2,1,65,0,0\n[HitObjects]\n"
        "406,228,146790,6,0,L|374:371,2,118.999996368408\n406,228,147690,1,2\n"
        "73,166,155190,6,0,L|104:6,1,152.999995330811\n102,15,155790,1,0\n"
        "102,15,155940,1,0\n";
    CHECK(heights(input) == v5(0, -1, 0, -1, -2));
}

PPPP_TEST(stacking_stacked_slider_moves_its_points_but_not_its_path) {
    const std::string input =
        "osu file format v14\n[Difficulty]\nSliderMultiplier:1\n"
        "[TimingPoints]\n0,500\n[HitObjects]\n"
        "100,100,1000,2,0,L|200:100,2,100\n100,100,1100,2,0,L|200:100,1,100\n";
    ParseOptions paths = lazer_options();
    paths.calculate_slider_paths = true;
    std::vector<PathPoint> unstacked;
    const Beatmap without = parse_str(input, paths);
    for (size_t i = 0; i < without.slider_paths[0].points.size(); i++) {
        unstacked.push_back(without.slider_paths[0].points[i]);
    }
    ParseOptions stacked_options = paths;
    stacked_options.apply_stacking = true;
    const Beatmap map = parse_str(input, stacked_options);
    CHECK(map.stacking[0].stack_height == 1);
    const nonstd::span<const SliderPoint> points =
        map.slider_points.subspan(map.sliders[0].point_begin, map.sliders[0].point_count);
    CHECK(points[0].x == 200 - kLevel);
    CHECK(points[0].y == 100 - kLevel);
    const nonstd::span<PathPoint> path = map.slider_paths[0].points;
    CHECK(std::vector<PathPoint>(path.begin(), path.end()) == unstacked);
}

PPPP_TEST(stacking_applies_only_to_osu_standard) {
    for (int mode = 1; mode <= 3; mode++) {
        const std::string input =
            "osu file format v14\n[General]\nMode:" + to_str(mode) +
            "\n[HitObjects]\n100,100,0,1,0\n100,100,1,1,0\n";
        const Beatmap map = parse_str(input, stacking_options());
        CHECK(map.stacking.empty());
        CHECK(map.hit_objects[0].x == 100);
    }
}

// An object stacks as the kind the parser chose (circle, slider, spinner,
// hold), whatever other kind bits its type also sets.
PPPP_TEST(stacking_objects_with_several_kind_bits_stack_as_their_kind) {
    const std::string header =
        "osu file format v14\n[Difficulty]\nSliderMultiplier:1\n"
        "[TimingPoints]\n0,500\n[HitObjects]\n";
    // 3 = circle and slider bits, 9 = circle and spinner.
    CHECK(heights(header + "100,100,1000,3,0\n100,100,1100,1,0\n") == v2(1, 0));
    CHECK(heights(header + "100,100,1000,1,0\n100,100,1100,9,0\n") == v2(1, 0));
    // 10 = slider and spinner, 130 = slider and hold.
    CHECK(heights(header + "100,100,1000,1,0\n100,100,1100,10,0,L|200:100,1,100\n") == v2(1, 0));
    CHECK(heights(header + "100,100,1000,10,0,L|200:100,1,100\n100,100,1100,1,0\n") == v2(1, 0));
    CHECK(heights(header + "100,100,1000,1,0\n100,100,1100,130,0,L|200:100,1,100\n") == v2(1, 0));
}
