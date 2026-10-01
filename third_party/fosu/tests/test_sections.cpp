// Section parsing (upstream tests/test_sections.cc, without the SIMD-only cases).
#include "test_support.h"
#include <cmath>
#include <cstring>

using namespace fosu;

PPPP_TEST(all_sections) {
    const char* input = "\xEF\xBB\xBFosu file format v14\r\n"
                        "\r\n"
                        "[General]\r\n"
                        "AudioFilename: audio.mp3\r\n"
                        "AudioLeadIn: 0\r\n"
                        "PreviewTime: 53342\r\n"
                        "Countdown: 0\r\n"
                        "SampleSet: Soft\r\n"
                        "SampleVolume: 73\r\n"
                        "StackLeniency: 0.7\r\n"
                        "Mode: 0\r\n"
                        "LetterboxInBreaks: 0\r\n"
                        "WidescreenStoryboard: 1\r\n"
                        "\r\n"
                        "[Editor]\r\n"
                        "Bookmarks: 11240,22540\r\n"
                        "DistanceSpacing: 1.1\r\n"
                        "BeatDivisor: 4\r\n"
                        "GridSize: 32\r\n"
                        "TimelineZoom: 2.4\r\n"
                        "VelocityPresets: 1,1.5,2\r\n"
                        "\r\n"
                        "[Metadata]\r\n"
                        "Title:Painters of the Tempest\r\n"
                        "TitleUnicode:Painters of the Tempest\r\n"
                        "Artist:Ne Obliviscaris\r\n"
                        "ArtistUnicode:Ne Obliviscaris\r\n"
                        "Creator:cmyui\r\n"
                        "Version:Extreme\r\n"
                        "Source:\r\n"
                        "Tags:prog metal akatsuki\r\n"
                        "BeatmapID:1193177\r\n"
                        "BeatmapSetID:562454\r\n"
                        "\r\n"
                        "[Difficulty]\r\n"
                        "HPDrainRate:5.5\r\n"
                        "CircleSize:4\r\n"
                        "OverallDifficulty:9\r\n"
                        "ApproachRate:9.3\r\n"
                        "SliderMultiplier:1.8\r\n"
                        "SliderTickRate:1\r\n"
                        "\r\n"
                        "[Events]\r\n"
                        "//Background and Video events\r\n"
                        "0,0,\"bg.jpg\",0,0\r\n"
                        "Video,-320,\"intro.mp4\"\r\n"
                        "//Break Periods\r\n"
                        "2,133342,140010\r\n"
                        "//Storyboard Layer 0 (Background)\r\n"
                        "Sprite,Background,Centre,\"sb/flash.png\",320,240\r\n"
                        " F,0,133342,,1,0\r\n"
                        "\r\n"
                        "[TimingPoints]\r\n"
                        "1240,342.857142857143,4,2,1,60,1,0\r\n"
                        "11240,-83.3333333333333,4,2,1,60,0,1\r\n"
                        "\r\n"
                        "[Colours]\r\n"
                        "Combo1 : 255,128,0\r\n"
                        "Combo2 : 0,64,255\r\n"
                        "\r\n"
                        "[HitObjects]\r\n"
                        "256,192,11240,1,0,0:0:0:0:\r\n"
                        "100,100,11583,5,12,0:0:0:0:\r\n"
                        "52,84,11926,2,0,B|172:44|292:84,1,240,2|0,0:0|0:0,0:0:0:0:\r\n"
                        "256,192,13297,12,4,15354,0:0:0:0:\r\n"
                        "448,320,15697,6,2,P|384:236|306:222,2,180.599999999999\r\n"
                        "77,406,17068,1,2\r\n";
    const Beatmap bm = parse_str(input);
    CHECK(bm.format_version == 14);
    CHECK(bm.audio_filename == StringView("audio.mp3"));
    CHECK(bm.preview_time == 53342);
    CHECK(bm.sample_set == SampleSet::Soft);
    CHECK(bm.sample_volume == 73);
    CHECK(bm.widescreen_storyboard);
    CHECK(!bm.letterbox_in_breaks);
    CHECK(bm.bookmarks == StringView("11240,22540"));
    CHECK(bm.grid_size == 32);
    CHECK(bm.velocity_presets.size() == 3u);
    CHECK(bm.velocity_presets[0] == 1);
    CHECK(bm.velocity_presets[1] == 1.5);
    CHECK(bm.velocity_presets[2] == 2);
    CHECK(bm.title == StringView("Painters of the Tempest"));
    CHECK(bm.creator == StringView("cmyui"));
    CHECK(bm.beatmap_id == 1193177);
    CHECK(bm.beatmap_set_id == 562454);
    CHECK(std::fabs(bm.hp - 5.5) < 1e-9);
    CHECK(bm.ar == double(9.3f));
    CHECK(std::fabs(bm.slider_multiplier - 1.8) < 1e-9);
    CHECK(bm.background == StringView("bg.jpg"));
    CHECK(bm.video == StringView("intro.mp4"));
    CHECK(bm.breaks.size() == 1u);
    CHECK(bm.breaks[0].start == 133342);
    CHECK(bm.breaks[0].end == 140010);
    CHECK(bm.stats.storyboard_lines == 2u);
    CHECK(bm.combo_colours.size() == 2u);
    CHECK(bm.combo_colours[0] == 0xFF8000u);
    CHECK(bm.combo_colours[1] == 0x0040FFu);

    CHECK(bm.timing_points.size() == 2u);
    CHECK(std::fabs(bm.timing_points[0].beat_length - 342.857142857143) < 1e-6);
    CHECK(bm.timing_points[0].uninherited);
    CHECK(!bm.timing_points[1].uninherited);
    CHECK(bm.timing_points[1].effects == 1u);
    CHECK(bm.timing_points[1].volume == 60);

    CHECK(bm.hit_objects.size() == 6u);
    const HitObject& circ = bm.hit_objects[0];
    CHECK(circ.x == 256);
    CHECK(circ.y == 192);
    CHECK(circ.time == 11240);
    CHECK(circ.type == 1u);
    CHECK(circ.hitsound == 0u);
    CHECK(circ.hit_sample == StringView("0:0:0:0:"));
    CHECK(bm.hit_objects[1].hitsound == 12u); // 2-digit hitsound

    const HitObject& sl = bm.hit_objects[2];
    CHECK(sl.is_slider());
    CHECK(sl.slider != HitObject::kNoSlider);
    const Slider& s = bm.sliders[sl.slider];
    CHECK(s.curve_type == CurveType::Bezier);
    CHECK(s.point_count == 2u);
    CHECK(bm.slider_points[s.point_begin].x == 172);
    CHECK(bm.slider_points[s.point_begin].y == 44);
    CHECK(bm.slider_points[s.point_begin + 1].x == 292);
    CHECK(s.slides == 1);
    CHECK(std::fabs(s.length - 240) < 1e-9);
    CHECK(s.edge_sounds == StringView("2|0"));
    CHECK(s.edge_sets == StringView("0:0|0:0"));
    CHECK(sl.hit_sample == StringView("0:0:0:0:"));

    const HitObject& sp = bm.hit_objects[3];
    CHECK(sp.is_spinner());
    CHECK(sp.end_time == 15354);

    const HitObject& sl2 = bm.hit_objects[4];
    CHECK(sl2.is_slider());
    CHECK(std::fabs(bm.sliders[sl2.slider].length - 180.599999999999) < 1e-6);
    CHECK(bm.sliders[sl2.slider].slides == 2);

    const HitObject& bare = bm.hit_objects[5]; // v3-style, no hitSample
    CHECK(bare.x == 77);
    CHECK(bare.hitsound == 2u);
    CHECK(bare.hit_sample.empty());

    CHECK(bm.stats.malformed_lines == 0u);
    CHECK(bm.stats.slow_path_lines == 6u);
}

