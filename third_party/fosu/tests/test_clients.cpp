// Where osu!stable and lazer read a map differently, each client's rule
// (upstream tests/test_clients.cc). The SIMD/scalar engine split is dropped:
// the port has one engine.
#include "test_support.h"

#include <fosu/enums.h>
#include <fosu/slider_path.h>

#include <climits>
#include <vector>

using namespace fosu;

namespace {
    ParseOptions stable_options() {
        ParseOptions options;
        options.client = Client::Stable;
        return options;
    }

    ParseOptions paths_options(Client::Value client) {
        ParseOptions options;
        options.client = client;
        options.calculate_slider_paths = true;
        return options;
    }

    const char* const kTimingAndCircle = "[TimingPoints]\n0,500\n[HitObjects]\n64,64,100,1,0\n";

    // The line that makes `client` refuse the map, or 0 if it loads.
    fosu::u32 unloadable_line(const std::string& input, Client::Value client) {
        Parser parser;
        ParseOptions options;
        options.client = client;
        if (parser.parse(input.data(), input.size(), options)) {
            return 0;
        }
        CHECK(parser.error().code == ParseErrorCode::Unloadable);
        return parser.error().line;
    }
} // namespace

PPPP_TEST(clients_version_line_position) {
    static const char* const starts[] = {" \r\nosu file format v9\r\n", "\nosu file format v9\n",
                                         " osu file format v9\n"};
    for (size_t i = 0; i < 3; i++) {
        const std::string input = std::string(starts[i]) + kTimingAndCircle;
        CHECK(parse_str(input, stable_options()).format_version == 14);
        CHECK(parse_str(input, lazer_options()).format_version == 9);
    }
}

// An unreadable version number fails the map, wherever the client looks for
// it. stable reads Int32.Parse's full range; lazer its symmetric one.
PPPP_TEST(clients_unreadable_version_fails_the_map) {
    static const Client::Value clients[] = {Client::Stable, Client::Lazer};
    for (size_t i = 0; i < 2; i++) {
        CHECK(unloadable_line(std::string("osu file format vx\n") + kTimingAndCircle, clients[i]) == 1u);
        CHECK(unloadable_line(std::string("osu file format v5v\n") + kTimingAndCircle, clients[i]) == 1u);
    }
    const std::string later = std::string("\n\nosu file format v1x\n") + kTimingAndCircle;
    CHECK(unloadable_line(later, Client::Lazer) == 3u);
    CHECK(unloadable_line(later, Client::Stable) == 0u);

    const std::string minimum = std::string("osu file format v-2147483648\n") + kTimingAndCircle;
    CHECK(parse_str(minimum, stable_options()).format_version == INT_MIN);
    CHECK(unloadable_line(minimum, Client::Lazer) == 1u);
}

// stable consumes the first line as its version line even when it is a
// section header. Its header reader then never enters that section, though
// the object reader does.
PPPP_TEST(clients_first_line_header) {
    const std::string general = std::string("[General]\nMode:1\n") + kTimingAndCircle;
    CHECK(parse_str(general, stable_options()).mode == 0);
    CHECK(parse_str(general, lazer_options()).mode == 1);
    const std::string objects = "[HitObjects]\n64,64,100,1,0\n";
    CHECK(parse_str(objects, stable_options()).hit_objects.size() == 1u);
    CHECK(parse_str(objects, lazer_options()).hit_objects.size() == 1u);
}

// Lazer reads lines before the first header as [General]; stable ignores
// them.
PPPP_TEST(clients_lines_before_the_first_header) {
    const std::string input = std::string("osu file format v14\nStackLeniency:0.2\n") + kTimingAndCircle;
    CHECK(parse_str(input, stable_options()).stack_leniency == double(0.7f));
    CHECK(parse_str(input, lazer_options()).stack_leniency == double(0.2f));
}

