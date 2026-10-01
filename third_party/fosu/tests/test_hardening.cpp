// Numeric boundaries, fractional times, NaN, framing and truncated inputs (upstream
// tests/test_hardening.cc; its scalar-vs-SIMD comparisons become "parses without failing").
#include "test_support.h"
#include <cstdlib>
#include <cstring>
#include <fosu/engine/hit_objects/samples.h>

using namespace fosu;

static Beatmap must_parse(Parser& parser, const FileBuffer& input,
                          const ParseOptions& options = ParseOptions()) {
    return require_parse(parser.parse(input, options));
}

// Every input, however truncated or malformed, parses without failing.
static void check(const std::string& text) {
    FileBuffer input;
    CHECK(make_padded(StringView(text.data(), text.size()), input));
    Parser parser;
    CHECK(parser.parse(input).ok());
}

PPPP_TEST(short_sample_shape) {
    // The short sample shortcut must reject every non-digit/separator byte, including
    // high-bit bytes. Full parsing may accept other spellings via its bounded fallback.
    bool shapes_ok = true;
    for (size_t pos = 0; pos < 8; ++pos) {
        for (unsigned byte = 0; byte < 256; ++byte) {
            std::string sample = "0:1:2:3:";
            sample[pos] = static_cast<char>(byte);
            FileBuffer padded;
            CHECK(make_padded(StringView(sample.data(), sample.size()), padded));
            const bool exact_shape = pos % 2 ? byte == ':' : byte >= '0' && byte <= '9';
            shapes_ok = shapes_ok && internal::short_sample(padded.data) == exact_shape;
            check("[HitObjects]\n1,2,3,1,0," + sample);
        }
    }
    CHECK(shapes_ok);
    // Circle precedence applies even when slider/spinner/hold bits are set.
    static const int types[] = {1, 3, 9, 129, 255};
    static const char* const endings[] = {"", "\n", "\r\n"};
    for (size_t t = 0; t < 5; t++) {
        const std::string line = "[HitObjects]\n1,2,3," + to_str(types[t]) + ",0,0:1:2:3:";
        for (size_t e = 0; e < 3; e++) {
            check(line + endings[e]);
            const Beatmap map = parse_str(line + endings[e]);
            CHECK(map.hit_objects.size() == 1u && map.hit_objects[0].is_circle());
            CHECK(map.hit_objects[0].hit_sample == StringView("0:1:2:3:"));
        }
    }
}

PPPP_TEST(framing_and_combo_state) {
    Parser parser;
    const Result<Beatmap*> empty = parser.parse(0, 0);
    CHECK(empty.ok() && empty.value()->hit_objects.empty());
    FileBuffer empty_input;
    CHECK(make_padded(StringView(), empty_input));
    CHECK(must_parse(parser, empty_input).hit_objects.empty());
    FileBuffer embedded;
    CHECK(make_padded(StringView("[Metadata]\nTitle:[HitObjects]\n1,2,3,1,0\n[HitObjects]\n1,2,4,1,0\n"),
                      embedded));
    ParseOptions hit_objects_only;
    hit_objects_only.sections = kSectionHitObjects;
    const Beatmap selected = must_parse(parser, embedded, hit_objects_only);
    CHECK(selected.hit_objects.size() == 1 && selected.hit_objects[0].time == 4);
    // Only accepted objects carry combo state across malformed lines and
    // repeated HitObjects sections, including after a spinner.
    {
        FileBuffer input;
        CHECK(make_padded(StringView("[HitObjects]\n256,192,100,8,0,150\n0,0,160,8,0,bad\n"
                                     "[Metadata]\nTitle:gap\n[HitObjects]\n"
                                     "100,100,200,1,0\n101,100,201,1,0\n"
                                     "1.5,2,250,8,0,300\n1,2,310,1,0\n"),
                          input));
        const Beatmap map = must_parse(parser, input);
        CHECK(map.stats.malformed_lines == 1);
        CHECK(map.hit_objects.size() == 5);
        CHECK(map.hit_objects[1].new_combo);
        CHECK(!map.hit_objects[2].new_combo);
        CHECK(map.hit_objects[4].new_combo);
    }
    FileBuffer point_input;
    CHECK(make_padded(StringView("[HitObjects]\n1,2,3,2,0,B|1:2.5|3:4e1,1,10\n"), point_input));
    const Beatmap points = must_parse(parser, point_input);
    CHECK(points.sliders.size() == 1 && points.slider_points.size() == 2);
    CHECK(points.slider_points[0].y == 2 && points.slider_points[1].y == 40);
    check("[TimingPoints]\n\r\r// comment\n0,500\n[HitObjects]\n\r\r// comment\n1,2,3,1,0");
    check("[Events]\n\n \n\t// comment\n");
    check("[HitObjects]\n0,2,3,3,0\r,");
    check("[HitObjects]\n1,2,3,2,0,B|1:2\v|3:4,1,10");
}