PPPP_TEST(old_format) {
    const Beatmap bm = parse_str("osu file format v3\n"
                                 "[General]\n"
                                 "AudioFilename: old.mp3\n"
                                 "[Difficulty]\n"
                                 "HPDrainRate:6\n"
                                 "CircleSize:4\n"
                                 "OverallDifficulty:7\n"
                                 "SliderMultiplier: 1.4\n"
                                 "SliderTickRate: 1\n"
                                 "[TimingPoints]\n"
                                 "8074.13793103448,344.827586206897\n"
                                 "[HitObjects]\n"
                                 "56,120,8074,1,4\n"
                                 "200,72,8419,2,0,B|248:8|320:40,1,140\n");
    CHECK(bm.format_version == 3);
    CHECK(std::fabs(bm.ar - 7.0) < 1e-9); // AR falls back to OD
    CHECK(bm.timing_points.size() == 1u);
    CHECK(bm.timing_points[0].meter == 4);
    CHECK(bm.timing_points[0].volume == 100);
    CHECK(bm.timing_points[0].uninherited);
    CHECK(bm.hit_objects.size() == 2u);
    CHECK(bm.hit_objects[0].hitsound == 4u);
}

PPPP_TEST(long_timing_offsets) {
    // Offsets past an 8-byte digit window (>27h marathons) must not vanish as malformed.
    const Beatmap bm = parse_str("osu file format v14\n"
                                 "[TimingPoints]\n"
                                 "123456789,300.5,4,2,1,60,1,0\n"
                                 "2123456789,-50,4,2,1,60,0,0\n");
    CHECK(bm.timing_points.size() == 2u);
    CHECK(bm.timing_points[0].time == 123456789.0);
    CHECK(bm.timing_points[1].time == 2123456789.0);
    CHECK(bm.timing_points[0].volume == 60);
    CHECK(bm.stats.malformed_lines == 0u);
}

PPPP_TEST(mania_hold) {
    const Beatmap bm = parse_str("osu file format v14\n"
                                 "[General]\n"
                                 "Mode: 3\n"
                                 "[HitObjects]\n"
                                 "320,192,16504,128,0,16999:0:0:0:0:\n");
    CHECK(bm.hit_objects.size() == 1u);
    const HitObject& h = bm.hit_objects[0];
    CHECK(h.is_hold());
    CHECK(h.end_time == 16999);
    CHECK(h.hit_sample == StringView("0:0:0:0:"));
}

PPPP_TEST(aspire_edge_cases) {
    const Beatmap bm = parse_str("osu file format v14\n"
                                 "[HitObjects]\n"
                                 "-48,192,1000,1,0\n" // negative x
                                 "640,-24,2000,1,0\n" // negative y
                                 "256,192,-1000,1,0\n" // negative time
                                 "5120,192,3000,1,0\n" // 4-digit x
                                 "256.5,112.2,4000,1,0\n" // decimal coords
                                 "0,0,4294967290,1,0\n" // time > INT32_MAX
                                 "100,100,5000,2,0,B|-64:-32|700:512,1,600\n"); // negative ctrl points
    CHECK(bm.hit_objects.size() == 6u);
    CHECK(bm.hit_objects[0].time == -1000);
    CHECK(bm.hit_objects[1].x == -48);
    CHECK(bm.hit_objects[2].y == -24);
    CHECK(bm.hit_objects[3].x == 5120);
    CHECK(bm.hit_objects[4].x == 256); // truncated
    CHECK(bm.hit_objects[4].y == 112);
    const Slider& s = bm.sliders[bm.hit_objects[5].slider];
    CHECK(bm.slider_points[s.point_begin].x == -64);
    CHECK(bm.slider_points[s.point_begin].y == -32);
    CHECK(bm.stats.malformed_lines == 1u);
}