// A header naming no section starts [General] in lazer and leaves the
// current section in effect in stable. Each also knows names fosu ignores:
// lazer Variables, Fonts, CatchTheBeat and Mania; stable Variables, Unknown
// and All.
PPPP_TEST(clients_unknown_section_names) {
    struct Case {
        const char* header;
        double stable_od;
        double lazer_stack_leniency;
    };
    static const Case cases[] = {
        {"[Future]", 4, 0.2f}, {"[Fonts]", 4, 0.7f},   {"[Variables]", 3, 0.7f},
        {"[Unknown]", 3, 0.2f}, {"[All]", 3, 0.2f},
    };
    for (size_t i = 0; i < 5; i++) {
        const std::string input = std::string("osu file format v14\n[Difficulty]\nOverallDifficulty:3\n") +
                                  cases[i].header +
                                  "\nOverallDifficulty:4\nStackLeniency:0.2\n" + kTimingAndCircle;
        const Beatmap stable = parse_str(input, stable_options());
        CHECK(stable.od == cases[i].stable_od);
        CHECK(stable.stack_leniency == double(0.7f));
        const Beatmap lazer = parse_str(input, lazer_options());
        CHECK(lazer.od == 3);
        CHECK(lazer.stack_leniency == cases[i].lazer_stack_leniency);
    }
    const std::string input = std::string("osu file format v14\n") + kTimingAndCircle + "[Future]\n64,64,200,1,0\n";
    CHECK(parse_str(input, stable_options()).hit_objects.size() == 2u);
    CHECK(parse_str(input, lazer_options()).hit_objects.size() == 1u);
}

// stable reads any line starting with '[' as a header and trims every
// bracket from both ends. fosu follows its object reader in trimming no
// whitespace; its header reader also accepts "[Difficulty] ". Lazer needs
// "[name]" after trimming the line's end.
PPPP_TEST(clients_header_spelling) {
    struct Case {
        const char* header;
        double stable_od;
        double lazer_od;
    };
    static const Case cases[] = {
        {"[Difficulty]", 4, 4}, {"[[Difficulty]]", 4, 5}, {"[Difficulty", 4, 5}, {"[Difficulty] ", 5, 4},
    };
    for (size_t i = 0; i < 4; i++) {
        const std::string input = std::string("osu file format v14\n") + cases[i].header +
                                  "\nOverallDifficulty:4\n" + kTimingAndCircle;
        CHECK(parse_str(input, stable_options()).od == cases[i].stable_od);
        CHECK(parse_str(input, lazer_options()).od == cases[i].lazer_od);
    }
}

// stable's header reader never leaves [HitObjects], so later General,
// Metadata, Difficulty and TimingPoints sections are ignored. Its object
// reader still reads later Events, Colours and HitObjects.
PPPP_TEST(clients_header_sections_after_hit_objects) {
    const std::string input = std::string("osu file format v14\n") + kTimingAndCircle +
                              "[Difficulty]\nOverallDifficulty:4\n"
                              "[TimingPoints]\n50,250\n[Metadata]\nTitle:late\n"
                              "[Events]\n2,300,1000\n[Colours]\nCombo1:1,2,3\n"
                              "[HitObjects]\n64,64,2000,1,0\n";
    const Beatmap stable = parse_str(input, stable_options());
    CHECK(stable.od == 5);
    CHECK(stable.timing_points.size() == 1u);
    CHECK(stable.title.empty());
    CHECK(stable.breaks.size() == 1u);
    CHECK(stable.combo_colours.size() == 1u);
    CHECK(stable.hit_objects.size() == 2u);
    const Beatmap lazer = parse_str(input, lazer_options());
    CHECK(lazer.od == 4);
    CHECK(lazer.timing_points.size() == 2u);
    CHECK(lazer.title == std::string("late"));
    CHECK(lazer.hit_objects.size() == 2u);
}

// stable cannot load a map with a hit object it fails to read; lazer skips
// the line.
PPPP_TEST(clients_unreadable_hit_object_fails_stable) {
    static const char* const lines[] = {"64,64,x,1,0", "64,64,200,1,0,bad", "64,64,200,2,0,L|1:x,1,10",
                                        "64,64,200,8,0,x", "64,64"};
    static const char* const endings[] = {"\n", "\r\n", "\r"};
    static const char* const parts[] = {"osu file format v14", "[TimingPoints]", "0,500", "[HitObjects]",
                                        "64,64,100,1,0"};
    for (size_t l = 0; l < 5; l++) {
        for (size_t e = 0; e < 3; e++) {
            std::string input;
            for (size_t p = 0; p < 5; p++) {
                input += parts[p];
                input += endings[e];
            }
            input += lines[l];
            input += endings[e];
            input += "64,64,300,1,0";
            input += endings[e];
            Parser parser;
            CHECK(!parser.parse(input.data(), input.size(), stable_options()));
            CHECK(parser.error().code == ParseErrorCode::Unloadable);
            CHECK(parser.error().line == 6u);
            const Beatmap& lazer = require_parse(parser.parse(input.data(), input.size(), lazer_options()));
            CHECK(lazer.hit_objects.size() == 2u);
            CHECK(lazer.stats.malformed_lines == 1u);
        }
    }
}