PPPP_TEST(slider_tails) {
    static const char* const tails[] = {
        "B|1:2,1,10",
        "B|1:2,1",
        "B|1:2,12,142.499996185303,2|0,0:0|0:0,0:0:0:0:",
        "P|1:2|3:4,1,100,0|0,0:0|0:0,0:0:0:0:a,b",
        "L|1:2,1,100,0|0,0:0|0:0,0:0:0:0:,",
        "L|1:2,1,100,,,",
        "L|1:2,1,100,2|0,0:0|0:0",
        "L|1:2,1,100,2|0",
        "L|1:2, 1 , 100 ,0|0,0:0|0:0,0:0:0:0:",
        "L|1:2,100,100",
        "L|1:2,9001,100",
        "L|1:2,1,131072",
        "L|1:2,1,131072.5",
        "L|1:2,1,-5",
        "L|1:2,1,1e2",
        "L|1:2,1,,",
        "B|1:2|3:4|5:6|7:8|9:10|11:12|13:14|15:16|17:18,1,142.499996185303,0|0|0|0,0:0|0:0|0:0|0:0,0:0:0:0:",
        "L|1:2,1,100,0|0,0:0|0:0,0:0:0:0:file name with spaces.wav",
        "L|1:2,1,100,0|0,0:0|0:0,0:0:0:0:x,junk,more",
        "L|1:2.5,1,100",
        "L|-1:2,1,100",
        "L|1:2,1,100,0|0,0:0|9:9,0:0:0:0:",
        "L|1:2,1,100,0|0,0:0|/:0,0:0:0:0:",
    };
    for (size_t i = 0; i < sizeof(tails) / sizeof(tails[0]); i++) {
        check(std::string("[HitObjects]\n256,192,1000,2,0,") + tails[i] + "\n1,2,3,1,0\n");
        check(std::string("[HitObjects]\n256,192,1000,2,0,") + tails[i] + "\r\n");
    }
    {
        const Beatmap m = parse_str("[HitObjects]\n1,2,3,2,0,L|1:2,1,100,0|0,0:0|0:0,0:0:0:0:a,b\n");
        CHECK(m.hit_objects.size() == 1 && m.hit_objects[0].hit_sample == StringView("0:0:0:0:a"));
    }
}

PPPP_TEST(decimal_boundaries) {
    static const char* const decimals[] = {"111.99999999999987", "999.9999999999999", "99999.9999999999999"};
    for (size_t i = 0; i < 3; i++) {
        const std::string decimal = decimals[i];
        const std::string text = "[TimingPoints]\n0,100.0000000000000,4,2,1,100,1,0\n1," + decimal +
                                 ",4,2,1,100,1,0\n[HitObjects]\n1,2,3,2,0,B|1:2,1," + decimal;
        check(text);
        const Beatmap map = parse_str(text);
        const double expected = std::strtod(decimal.c_str(), 0);
        CHECK(map.timing_points[1].beat_length == expected);
        CHECK(map.sliders[0].length == expected);
    }
    {
        const Beatmap m = parse_str("[Metadata]\nTitle:real\nTitlX:wrong\nBeatmapID:-9223372036854775808\n"
                                    "[MetadataFake]\nTitle:wrong\n[Difficulty]\nApproachRate:1e309\n"
                                    "[TimingPoints]\n0,500\n1,NaN,4,2,1,100,0,0\n2,NaN,4,2,1,100,1,0\n"
                                    "[HitObjects]\n256.5,192,1000.5,1,0\n1,2,2000.25,8,0,3000.75\n"
                                    "1,2,4000,2,0,B|1.5:2.5,1,2.5e2\n"
                                    "[Events]\n2,1e309,100\n2,1.25,9.75\n");
        CHECK(m.title == StringView("real") && m.beatmap_id == -1);
        CHECK(m.ar == 5 && m.stats.malformed_lines == 4);
        CHECK(m.timing_points.size() == 2 &&
              m.timing_points[1].beat_length != m.timing_points[1].beat_length);
        CHECK(!m.timing_points[1].uninherited);
        CHECK(m.hit_objects.size() == 3 && m.hit_objects[0].x == 256);
        CHECK(m.hit_objects[0].time == 1000.5 && m.hit_objects[1].end_time == 3000.75);
        CHECK(m.sliders[0].length == 250 && m.slider_points[0].x == 1);
        CHECK(m.breaks.size() == 1 && m.breaks[0].start == 1.25 && m.breaks[0].end == 9.75);
    }
}

PPPP_TEST(truncated_inputs) {
    // Long digit runs and every truncation point, including EOF without LF.
    const std::string lines[] = {
        "[HitObjects]\n1,2,3,2,0,B|" + std::string(150, '9'),
        "[HitObjects]\n1,2,3,2,0,B|1:2,1," + std::string(150, '9'),
        "[TimingPoints]\n" + std::string(150, '9'),
        std::string("[Metadata]\nBeatmapID:-9223372036854775808\n"),
        std::string("[HitObjects]\n1,2,3,1,0\0junk", 27),
    };
    for (size_t l = 0; l < 5; l++) {
        for (size_t size = 0; size <= lines[l].size(); ++size) {
            check(lines[l].substr(0, size));
        }
    }
    static const char* const numbers[] = {"9223372036854775807", "9223372036854775808",
                                          "18446744073709551615", "99999999999999999999999999999"};
    for (size_t i = 0; i < 4; i++) {
        check(std::string("[Metadata]\nBeatmapID:") + numbers[i]);
        check(std::string("[Metadata]\nBeatmapID:-") + numbers[i]);
        CHECK(parse_str(std::string("[Metadata]\nBeatmapID:") + numbers[i]).beatmap_id == -1);
    }
    // Explicit logical end must bound the slow numeric fallback too.
    FileBuffer input;
    CHECK(make_padded(StringView("1.234567890123456789e2junk"), input));
    double value;
    const char* start = input.data;
    CHECK(internal::parse_double(start, start + 20, value) <= start + 20);
}

PPPP_TEST_MAIN()