PPPP_TEST(modern_curve_segments) {
    const Beatmap modern = parse_str("osu file format v128\n[HitObjects]\n"
                                     "10,20,100,2,0,B2|30.5:40.25|50:60|L|70.75:80.5,1,100\n");
    CHECK(modern.hit_objects.size() == 1u);
    const Slider& slider = modern.sliders[modern.hit_objects[0].slider];
    CHECK(slider.segment_count == 2u);
    const CurveSegment first = modern.slider_segments[slider.segment_begin];
    const CurveSegment second = modern.slider_segments[slider.segment_begin + 1];
    CHECK(first.type == CurveType::Bezier);
    CHECK(first.has_degree && first.degree == 2u);
    CHECK(first.point_count == 3u);
    CHECK(second.type == CurveType::Linear);
    CHECK(!second.has_degree);
    CHECK(second.point_count == 1u);
    CHECK(modern.slider_points[slider.point_begin + first.point_begin].x == 30.5f);
    CHECK(modern.slider_points[slider.point_begin + first.point_begin].y == 40.25f);
    CHECK(modern.slider_points[slider.point_begin + second.point_begin].x == 70.75f);

    const Beatmap degree = parse_str("osu file format v128\n[HitObjects]\n"
                                     "0,0,100,2,0,B2|100:0|100:100|0:100,1,300\n");
    const Slider& degree_slider = degree.sliders[degree.hit_objects[0].slider];
    CHECK(degree_slider.segment_count == 1u);
    const CurveSegment degree_segment = degree.slider_segments[degree_slider.segment_begin];
    CHECK(degree_segment.type == CurveType::Bezier);
    CHECK(degree_segment.has_degree && degree_segment.degree == 2u);
    CHECK(degree_segment.point_count == 3u);

    const Beatmap coordinates = parse_str("osu file format v128\n[HitObjects]\n"
                                          "256.99853,256.001,100,1,0\n");
    CHECK(coordinates.hit_objects[0].x == static_cast<float>(256.99853));
    CHECK(coordinates.hit_objects[0].y == static_cast<float>(256.001));

    const Beatmap legacy = parse_str("osu file format v14\n[HitObjects]\n"
                                     "10,20,100,2,0,B|30.5:40.25,1,100\n");
    const Slider& legacy_slider = legacy.sliders[legacy.hit_objects[0].slider];
    CHECK(legacy_slider.segment_count == 0u);
    CHECK(legacy.slider_points[legacy_slider.point_begin].x == 30.0f);
    CHECK(legacy.slider_points[legacy_slider.point_begin].y == 40.0f);
}

PPPP_TEST(malformed) {
    const Beatmap bm = parse_str("osu file format v14\n"
                                 "[HitObjects]\n"
                                 "\n"
                                 ",,,,\n"
                                 "abc\n"
                                 "12,,123,4,0\n" // empty field (index-alias trap)
                                 "256,192\n" // truncated line
                                 "256,192,1000,2,0,B|\n" // truncated slider
                                 "256,192,1000,1,0\n"); // one valid line
    CHECK(bm.hit_objects.size() == 1u);
    CHECK(bm.hit_objects[0].time == 1000);
    CHECK(bm.stats.malformed_lines == 5u);
}

PPPP_TEST(invalid_byte_in_hitobjects) {
    static const char* const records[] = {
        "256,192,100,1,0,0:0:0:0:",
        "256,192,100,2,0,B|100:100|200:200,1,100",
        "256,192,100,8,0,200",
        "256,192,100,128,0,200:0:0:0:0:",
    };
    for (size_t r = 0; r < sizeof(records) / sizeof(records[0]); r++) {
        const std::string record(records[r]);
        for (size_t position = 0; position <= record.size(); ++position) {
            std::string damaged(record);
            damaged.insert(position, 1, '\x01');
            const Beatmap map = parse_str("osu file format v14\n[HitObjects]\n" + damaged +
                                          "\n300,100,300,1,0\n[Metadata]\nTitle:sentinel\n");
            CHECK_MSG(!map.hit_objects.empty(), damaged.c_str());
            if (!map.hit_objects.empty()) {
                CHECK_MSG(map.hit_objects[map.hit_objects.size() - 1].time == 300, damaged.c_str());
            }
            CHECK_MSG(map.title == StringView("sentinel"), damaged.c_str());
            if (map.hit_objects.size() == 1) {
                CHECK_MSG(map.stats.malformed_lines == 1u, damaged.c_str());
            }
            size_t required_end = 0;
            for (int field = 0; field < 5; ++field) {
                required_end = record.find(',', required_end) + 1;
            }
            if (position < required_end) {
                CHECK_MSG(map.hit_objects.size() == 1u, damaged.c_str());
                CHECK_MSG(map.stats.malformed_lines == 1u, damaged.c_str());
            }
        }
    }
}

PPPP_TEST(invalid_byte_in_timing_points) {
    const std::string timing = "100,500,4,2,1,60,1,0";
    for (size_t position = 0; position <= timing.size(); ++position) {
        std::string damaged(timing);
        damaged.insert(position, 1, '\x01');
        const Beatmap map = parse_str("osu file format v14\n[TimingPoints]\n" + damaged +
                                      "\n200,500,4,2,1,60,1,0\n[Metadata]\nTitle:sentinel\n");
        CHECK_MSG(!map.timing_points.empty(), damaged.c_str());
        if (!map.timing_points.empty()) {
            CHECK_MSG(map.timing_points[map.timing_points.size() - 1].time == 200, damaged.c_str());
        }
        CHECK_MSG(map.title == StringView("sentinel"), damaged.c_str());
        if (map.timing_points.size() == 1) {
            CHECK_MSG(map.stats.malformed_lines == 1u, damaged.c_str());
        }
        const size_t required_end = timing.find(',', timing.find(',') + 1) + 1;
        if (position < required_end) {
            CHECK_MSG(map.timing_points.size() == 1u, damaged.c_str());
            CHECK_MSG(map.stats.malformed_lines == 1u, damaged.c_str());
        }
    }
}

PPPP_TEST(invalid_byte_in_other_sections) {
    const Beatmap map = parse_str("osu file format v14\n"
                                  "[General]\nMode:\x01\nMode:0\n"
                                  "[Editor]\nGridSize:3\x01\nGridSize:32\n"
                                  "[Difficulty]\nSliderMultiplier:1.\x01\nSliderMultiplier:1.4\n"
                                  "[Events]\n2,10\x01,20\n2,30,40\n"
                                  "[Colours]\nCombo1:1,\x01,3\nCombo2:4,5,6\n"
                                  "[Metadata]\nTitle:a\x01"
                                  "b\nBeatmapID:1\x01\nBeatmapID:42\n"
                                  "[HitObjects]\n1,2,100,1,0\n");
    CHECK(map.mode == 0);
    CHECK(map.grid_size == 32);
    CHECK(map.slider_multiplier == 1.4);
    CHECK(map.breaks.size() == 1u);
    CHECK(map.breaks[0].start == 30);
    CHECK(map.combo_colours.size() == 1u);
    CHECK(map.beatmap_id == 42);
    CHECK(map.title == std::string("a\x01"
                                   "b"));
    CHECK(map.hit_objects.size() == 1u);
    CHECK(map.stats.malformed_lines == 6u);
}