// stable ignores a hit object whose type names no kind, and skips lines
// indented with ' ' or '_' and lines starting with '['. fosu still counts
// the first as malformed. Lazer reads an indented line and rejects the
// others.
PPPP_TEST(clients_hit_object_lines_stable_skips) {
    struct Case {
        const char* line;
        size_t stable_objects, stable_malformed;
        size_t lazer_objects, lazer_malformed;
    };
    static const Case cases[] = {
        {"64,64,200,4,0", 2, 1, 2, 1},
        {" 64,64,200,1,0", 2, 0, 3, 0},
        {"_64,64,200,1,0", 2, 0, 2, 1},
        {"[64,64,200,1,0", 2, 0, 2, 1},
    };
    for (size_t i = 0; i < 4; i++) {
        const std::string input = std::string("osu file format v14\n[TimingPoints]\n0,500\n"
                                              "[HitObjects]\n64,64,100,1,0\n") +
                                  cases[i].line + "\n64,64,300,1,0\n";
        const Beatmap stable = parse_str(input, stable_options());
        CHECK(stable.hit_objects.size() == cases[i].stable_objects);
        CHECK(stable.stats.malformed_lines == cases[i].stable_malformed);
        const Beatmap lazer = parse_str(input, lazer_options());
        CHECK(lazer.hit_objects.size() == cases[i].lazer_objects);
        CHECK(lazer.stats.malformed_lines == cases[i].lazer_malformed);
    }
}

// Both clients read a one-letter curve type other than B, C, L or P as
// Catmull. stable also ignores any other one-character marker; lazer reads
// one as a point, which fails. A slider that ends at its type is one stable
// cannot read: its repeat count is missing.
PPPP_TEST(clients_unknown_curve_types) {
    static const char* const curves[] = {"X|100:100,1,50", "b,1,50"};
    static const Client::Value clients[] = {Client::Stable, Client::Lazer};
    for (size_t c = 0; c < 2; c++) {
        for (size_t o = 0; o < 2; o++) {
            const std::string input = std::string("osu file format v14\n[TimingPoints]\n0,500\n[HitObjects]\n"
                                                  "64,64,100,2,0,") +
                                      curves[c] + "\n64,64,300,1,0\n";
            const Beatmap map = parse_str(input, o == 0 ? stable_options() : lazer_options());
            CHECK(map.hit_objects.size() == 2u);
            CHECK(map.sliders[0].curve_type == CurveType::Catmull);
            CHECK(map.stats.malformed_lines == 0u);
        }
    }
    const std::string digit = std::string("osu file format v14\n[TimingPoints]\n0,500\n[HitObjects]\n"
                                          "64,64,100,2,0,0|100:100,1,50\n64,64,300,1,0\n");
    CHECK(parse_str(digit, stable_options()).sliders[0].curve_type == CurveType::Catmull);
    CHECK(parse_str(digit, lazer_options()).sliders.empty());
    CHECK(unloadable_line(std::string("osu file format v14\n[TimingPoints]\n0,500\n[HitObjects]\n"
                                      "64,64,100,2,0,X\n64,64,300,1,0\n"),
                          Client::Stable) == 5u);
}

// Like storyboard commands, stable skips colour lines indented with ' ' or
// '_'. Lazer trims them.
PPPP_TEST(clients_indented_colours) {
    const std::string input = std::string("osu file format v14\n[Colours]\n Combo1 : 1,2,3\n"
                                          "Combo2:4,5,6\n") +
                              kTimingAndCircle;
    CHECK(parse_str(input, stable_options()).combo_colours.size() == 1u);
    CHECK(parse_str(input, lazer_options()).combo_colours.size() == 2u);
}

