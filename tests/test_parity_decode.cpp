#include "common.h"
#include "parity/decode.h"

#include <cstddef>
#include <doctest.h>
#include <fosu/parser.h>
#include <string>

TEST_CASE("the decoder matches upstream over the curated maps") {
    const size_t rows = sizeof(parity::decode_rows) / sizeof(parity::decode_rows[0]);
    REQUIRE(rows > 0);

    for (size_t i = 0; i < rows; i++) {
        const parity::DecodeRow& row = parity::decode_rows[i];
        INFO(row.map);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + row.map;
        fosu::Parser parser;
        fosu::ParseOptions options;
        options.client = fosu::Client::Lazer;
        const fosu::Beatmap* parsed = parser.parse_file(path.c_str(), options);
        REQUIRE_UNARY(parsed != NULL);
        const fosu::Beatmap& beatmap = *parsed;

        CHECK(static_cast<int>(beatmap.format_version) == row.format);
        CHECK(static_cast<int>(beatmap.mode) == row.mode);
        CHECK(beatmap.ar == pppp_test::difficulty_approx(row.ar));
        CHECK(beatmap.od == pppp_test::difficulty_approx(row.od));
        CHECK(beatmap.cs == pppp_test::difficulty_approx(row.cs));
        CHECK(beatmap.hp == pppp_test::difficulty_approx(row.hp));
        CHECK(beatmap.slider_multiplier == pppp_test::difficulty_approx(row.slider_multiplier));
        CHECK(beatmap.slider_tick_rate == pppp_test::difficulty_approx(row.slider_tick_rate));
        CHECK(static_cast<int>(beatmap.hit_objects.size()) == row.hit_objects);

        int uninherited = 0;
        for (size_t point = 0; point < beatmap.timing_points.size(); point++) {
            if (beatmap.timing_points[point].uninherited != 0) {
                uninherited++;
            }
        }
        CHECK(uninherited == row.uninherited_points);

        CHECK(static_cast<int>(beatmap.breaks.size()) == row.breaks);
        CHECK(beatmap.stack_leniency == pppp_test::difficulty_approx(row.stack_leniency));
    }
}