PPPP_TEST(bracketed_records_do_not_change_section) {
    const Beatmap bracketed = parse_str("osu file format v14\n[HitObjects]\n[256,192,100,1,0\n"
                                        "300,100,300,1,0\n[TimingPoints]\n[100,500,4,2,1,60,1,0\n"
                                        "200,500,4,2,1,60,1,0\n[Metadata]\nTitle:sentinel\n");
    CHECK(bracketed.hit_objects.size() == 1u);
    CHECK(bracketed.timing_points.size() == 1u);
    CHECK(bracketed.stats.malformed_lines == 2u);
    CHECK(bracketed.title == StringView("sentinel"));
}

PPPP_TEST(unknown_section_does_not_resume_hitobjects) {
    const Beatmap unknown = parse_str("[HitObjects]\n1,2,100,1,0\n[Unknown]\n1,2,200,1,0\n"
                                      "[HitObjects]\n1,2,300,1,0\n");
    CHECK(unknown.hit_objects.size() == 2u);
    CHECK(unknown.stats.malformed_lines == 0u);
}

PPPP_TEST(line_endings) {
    const std::string lines = "osu file format v14\n"
                              "[General]\nMode:0\n"
                              "[Editor]\nGridSize:32\n"
                              "[Metadata]\nTitle:sentinel\n"
                              "[Difficulty]\nSliderMultiplier:1.4\n"
                              "[Events]\n2,100,200\n"
                              "[TimingPoints]\n100,500,4,1,0,100,1,0\n"
                              "[Colours]\nCombo1:255,128,0\n"
                              "[HitObjects]\n256,192,1000,1,0\n";
    static const char* const endings[] = {"\n", "\r\n", "\r"};
    for (size_t e = 0; e < 3; e++) {
        std::string input;
        for (size_t i = 0; i < lines.size(); i++) {
            if (lines[i] == '\n') {
                input.append(endings[e]);
            } else {
                input.push_back(lines[i]);
            }
        }
        const Beatmap map = parse_str(input);
        CHECK(map.grid_size == 32);
        CHECK(map.title == StringView("sentinel"));
        CHECK(map.breaks.size() == 1u);
        CHECK(map.timing_points.size() == 1u);
        CHECK(map.combo_colours.size() == 1u);
        CHECK(map.hit_objects.size() == 1u);
        CHECK(map.stats.malformed_lines == 0u);
    }
}

// A document whose CRLF framing is damaged around the [HitObjects] header or its first record.
static void check_crlf_recovery(const char* header_and_first, size_t hit_objects, size_t timing_points) {
    const Beatmap map = parse_str(std::string("osu file format v14\r\n"
                                              "[TimingPoints]\r\n100,500,4,1,0,100,1,0\r\n"
                                              "200,500,4,1,0,100,1,0\r\n") +
                                  header_and_first +
                                  "300,100,2000,1,0\r\n"
                                  "[Metadata]\r\nTitle:after\r\n");
    CHECK(map.hit_objects.size() == hit_objects);
    CHECK(map.timing_points.size() == timing_points);
    CHECK(map.title == StringView("after"));
    CHECK(map.stats.malformed_lines == 1u);
}

PPPP_TEST(invalid_byte_before_header_cr) {
    check_crlf_recovery("[HitObjects]\x01\r\n256,192,1000,1,0\r\n", 0, 4);
}

PPPP_TEST(invalid_byte_between_header_cr_lf) {
    check_crlf_recovery("[HitObjects]\r\x01\n256,192,1000,1,0\r\n", 2, 2);
}

PPPP_TEST(invalid_byte_after_header_lf) {
    check_crlf_recovery("[HitObjects]\r\n\x01"
                        "256,192,1000,1,0\r\n",
                        1, 2);
}

PPPP_TEST(invalid_byte_replaces_header_cr) {
    check_crlf_recovery("[HitObjects]\x01\n256,192,1000,1,0\r\n", 0, 4);
}

PPPP_TEST(invalid_byte_replaces_header_lf) {
    check_crlf_recovery("[HitObjects]\r\x01"
                        "256,192,1000,1,0\r\n",
                        1, 2);
}

PPPP_TEST(invalid_byte_before_hitobject_cr) {
    check_crlf_recovery("[HitObjects]\r\n256,192,1000,1,0\x01\r\n", 1, 2);
}

PPPP_TEST(invalid_byte_between_hitobject_cr_lf) {
    check_crlf_recovery("[HitObjects]\r\n256,192,1000,1,0\r\x01\n", 2, 2);
}

PPPP_TEST(invalid_byte_after_hitobject_lf) {
    check_crlf_recovery("[HitObjects]\r\n256,192,1000,1,0\r\n\x01", 1, 2);
}

PPPP_TEST(invalid_byte_replaces_hitobject_cr) {
    check_crlf_recovery("[HitObjects]\r\n256,192,1000,1,0\x01\n", 1, 2);
}

PPPP_TEST(invalid_byte_replaces_hitobject_lf) {
    check_crlf_recovery("[HitObjects]\r\n256,192,1000,1,0\r\x01", 1, 2);
}

PPPP_TEST(omitted_sections_use_defaults) {
    const Beatmap bm = parse_str("[Metadata]\nTitle:Only metadata\n");
    CHECK(bm.title == StringView("Only metadata"));
    CHECK(bm.audio_filename.empty());
    CHECK(bm.sample_set == SampleSet::Normal);
    CHECK(bm.preview_time == -1);
    CHECK(bm.grid_size == 0);
    CHECK(bm.hp == 5);
    CHECK(bm.cs == 5);
    CHECK(bm.od == 5);
    CHECK(bm.ar == 5);
    CHECK(bm.background.empty() && bm.video.empty());
    CHECK(bm.timing_points.empty() && bm.breaks.empty());
    CHECK(bm.combo_colours.empty() && bm.hit_objects.empty());
    CHECK(bm.sliders.empty() && bm.slider_points.empty());
    CHECK(bm.velocity_presets.size() == 3u);
    CHECK(bm.velocity_presets[0] == 0.75);
    CHECK(bm.velocity_presets[1] == 1);
    CHECK(bm.velocity_presets[2] == 1.5);
    CHECK(bm.stats.malformed_lines == 0u);
}

static ParseOptions sections_only(fosu_uint32 sections) {
    ParseOptions options;
    options.sections = sections;
    return options;
}