// stable reads a one-character token as the curve type of the whole slider,
// the last one winning, and ignores one naming no type; a longer token must
// be a point. Lazer starts a segment at each letter (see test_slider_paths)
// and rejects a one-character token that is not a letter.
PPPP_TEST(clients_curve_type_tokens) {
    const std::string base =
        "osu file format v14\n[TimingPoints]\n0,500\n[HitObjects]\n0,0,1000,2,0,";
    const std::string stable_input = base + "B|100:0|L|100:100|0:100,1,0\n";
    const Beatmap stable = parse_str(stable_input, paths_options(Client::Stable));
    CHECK(stable.sliders[0].curve_type == CurveType::Linear);
    CHECK(stable.slider_points.size() == 3u);
    CHECK(stable.slider_segments.empty());
    CHECK(stable.slider_paths[0].distance() == 300);
    const Beatmap lazer = parse_str(stable_input, lazer_options());
    CHECK(lazer.sliders[0].curve_type == CurveType::Bezier);
    CHECK(lazer.slider_segments.size() == 2u);

    static const char* const stable_curves[] = {"B|100:0|X|100:100", "B|100:0|5|100:100",
                                                "B|100:0|L", "B|L|100:100"};
    for (size_t i = 0; i < 4; i++) {
        const std::string input = base + stable_curves[i] + ",1,0\n";
        CHECK(parse_str(input, stable_options()).sliders.size() == 1u);
    }
    const Beatmap unknown = parse_str(base + "B|100:0|X|100:100,1,0\n", lazer_options());
    CHECK(unknown.slider_segments.size() == 2u);
    CHECK(unknown.slider_segments[1].type == CurveType::Catmull);
    static const char* const lazer_curves[] = {"B|100:0|5|100:100", "B|100:0|L"};
    for (size_t i = 0; i < 2; i++) {
        const Beatmap map = parse_str(base + lazer_curves[i] + ",1,0\n", lazer_options());
        CHECK(map.sliders.empty());
        CHECK(map.stats.malformed_lines == 1u);
    }
    static const char* const unloadable[] = {"B|100:0|LL|100:100", "B|100:0|B2|100:100"};
    for (size_t i = 0; i < 2; i++) {
        CHECK(unloadable_line(base + unloadable[i] + ",1,0\n", Client::Stable) == 5u);
        CHECK(unloadable_line(base + unloadable[i] + ",1,0\n", Client::Lazer) == 0u);
    }
}

// Lazer rejects coordinates and slider lengths beyond +-131072; stable reads
// them. Ranked maps have control points and lengths past that bound.
PPPP_TEST(clients_coordinate_and_length_bounds) {
    const std::string input =
        "osu file format v14\n[TimingPoints]\n0,500\n[HitObjects]\n"
        "200000,64,100,1,0\n"
        "64,64,200,2,0,L|262144:-460440,1,140000\n"
        "64,64,300,2,0,L|100:100,1,-1.79769313486231E+308\n";
    const Beatmap stable = parse_str(input, stable_options());
    CHECK(stable.hit_objects.size() == 3u);
    CHECK(stable.hit_objects[0].x == 512);
    CHECK(stable.slider_points[0].x == 262144);
    CHECK(stable.slider_points[0].y == -460440);
    CHECK(stable.sliders[0].length == 140000);
    CHECK(stable.sliders[1].length == 0);
    const Beatmap lazer = parse_str(input, lazer_options());
    CHECK(lazer.hit_objects.empty());
    CHECK(lazer.stats.malformed_lines == 3u);
}

