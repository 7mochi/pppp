// Hit object field parsing (upstream tests/test_hitobject_fields.cc).
#include "test_support.h"
#include <fosu/engine/hit_objects/object_types.h>

using namespace fosu;

static std::string slider_document(const std::string& slider) {
    return "osu file format v14\n[HitObjects]\n0,0,0,2,0," + slider + '\n';
}

PPPP_TEST(slider_points) {
    struct Case {
        const char* points;
        float x;
        float y;
    };
    static const Case cases[] = {
        {"|172:44", 172, 44},
        {"|-1.9:2.9", -1, 2},
        {"|1:2e1", 1, 20},
        {"|131072:-131072", 131072, -131072},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const Beatmap map = parse_str(slider_document("B" + std::string(cases[i].points) + ",1,10"));
        CHECK(map.hit_objects.size() == 1u);
        CHECK(map.sliders.size() == 1u);
        if (map.sliders.empty()) {
            continue;
        }
        const Slider& slider = map.sliders[0];
        CHECK(slider.point_count == 1u);
        CHECK(map.slider_points[slider.point_begin].x == cases[i].x);
        CHECK(map.slider_points[slider.point_begin].y == cases[i].y);
    }

    static const char* const malformed[] = {"|", "|1", "|1:", "|:2", "|bad:2", "|1:bad", "|131073:2"};
    for (size_t i = 0; i < 7; i++) {
        const Beatmap map = parse_str(slider_document("B" + std::string(malformed[i]) + ",1,10"));
        CHECK_MSG(map.hit_objects.empty(), malformed[i]);
        CHECK_MSG(map.stats.malformed_lines == 1u, malformed[i]);
    }
}

PPPP_TEST(slider_pool_indices_after_rejected_record) {
    static const int versions[] = {14, 128};
    for (size_t v = 0; v < 2; v++) {
        const int version = versions[v];
        const std::string curve = version == 128 ? "B1" : "L";
        const Beatmap map = parse_str("osu file format v" + to_str(version) +
                                      "\n[HitObjects]\n0,0,100,1,0\n0,0,200,2,0," + curve +
                                      "|10:20,1,30\n"
                                      "0,0,300,2,0,L|30:40|B|50:60|bad:80,1,30\n"
                                      "[Metadata]\nTitle:gap\n[HitObjects]\n"
                                      "0.5,0,400,2,0," +
                                      curve + "|70:80,1,40\n");
        CHECK(map.stats.malformed_lines == 1u);
        CHECK(map.hit_objects.size() == 3u);
        CHECK(map.sliders.size() == 2u);
        CHECK(map.slider_points.size() == 2u);
        CHECK(map.slider_segments.size() == (version == 128 ? 2u : 0u));
        if (map.hit_objects.size() != 3 || map.sliders.size() != 2 || map.slider_points.size() != 2) {
            continue;
        }
        CHECK(map.hit_objects[0].slider == HitObject::kNoSlider);
        CHECK(map.hit_objects[1].slider == 0u);
        CHECK(map.hit_objects[2].slider == 1u);
        for (size_t i = 0; i < 2; ++i) {
            CHECK(map.sliders[i].point_begin == i);
            CHECK(map.sliders[i].point_count == 1u);
            CHECK(map.sliders[i].segment_begin == (version == 128 ? i : 0u));
            CHECK(map.sliders[i].segment_count == (version == 128 ? 1u : 0u));
        }
        CHECK(map.slider_points[0].x == 10);
        CHECK(map.slider_points[0].y == 20);
        CHECK(map.slider_points[1].x == 70);
        CHECK(map.slider_points[1].y == 80);
    }
}

PPPP_TEST(slider_point_digit_widths) {
    static const int first_xs[] = {1, 12, 123, 1234};
    static const int first_ys[] = {5, 56, 567, 5678};
    static const int second_xs[] = {9, 98, 987, 9876};
    static const int second_ys[] = {4, 43, 432, 4321};
    for (int a = 0; a < 4; a++) {
        for (int b = 0; b < 4; b++) {
            for (int c = 0; c < 4; c++) {
                for (int d = 0; d < 4; d++) {
                    const std::string points = "B|" + to_str(first_xs[a]) + ":" + to_str(first_ys[b]) + "|" +
                                               to_str(second_xs[c]) + ":" + to_str(second_ys[d]) + ",1,10";
                    const Beatmap map = parse_str(slider_document(points));
                    CHECK(map.sliders.size() == 1u);
                    if (map.sliders.empty()) {
                        continue;
                    }
                    const Slider& slider = map.sliders[0];
                    CHECK(slider.point_count == 2u);
                    CHECK(map.slider_points[slider.point_begin].x == first_xs[a]);
                    CHECK(map.slider_points[slider.point_begin].y == first_ys[b]);
                    CHECK(map.slider_points[slider.point_begin + 1].x == second_xs[c]);
                    CHECK(map.slider_points[slider.point_begin + 1].y == second_ys[d]);
                }
            }
        }
    }
}

PPPP_TEST(slider_point_pairs_resume_after_fallback) {
    const std::string input =
        slider_document("B|123:456|789:123|12.5:7.5|123:456|789:123|123:45|678:90|1:2|3:4,1,10");
    static const float expected[9][2] = {
        {123, 456}, {789, 123}, {12, 7}, {123, 456}, {789, 123}, {123, 45}, {678, 90}, {1, 2}, {3, 4},
    };
    const Beatmap map = parse_str(input);
    CHECK(map.sliders.size() == 1u);
    CHECK(map.slider_points.size() == 9u);
    if (map.slider_points.size() == 9) {
        for (size_t i = 0; i < 9; ++i) {
            CHECK(map.slider_points[i].x == expected[i][0]);
            CHECK(map.slider_points[i].y == expected[i][1]);
        }
    }
}