PPPP_TEST(difficulty_selection_skips_other_sections) {
    const Beatmap bm =
        parse_str("[Metadata]\nTitle:Unrequested\n"
                  "[Difficulty]\nHPDrainRate:3\nCircleSize:4\nOverallDifficulty:7\nApproachRate:8\n"
                  "[TimingPoints]\n0,500\n"
                  "[Colours]\nCombo1:255,0,0\n"
                  "[HitObjects]\n64,96,1000,1,0\n",
                  sections_only(kSectionDifficulty));
    CHECK(bm.hp == 3);
    CHECK(bm.cs == 4);
    CHECK(bm.od == 7);
    CHECK(bm.ar == 8);
    CHECK(bm.title.empty());
    CHECK(bm.timing_points.empty() && bm.combo_colours.empty());
    CHECK(bm.hit_objects.empty());
}

PPPP_TEST(metadata_and_difficulty_selection) {
    const Beatmap bm = parse_str("[General]\nAudioFilename:unrequested.mp3\n"
                                 "[Metadata]\nTitle:Selected metadata\nBeatmapID:42\n"
                                 "[Difficulty]\nOverallDifficulty:6\n"
                                 "[HitObjects]\n128,192,2000,1,0\n",
                                 sections_only(kSectionMetadata | kSectionDifficulty));
    CHECK(bm.title == StringView("Selected metadata"));
    CHECK(bm.beatmap_id == 42);
    CHECK(bm.od == 6);
    CHECK(bm.ar == 6);
    CHECK(bm.audio_filename.empty());
    CHECK(bm.hit_objects.empty());
}

PPPP_TEST(hitobject_selection_skips_preceding_sections) {
    const Beatmap bm = parse_str("[Metadata]\nTitle:Skipped metadata\n"
                                 "[TimingPoints]\n100,400\n"
                                 "[HitObjects]\n32,48,3000,1,2\n256,192,4000,8,0,5000\n",
                                 sections_only(kSectionHitObjects));
    CHECK(bm.title.empty() && bm.timing_points.empty());
    CHECK(bm.hit_objects.size() == 2u);
    CHECK(bm.hit_objects[0].x == 32);
    CHECK(bm.hit_objects[0].time == 3000);
    CHECK(bm.hit_objects[0].hitsound == 2u);
    CHECK(bm.hit_objects[1].is_spinner());
    CHECK(bm.hit_objects[1].end_time == 5000);
}

PPPP_TEST(selected_missing_section_uses_defaults) {
    const Beatmap bm = parse_str("[Metadata]\nTitle:No difficulty section\n"
                                 "[HitObjects]\n96,64,6000,1,0\n",
                                 sections_only(kSectionDifficulty));
    CHECK(bm.hp == 5);
    CHECK(bm.cs == 5);
    CHECK(bm.od == 5);
    CHECK(bm.ar == 5);
    CHECK(bm.title.empty() && bm.hit_objects.empty());
    CHECK(bm.stats.malformed_lines == 0u);
}

PPPP_TEST(all_section_mask_matches_default) {
    const std::string input = "[General]\nMode:3\n"
                              "[Metadata]\nTitle:Explicit all sections\n"
                              "[Events]\n2,100.25,200.75\n"
                              "[TimingPoints]\n300,250\n"
                              "[HitObjects]\n320,192,7000,128,0,7500:0:0:0:0:\n";
    Parser explicit_parser;
    Parser default_parser;
    const Beatmap& explicit_mask =
        require_parse(explicit_parser.parse(input.data(), input.size(), sections_only(kAllSections)));
    const Beatmap& default_mask = require_parse(default_parser.parse(input.data(), input.size()));
    CHECK(explicit_mask.mode == default_mask.mode);
    CHECK(explicit_mask.title == default_mask.title);
    CHECK(explicit_mask.breaks.size() == 1u && default_mask.breaks.size() == 1u);
    CHECK(explicit_mask.breaks[0].end == default_mask.breaks[0].end);
    CHECK(explicit_mask.timing_points.size() == default_mask.timing_points.size());
    CHECK(explicit_mask.hit_objects.size() == 1u && default_mask.hit_objects.size() == 1u);
    CHECK(explicit_mask.hit_objects[0].end_time == default_mask.hit_objects[0].end_time);
    CHECK(explicit_mask.hit_objects[0].hit_sample == default_mask.hit_objects[0].hit_sample);
    CHECK(parse_str("[General]\nCountdown:DoubleSpeed\n").countdown == 3);
}

PPPP_TEST(exact_keys_and_event_aliases) {
    const Beatmap map = parse_str("[General]\nCountdown:Normal,HalfSpeed\nSampleSet:Soft\n"
                                  "[Metadata]\nTitle:\tkept \nTitleUnicode : unicode\n"
                                  "TitleExtra:ignored\n Title:ignored\nTitle\t: final: title \n"
                                  "[MetadataExtra]\nTitle:ignored section\n"
                                  "[Events]\n0,0,\"background.jpg\"\n"
                                  "1,0,\"old.mp4\"\nVideo,0,\"new.mp4\"\n"
                                  "2,10,20\nBreak,30,40\nVideoExtra,0,\"ignored.mp4\"\n");
    CHECK(map.countdown == 3);
    CHECK(map.sample_set == SampleSet::Soft);
    CHECK(map.title == StringView("final: title"));
    CHECK(map.title_unicode == StringView("unicode"));
    CHECK(map.background == StringView("background.jpg"));
    CHECK(map.video == StringView("new.mp4"));
    CHECK(map.breaks.size() == 2u);
    CHECK(map.breaks[0].start == 10);
    CHECK(map.breaks[1].end == 40);
    CHECK(map.stats.storyboard_lines == 1u);
}

PPPP_TEST(malformed_events) {
    const Beatmap map = parse_str("[Events]\n2,10,bad\nStoryboardLine\n");
    CHECK(map.breaks.empty());
    CHECK(map.stats.malformed_lines == 1u);
    CHECK(map.stats.storyboard_lines == 1u);
}

