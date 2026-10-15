#include "common.h"
#include "parity.h"

#include <cstddef>
#include <doctest.h>
#include <fosu/parser.h>
#include <string>

TEST_CASE("the decoder matches upstream over the curated maps") {
    const parity::Table rows("decode");
    REQUIRE(rows.size() > 0);

    for (int i = 0; i < rows.size(); i++) {
        const parity::Row row = rows[i];
        const parity::Row expected = row.object("decode");
        const char* map = row.text("map");
        INFO(map);

        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + map;
        fosu::Parser parser;
        fosu::ParseOptions options;
        options.client = fosu::Client::Lazer;
        const fosu::Beatmap* parsed = parser.parse_file(path.c_str(), options);
        REQUIRE_UNARY(parsed != NULL);
        const fosu::Beatmap& beatmap = *parsed;

        CHECK(static_cast<int>(beatmap.format_version) == expected.integer("format"));
        CHECK(static_cast<int>(beatmap.mode) == expected.integer("mode"));
        CHECK(beatmap.ar == pppp_test::difficulty_approx(expected.number("ar")));
        CHECK(beatmap.od == pppp_test::difficulty_approx(expected.number("od")));
        CHECK(beatmap.cs == pppp_test::difficulty_approx(expected.number("cs")));
        CHECK(beatmap.hp == pppp_test::difficulty_approx(expected.number("hp")));
        CHECK(beatmap.slider_multiplier ==
              pppp_test::difficulty_approx(expected.number("slider_multiplier")));
        CHECK(beatmap.slider_tick_rate == pppp_test::difficulty_approx(expected.number("slider_tick_rate")));
        CHECK(static_cast<int>(beatmap.hit_objects.size()) == expected.integer("hit_objects"));

        int uninherited = 0;
        for (size_t point = 0; point < beatmap.timing_points.size(); point++) {
            if (beatmap.timing_points[point].uninherited != 0) {
                uninherited++;
            }
        }
        CHECK(uninherited == expected.integer("uninherited_points"));

        CHECK(static_cast<int>(beatmap.breaks.size()) == expected.integer("breaks"));
        CHECK(beatmap.stack_leniency == pppp_test::difficulty_approx(expected.number("stack_leniency")));
    }
}