PPPP_TEST(slider_repeats_and_length) {
    struct Case {
        const char* tail;
        fosu_int32 slides;
        double length;
    };
    static const Case cases[] = {
        {",1", 1, 0},     {",1,10", 1, 10}, {", 12 , 1e2 ", 12, 100}, {",9000,131072", 9000, 131072},
        {",-1,-5", 1, 0},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const Beatmap map = parse_str(slider_document("B|1:2" + std::string(cases[i].tail)));
        CHECK_MSG(map.sliders.size() == 1u, cases[i].tail);
        if (map.sliders.empty()) {
            continue;
        }
        CHECK_MSG(map.sliders[0].slides == cases[i].slides, cases[i].tail);
        CHECK_MSG(map.sliders[0].length == cases[i].length, cases[i].tail);
    }
    static const char* const malformed[] = {"",          ",",          ",1,",        ",9001,10",
                                            ",1,131073", ",1,10,,/:0", ",1,10,,,/:0"};
    for (size_t i = 0; i < 7; i++) {
        const Beatmap map = parse_str(slider_document("B|1:2" + std::string(malformed[i])));
        CHECK_MSG(map.hit_objects.empty(), malformed[i]);
        CHECK_MSG(map.stats.malformed_lines == 1u, malformed[i]);
    }
}

PPPP_TEST(slider_sounds) {
    // Extra columns after the hit sample do not belong to the Slider or HitObject models.
    static const size_t sizes[] = {0, 21, 22, 23, 80};
    for (size_t i = 0; i < 5; i++) {
        const std::string edge_sounds(sizes[i], '0');
        const Beatmap map =
            parse_str(slider_document("B|1:2,2,10," + edge_sounds + ",0:0|0:0|0:0,1:2,ignored"));
        CHECK(map.sliders.size() == 1u);
        if (map.sliders.empty()) {
            continue;
        }
        CHECK(map.sliders[0].edge_sounds == edge_sounds);
        CHECK(map.sliders[0].edge_sets == StringView("0:0|0:0|0:0"));
        CHECK(map.hit_objects[0].hit_sample == StringView("1:2"));
    }
}

PPPP_TEST(hitobject_details) {
    struct Case {
        fosu_uint32 type;
        const char* text;
        double end_time;
        const char* sample;
    };
    static const Case cases[] = {
        {1, "", 0, ""},
        {3, ",0:0:0:0:", 0, "0:0:0:0:"}, // Circle wins over slider.
        {8, ",12.5,0:0:0:0:x,ignored", 12.5, "0:0:0:0:x"},
        {128, "", 10, ""},
        {128, ",", 10, ""},
        {128, ",12.5:0:0:0:0:", 12.5, "0:0:0:0:"},
        {128, ",12:0:0:0:0:", 12, "0:0:0:0:"},
        {128, ",000012:0:0:0:0:", 12, "0:0:0:0:"},
        {128, ",2147483647:0:0:0:0:", 2147483647, "0:0:0:0:"},
        {128, ",12.5,ignored", 12.5, ""},
        {136, ",12.5,0:0", 12.5, "0:0"}, // Spinner wins over hold.
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        FileBuffer input;
        CHECK(make_padded(StringView(cases[i].text), input));
        const char* begin = input.data;
        const char* end = input.data + input.size;
        const internal::HitObjectKind::Value kind = internal::classify_hitobject_kind(cases[i].type);
        if (kind == internal::HitObjectKind::Circle) {
            internal::CircleDetails details;
            CHECK_MSG(internal::parse_circle_details(begin, end, details), cases[i].text);
            CHECK_MSG(details.hit_sample == StringView(cases[i].sample), cases[i].text);
        } else if (kind == internal::HitObjectKind::Spinner) {
            internal::TimedHitObjectDetails details = {0, StringView()};
            CHECK_MSG(internal::parse_spinner_details(begin, end, details), cases[i].text);
            CHECK_MSG(details.end_time == cases[i].end_time, cases[i].text);
            CHECK_MSG(details.hit_sample == StringView(cases[i].sample), cases[i].text);
        } else {
            internal::TimedHitObjectDetails details = {0, StringView()};
            CHECK_MSG(internal::parse_hold_details(10, begin, end, details), cases[i].text);
            CHECK_MSG(details.end_time == cases[i].end_time, cases[i].text);
            CHECK_MSG(details.hit_sample == StringView(cases[i].sample), cases[i].text);
        }
    }
    static const char* const bad_spinners[] = {"", ",", ",bad", ",12:0:0", ",12,/:0"};
    for (size_t i = 0; i < 5; i++) {
        FileBuffer input;
        CHECK(make_padded(StringView(bad_spinners[i]), input));
        internal::TimedHitObjectDetails details = {0, StringView()};
        CHECK_MSG(!internal::parse_spinner_details(input.data, input.data + input.size, details),
                  bad_spinners[i]);
    }
    static const char* const bad_holds[] = {",2147483648:0:0:0:0:", ",12x:0:0:0:0:"};
    for (size_t i = 0; i < 2; i++) {
        FileBuffer input;
        CHECK(make_padded(StringView(bad_holds[i]), input));
        internal::TimedHitObjectDetails details = {0, StringView()};
        CHECK_MSG(!internal::parse_hold_details(10, input.data, input.data + input.size, details),
                  bad_holds[i]);
    }
}

PPPP_TEST_MAIN()