PPPP_TEST(long_event_lines) {
    static const size_t lengths[] = {63, 64, 65, 95, 96, 97, 200};
    static const char* const endings[] = {"", "\n", "\r\n"};
    for (size_t l = 0; l < 7; l++) {
        const size_t length = lengths[l];
        for (size_t e = 0; e < 3; e++) {
            const std::string filename(length - 6, 'x');
            const Beatmap map = parse_str("[Events]\n0,0,\"" + filename + "\"" + endings[e]);
            CHECK(map.background == filename);
        }
        const Beatmap map =
            parse_str("[Events]\n " + std::string(length, 'x') + "\n[Metadata]\nTitle:after events\n");
        CHECK(map.stats.storyboard_lines == 1u);
        CHECK(map.title == StringView("after events"));
    }
}

PPPP_TEST(timing_integer_widths) {
    static const long values[] = {9, 99, 999, 9999, 10000, 99999999, 2147483647L};
    for (size_t i = 0; i < 7; i++) {
        const std::string field = to_str(values[i]);
        const std::string input =
            "[TimingPoints]\n0,-100," + field + ",2," + field + "," + field + ",0," + field;
        const Beatmap map = parse_str(input);
        CHECK(map.timing_points.size() == 1u);
        CHECK(map.stats.malformed_lines == 0u);
        if (map.timing_points.size() != 1) {
            continue;
        }
        const TimingPoint& point = map.timing_points[0];
        CHECK(point.time == 0.0);
        CHECK(point.beat_length == -100.0);
        CHECK(point.meter == values[i]);
        CHECK(point.sample_set == SampleSet::Soft);
        CHECK(point.sample_index == values[i]);
        CHECK(point.volume == values[i]);
        CHECK(!point.uninherited);
        CHECK(point.effects == static_cast<fosu_uint32>(values[i]));
    }
}

PPPP_TEST(timing_point_spellings) {
    // The general numeric rules (upstream compares its masked fast path against these).
    struct Case {
        const char* line;
        bool accepted;
    };
    static const Case cases[] = {
        {"-1.5,500", true},
        {"0,NaN,4,0,0,100,0,0", true},
        {"0,500,0meter,0,0,100,1,0", true},
        {"0,500,4,0,0,100,1anything,0,ignored", true},
        {" 1 , 500 , 4 ,0,0,100,1,0", true},
        {"0,500,4,0,0,100,1,", false},
        {"0,500,,0,0,100,1,0", false},
        {"0,500,4,0,0,100,1,bad", false},
        {"0,NaN,4,0,0,100,1,0", false},
        {"bad,500", false},
        {"0,500,", false},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const Beatmap map = parse_str(std::string("[TimingPoints]\n") + cases[i].line + "\n");
        CHECK_MSG(map.timing_points.size() == (cases[i].accepted ? 1u : 0u), cases[i].line);
        CHECK_MSG(map.stats.malformed_lines == (cases[i].accepted ? 0u : 1u), cases[i].line);
    }
    const Beatmap meter = parse_str("[TimingPoints]\n0,500,0meter,0,0,100,1,0\n0,500,0,0,0,100,1,0\n");
    CHECK(meter.timing_points.size() == 2u);
    CHECK(meter.timing_points[0].meter == 4);
    CHECK(meter.timing_points[1].meter == 0);
    const Beatmap nan = parse_str("[TimingPoints]\n0,NaN,4,0,0,100,0,0\n");
    CHECK(nan.timing_points.size() == 1u &&
          nan.timing_points[0].beat_length != nan.timing_points[0].beat_length);
}

PPPP_TEST(byte_scan_boundaries) {
    static const char delimiters[] = {',', ':', '\n', '\0'};
    for (size_t d = 0; d < 4; d++) {
        for (size_t alignment = 0; alignment < 32; alignment += 7) {
            for (size_t length = 0; length <= 97; length += 5) {
                const size_t positions[] = {0, length / 2, length ? length - 1 : 0, length};
                for (size_t k = 0; k < 4; k++) {
                    const size_t position = positions[k];
                    std::string text(alignment + length + 1, 'x');
                    text[alignment + length] = delimiters[d];
                    if (position < length) {
                        text[alignment + position] = delimiters[d];
                    }
                    FileBuffer input;
                    CHECK(make_padded(StringView(text.data(), text.size()), input));
                    const char* p = input.data + alignment;
                    CHECK(internal::find_byte(delimiters[d], p, p + length) == p + position);
                }
            }
        }
    }
}

static void check_line_end_scan_boundaries(char ending) {
    static const size_t alignments[] = {0, 1, 15, 31};
    static const size_t lengths[] = {0, 1, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257};
    for (size_t a = 0; a < 4; a++) {
        for (size_t l = 0; l < sizeof(lengths) / sizeof(lengths[0]); l++) {
            const size_t alignment = alignments[a];
            const size_t length = lengths[l];
            const size_t positions[] = {0, length / 2, length ? length - 1 : 0, length};
            for (size_t k = 0; k < 4; k++) {
                const size_t position = positions[k];
                std::string text(alignment + length + 1, 'x');
                text[alignment + length] = ending;
                if (position < length) {
                    text[alignment + position] = ending;
                }
                FileBuffer input;
                CHECK(make_padded(StringView(text.data(), text.size()), input));
                const char* p = input.data + alignment;
                CHECK(internal::find_line_end(p, p + length) == p + position);
            }
        }
    }
}

PPPP_TEST(cr_scan_boundaries) { check_line_end_scan_boundaries('\r'); }

PPPP_TEST(lf_scan_boundaries) { check_line_end_scan_boundaries('\n'); }

PPPP_TEST(event_filename_boundaries) {
    static const size_t timestamp_lengths[] = {0, 30, 31, 32, 64};
    static const size_t filename_lengths[] = {0, 1, 30, 31, 32, 63, 64, 96};
    static const char* const suffixes[] = {"", ",0,0", "\nVideo,0"};
    for (size_t t = 0; t < 5; t++) {
        for (size_t f = 0; f < 8; f++) {
            const std::string filename(filename_lengths[f], 'x');
            const std::string event =
                "Video," + std::string(timestamp_lengths[t], '0') + ",\"" + filename + "\"";
            for (size_t s = 0; s < 3; s++) {
                const Beatmap map = parse_str("[Events]\n" + event + suffixes[s]);
                CHECK(map.video == filename);
            }
        }
    }
}