// stable has no rules past v14, its latest format, so it reads a v128 map
// like a v14 one: it truncates coordinates, reads curve types as markers and
// uses the legacy path rules. The map still reports v128.
PPPP_TEST(clients_v128_maps_in_stable) {
    const std::string stable_input =
        "osu file format v128\n[TimingPoints]\n0,500\n[HitObjects]\n10.75,20.5,100,2,0,B|100.5:0|L|100:100,1,0\n";
    const Beatmap stable = parse_str(stable_input, paths_options(Client::Stable));
    CHECK(stable.format_version == 128);
    CHECK(stable.hit_objects[0].x == 10);
    CHECK(stable.slider_points[0].x == 100);
    CHECK(stable.sliders[0].curve_type == CurveType::Linear);
    CHECK(stable.slider_segments.empty());
    const Beatmap lazer = parse_str(stable_input, lazer_options());
    CHECK(lazer.hit_objects[0].x == 10.75f);
    CHECK(lazer.slider_segments.size() == 2u);

    // A collinear perfect curve is a line under the legacy rules.
    const std::string v128_input =
        "osu file format v128\n[TimingPoints]\n0,500\n[HitObjects]\n10.75,20.5,100,2,0,P|100:20|200:20,1,0\n";
    const std::string v14_input =
        "osu file format v14\n[TimingPoints]\n0,500\n[HitObjects]\n10.75,20.5,100,2,0,P|100:20|200:20,1,0\n";
    const Beatmap v128 = parse_str(v128_input, paths_options(Client::Stable));
    const Beatmap v14 = parse_str(v14_input, paths_options(Client::Stable));
    CHECK(std::vector<PathPoint>(v128.slider_paths[0].points.begin(), v128.slider_paths[0].points.end()) ==
          std::vector<PathPoint>(v14.slider_paths[0].points.begin(), v14.slider_paths[0].points.end()));
    CHECK(unloadable_line(
              "osu file format v128\n[TimingPoints]\n0,500\n[HitObjects]\n10.75,20.5,100,2,0,B2|100:0|100:100,1,0\n",
              Client::Stable) == 5u);
}

// stable clamps stack leniency to [0, 1]; lazer keeps it.
PPPP_TEST(clients_stack_leniency_range) {
    const std::string input =
        "osu file format v14\n[General]\nStackLeniency:3\n" + std::string(kTimingAndCircle);
    CHECK(parse_str(input, stable_options()).stack_leniency == 1);
    CHECK(parse_str(input, lazer_options()).stack_leniency == 3);
}

// Before v13, stable reads HP, CS, OD and AR as integer bytes; lazer reads
// floats in every version.
PPPP_TEST(clients_difficulty_bytes_before_v13) {
    struct Case {
        const char* line;
        double stable, lazer;
    };
    static const Case cases[] = {
        {"OverallDifficulty:+8", 8, 8},
        {"OverallDifficulty:7.5", 5, 7.5},
        {"OverallDifficulty:300", 5, 10},
    };
    static const int versions[] = {12, 13};
    for (size_t c = 0; c < 3; c++) {
        for (size_t v = 0; v < 2; v++) {
            const std::string input = std::string("osu file format v") + to_str(versions[v]) +
                                      "\n[Difficulty]\n" + cases[c].line + "\n" + kTimingAndCircle;
            const double stable = versions[v] == 13 ? cases[c].lazer : cases[c].stable;
            CHECK(parse_str(input, stable_options()).od == stable);
            CHECK(parse_str(input, lazer_options()).od == cases[c].lazer);
        }
    }
}

// stable clamps circle size as it reads it, to mania's key range only when
// [General] already set mania and the map is v13+. Lazer clamps with the
// final mode.
PPPP_TEST(clients_circle_size_range_follows_mode_order) {
    const std::string before_mode =
        "osu file format v14\n[Difficulty]\nCircleSize:14\n[General]\nMode:3\n" + std::string(kTimingAndCircle);
    CHECK(parse_str(before_mode, stable_options()).cs == 10);
    CHECK(parse_str(before_mode, lazer_options()).cs == 14);
    const std::string mode_changes =
        "osu file format v14\n[General]\nMode:3\n[Difficulty]\nCircleSize:14\n[General]\nMode:0\n" +
        std::string(kTimingAndCircle);
    CHECK(parse_str(mode_changes, stable_options()).cs == 14);
    CHECK(parse_str(mode_changes, lazer_options()).cs == 10);
    static const int versions[] = {12, 14};
    for (size_t v = 0; v < 2; v++) {
        const std::string after_mode = std::string("osu file format v") + to_str(versions[v]) +
                                       "\n[General]\nMode:3\n[Difficulty]\nCircleSize:14\n" + kTimingAndCircle;
        CHECK(parse_str(after_mode, stable_options()).cs == (versions[v] == 14 ? 14 : 10));
        CHECK(parse_str(after_mode, lazer_options()).cs == 14);
    }
}