PPPP_TEST(section_skip_boundaries) {
    static const size_t paddings[] = {0, 30, 31, 32, 63, 64, 95};
    for (size_t i = 0; i < 7; i++) {
        Parser parser;
        const std::string text = "[Unknown]\nvalue:" + std::string(paddings[i], 'x') +
                                 "[Metadata]\nTitle:ignored\n[Metadata]\nTitle:retained";
        Beatmap& map = require_parse(parser.parse(text.data(), text.size(), sections_only(kSectionMetadata)));
        CHECK(map.title == StringView("retained"));
        const std::string missing = "[Unknown]\nvalue:[Metadata]";
        Beatmap& empty =
            require_parse(parser.parse(missing.data(), missing.size(), sections_only(kSectionMetadata)));
        CHECK(empty.title.empty());
    }
}

PPPP_TEST(enum_contracts) {
    static const char* const names[] = {"None", "Normal", "Soft", "Drum"};
    for (int value = 0; value < 4; ++value) {
        const std::string spellings[] = {names[value], to_str(value), " +" + to_str(value) + " ",
                                         "0" + to_str(value)};
        for (size_t s = 0; s < 4; s++) {
            const Beatmap map = parse_str("[General]\nSampleSet:" + spellings[s] + "\n");
            CHECK_MSG(map.stats.malformed_lines == 0u, spellings[s].c_str());
            CHECK_MSG(map.sample_set == static_cast<SampleSet::Value>(value), spellings[s].c_str());
        }
    }
    static const char* const invalid[] = {"-1", "4", "99", "2147483647", "Unknown", "Normal,Soft"};
    for (size_t i = 0; i < 6; i++) {
        const Beatmap map =
            parse_str(std::string("[General]\nSampleSet:Soft\nSampleSet:") + invalid[i] + "\n");
        CHECK_MSG(map.sample_set == SampleSet::Soft, invalid[i]);
        CHECK_MSG(map.stats.malformed_lines == 1u, invalid[i]);
    }
    static const char* const invalid_points[] = {"-1", "4", "9", "99", "2147483647"};
    for (size_t i = 0; i < 5; i++) {
        const Beatmap map = parse_str(std::string("[TimingPoints]\n0,500,4,") + invalid_points[i] +
                                      ",0,100,1,0\n1,500,4,2,0,100,1,0\n");
        CHECK(map.stats.malformed_lines == 1u);
        CHECK(map.timing_points.size() == 1u);
        CHECK(map.timing_points[0].sample_set == SampleSet::Soft);
    }
    static const char curve_types[] = {'B', 'C', 'L', 'P', 'X', 'b', '0'};
    for (size_t i = 0; i < 7; i++) {
        const char value = curve_types[i];
        const Beatmap map =
            parse_str(std::string("[HitObjects]\n0,0,1,2,0,") + value + "|1:2,1,30\n0,0,2,1,0\n");
        const bool valid = value == 'B' || value == 'C' || value == 'L' || value == 'P';
        CHECK(map.stats.malformed_lines == (valid ? 0u : 1u));
        CHECK(map.hit_objects.size() == (valid ? 2u : 1u));
        CHECK(map.sliders.size() == (valid ? 1u : 0u));
        CHECK(map.slider_points.size() == (valid ? 1u : 0u));
        if (valid) {
            CHECK(static_cast<char>(map.sliders[0].curve_type) == value);
        }
    }
}

PPPP_TEST(header_field_failures_preserve_values) {
    const Beatmap map = parse_str("[General]\nPreviewTime:17\nPreviewTime:2147483648\n"
                                  "Mode:3\nMode:4\nSampleSet:Soft\nSampleSet:Normal,Soft\n"
                                  "Countdown:Normal,HalfSpeed\nUseSkinSprites:1suffix\n"
                                  "LetterboxInBreaks:1suffix\nApproachRate:9\n"
                                  "[Metadata]\nTitle:  text:with:colons \t\n"
                                  "BeatmapID:2147483647\nBeatmapID:2147483648\nUnknown:bad\n"
                                  "[Difficulty]\nOverallDifficulty:6\nApproachRate:8.5\n"
                                  "[Difficulty]\nApproachRate:bad\nOverallDifficulty:7\n");
    CHECK(map.preview_time == 17);
    CHECK(map.mode == 3);
    CHECK(map.sample_set == SampleSet::Soft);
    CHECK(map.countdown == 3);
    CHECK(map.use_skin_sprites);
    CHECK(!map.letterbox_in_breaks);
    CHECK(map.title == StringView("text:with:colons"));
    CHECK(map.beatmap_id == kInt32Max);
    CHECK(map.ar == 8.5);
    CHECK(map.od == 7);
    CHECK(map.stats.malformed_lines == 6u);

    const Beatmap missing_ar = parse_str("[Difficulty]\nApproachRate:bad\nOverallDifficulty:6\n"
                                         "[General]\nApproachRate:9\n"
                                         "[Difficulty]\nOverallDifficulty:7\n");
    CHECK(missing_ar.ar == 7);
    CHECK(missing_ar.stats.malformed_lines == 1u);
}

PPPP_TEST(repeated_section_bodies) {
    const std::string text = "osu file format v14\n"
                             "[General]\nAudioFilename:first.mp3\n[Editor]\nGridSize:8\n"
                             "[Metadata]\nTitle:literal [Difficulty]\n[Difficulty]\nOverallDifficulty:7\n"
                             "[Events]\n2,1,2\n[TimingPoints]\n0,500\n[Colours]\nCombo1:1,2,3\n"
                             "[Future]\nTitle:ignored\n[HitObjects]\n1,2,3,1,0\n"
                             "[General]\n[Editor]\n[Metadata]\nArtist:final\n[Difficulty]\n"
                             "[Events]\n//comment\n2,3,4\n[TimingPoints]\n5,-100\n"
                             "[Colours]\nCombo2:4,5,6\n[HitObjects]\n4,5,6,1,0";
    for (int crlf = 0; crlf < 2; crlf++) {
        std::string input;
        for (size_t i = 0; i < text.size(); i++) {
            if (crlf && text[i] == '\n') {
                input += '\r';
            }
            input += text[i];
        }
        const Beatmap map = parse_str(input);
        CHECK(map.audio_filename == StringView("first.mp3"));
        CHECK(map.grid_size == 8);
        CHECK(map.title == StringView("literal [Difficulty]"));
        CHECK(map.artist == StringView("final"));
        CHECK(map.ar == 7);
        CHECK(map.breaks.size() == 2u);
        CHECK(map.timing_points.size() == 2u);
        CHECK(map.combo_colours.size() == 2u);
        CHECK(map.hit_objects.size() == 2u);
        if (map.hit_objects.size() == 2) {
            CHECK(map.hit_objects[1].time == 6);
        }
        CHECK(map.stats.malformed_lines == 0u);
    }
}

PPPP_TEST(repeated_lazer_velocity_presets) {
    const Beatmap map = parse_str("osu file format v128\n[Editor]\nVelocityPresets:1,2\n"
                                  "[Metadata]\nTitle:test\n[Editor]\nVelocityPresets:3,4,5,6,7\n");
    CHECK(map.velocity_presets.size() == 5u);
    CHECK(map.velocity_presets[0] == 3);
    CHECK(map.velocity_presets[1] == 4);
    CHECK(map.velocity_presets[2] == 5);
    CHECK(map.velocity_presets[3] == 6);
    CHECK(map.velocity_presets[4] == 7);
    CHECK(map.stats.malformed_lines == 0u);
}

PPPP_TEST(combo_colour_domain) {
    const Beatmap map = parse_str("[Colours]\nCombo1:1,2,3\nCombo0:4,5,6\nCombo9:7,8,9\n"
                                  "Combo:10,11,12\nCombo1suffix:13,14,15\nCombo-1:16,17,18\n"
                                  "Combo+8:19,20,21\nCombo01:22,23,24\nSliderBorder:25,26,27\n");
    CHECK(map.combo_colours.size() == 3u);
    CHECK(map.combo_colours[0] == 0x010203u);
    CHECK(map.combo_colours[1] == 0x131415u);
    CHECK(map.combo_colours[2] == 0x161718u);
    CHECK(map.stats.malformed_lines == 0u);

    const Beatmap malformed = parse_str("[Colours]\nCombo1:256,0,0\nCombo2:-1,0,0\nCombo3:1,2,3junk\n"
                                        "Combo4:1,2\nCombo5:1,2,3,4,5\nSliderBorder:no colour\n"
                                        "Combo6:1, 2 ,3,ignored alpha\nCombo7:4,5,6 // comment\n");
    CHECK(malformed.combo_colours.size() == 2u);
    CHECK(malformed.combo_colours[0] == 0x010203u);
    CHECK(malformed.combo_colours[1] == 0x040506u);
    CHECK(malformed.stats.malformed_lines == 6u);
}

PPPP_TEST(legacy_rules) {
    const Beatmap late_mode =
        parse_str("[Difficulty]\nCircleSize:18\nOverallDifficulty:20\nApproachRate:bad\n"
                  "[General]\nMode:0\n[General]\nMode:3\n");
    CHECK(late_mode.cs == 18);
    CHECK(late_mode.od == 10);
    CHECK(late_mode.ar == 10);
    const Beatmap explicit_ar = parse_str("[Difficulty]\nCircleSize:18\nApproachRate:20\n"
                                          "[Difficulty]\nApproachRate:bad\nOverallDifficulty:3\n"
                                          "[General]\nMode:3\n[General]\nMode:0\n");
    CHECK(explicit_ar.cs == 10);
    CHECK(explicit_ar.ar == 10);
    CHECK(explicit_ar.od == 3);
    const Beatmap map = parse_str("osu file format v4\n[General]\nMode:3\nPreviewTime:100\n"
                                  "[Metadata]\n Title :\xE3\x80\x80trimmed\xC2\xA0\n"
                                  "[Difficulty]\nHPDrainRate:20\nCircleSize:32\nOverallDifficulty:9.3\n"
                                  "SliderMultiplier:8\nSliderTickRate:0.1\n"
                                  "[Editor]\nDistanceSpacing:-1\nTimelineZoom:-2\nBeatDivisor:100\n"
                                  "[TimingPoints]\n0,500\n[Events]\n2,150,100\n"
                                  "[HitObjects]\n10,20,200,1,0\n30,40,0,8,0,-10\n"
                                  "50,60,50,2,0,B|100:100,0,0\n70,80,100,53,0\n"
                                  "90,100,100,49,0\n110,120,20,128,0,10\n130,140,201,1,0\n");
    CHECK(map.title == StringView("trimmed"));
    CHECK(map.hp == 10);
    CHECK(map.cs == 18);
    CHECK(map.ar == double(9.3f));
    CHECK(map.slider_multiplier == 3.6);
    CHECK(map.slider_tick_rate == 0.5);
    CHECK(map.distance_spacing == 0);
    CHECK(map.timeline_zoom == 0);
    CHECK(map.beat_divisor == 64);
    CHECK(map.preview_time == 124);
    CHECK(map.timing_points[0].time == 24);
    CHECK(map.breaks[0].start == 174);
    CHECK(map.breaks[0].end == 174);
    const Span<HitObject> objects = map.hit_objects;
    CHECK(objects[0].time == 24);
    CHECK(objects[0].end_time == 24);
    CHECK(objects[0].x == 256);
    CHECK(objects[0].y == 192);
    CHECK(objects[1].time == 44);
    CHECK(objects[1].end_time == 68);
    CHECK(objects[2].new_combo); // Slider follows spinner in source order.
    CHECK(map.sliders[objects[2].slider].slides == 1);
    CHECK(objects[3].x == 70); // Equal timestamps preserve input order.
    CHECK(objects[4].x == 90);
    CHECK(objects[3].combo_skip == 3);
    CHECK(objects[4].combo_skip == 0);
    CHECK(!objects[4].new_combo);
    CHECK(objects[5].new_combo); // First source object and first after break.
    CHECK(!objects[6].new_combo);

    const Beatmap breaks = parse_str("osu file format v14\n[Events]\n2,0,200\n2,0,100\n2,0,350\n"
                                     "[HitObjects]\n0,0,100,1,0\n0,0,200,1,0\n0,0,300,1,0\n"
                                     "0,0,400,1,0\n");
    CHECK(breaks.hit_objects[0].new_combo);
    CHECK(!breaks.hit_objects[1].new_combo); // Break ends are exclusive.
    CHECK(breaks.hit_objects[2].new_combo); // Earlier breaks cannot move backward.
    CHECK(breaks.hit_objects[3].new_combo);
}

PPPP_TEST_MAIN()
