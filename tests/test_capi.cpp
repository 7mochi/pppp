#include "common.h"
#include "maps.h"
#include "parity.h"
#include <doctest.h>
#include <fstream>
#include <iterator>
#include <pppp/capi.h>
#include <string>
#include <vector>

TEST_CASE("the C ABI reports the header version") {
    CHECK(std::string(Pppp_Version()) == PPPP_VERSION_STRING);
    CHECK(Pppp_VersionNumber() == PPPP_VERSION_NUMBER);
    CHECK(Pppp_VersionNumber() / 1000000 == PPPP_VERSION_MAJOR);
}

TEST_CASE("the C ABI reports a backend name") {
    const std::string backend(Pppp_Backend());
    CHECK((backend == "scalar" || backend == "avx2" || backend == "neon"));
}

TEST_CASE("the C ABI beatmap exposes metadata and settings") {
    PpppNativeBeatmap native;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/2785319.osu", &native) == Pppp_Success);
    CHECK_FALSE(Pppp_ErrorHandler_GetLastMessage());
    CHECK(native.ruleset_id == 0);
    CHECK(native.beatmap_id == 2785319);
    CHECK(native.format_version == 14);
    CHECK(native.approach_rate > 0.0f);
    CHECK(native.stack_leniency > 0.0f);

    PpppInt32 size = 0;
    CHECK(Pppp_Beatmap_GetTitle(native.handle, 0, &size) == Pppp_BufferSizeQuery);
    std::vector<char> text(size);
    CHECK(Pppp_Beatmap_GetTitle(native.handle, &text[0], &size) == Pppp_Success);
    CHECK(std::string(&text[0]) == "re[in]flaw");

    char short_title[2] = {1, 1};
    PpppInt32 short_size = 2;
    CHECK(Pppp_Beatmap_GetTitle(native.handle, short_title, &short_size) == Pppp_Success);
    CHECK(short_title[1] == 0);

    Pppp_Beatmap_Destroy(native.handle);
    Pppp_Beatmap_Destroy(0);
}

TEST_CASE("the C ABI returns an empty title when the map has none") {
    const char* text = "osu file format v14\n[General]\nMode:0\n";
    PpppNativeBeatmap native;
    REQUIRE(Pppp_Beatmap_CreateFromText(text, &native) == Pppp_Success);

    PpppInt32 size = 0;
    CHECK(Pppp_Beatmap_GetTitle(native.handle, 0, &size) == Pppp_BufferSizeQuery);
    CHECK(size == 1);
    char title[1] = {1};
    CHECK(Pppp_Beatmap_GetTitle(native.handle, title, &size) == Pppp_Success);
    CHECK(title[0] == 0);

    Pppp_Beatmap_Destroy(native.handle);
}

TEST_CASE("the C ABI rejects a missing file and a null beatmap") {
    PpppNativeBeatmap native;
    CHECK(Pppp_Beatmap_CreateFromFile("/no/such/map.osu", &native) == Pppp_Parse);
    const char* message = Pppp_ErrorHandler_GetLastMessage();
    REQUIRE(message);
    CHECK(std::string(message).find("parse") != std::string::npos);
    CHECK(Pppp_Beatmap_CreateFromFile(0, &native) == Pppp_InvalidArgument);
}

TEST_CASE("the C ABI ruleset exposes its id and short name") {
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    CHECK(ruleset.ruleset_id == 0);

    PpppInt32 size = 0;
    CHECK(Pppp_Ruleset_GetShortName(ruleset.handle, 0, &size) == Pppp_BufferSizeQuery);
    std::vector<char> name(size);
    CHECK(Pppp_Ruleset_GetShortName(ruleset.handle, &name[0], &size) == Pppp_Success);
    CHECK(std::string(&name[0]) == "osu");

    CHECK(Pppp_Ruleset_CreateFromId(4, &ruleset) == Pppp_RulesetUnavailable);
    CHECK(Pppp_Ruleset_CreateFromShortName("taiko", &ruleset) == Pppp_Success);
    CHECK(ruleset.ruleset_id == 1);

    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Ruleset_Destroy(0);
}

TEST_CASE("the C ABI mods object parses, reads and converts") {
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, "HD,DT:speed_change=1.4") == Pppp_Success);

    PpppInt32 count = 0;
    CHECK(Pppp_Mods_GetCount(mods.handle, &count) == Pppp_Success);
    CHECK(count == 2);

    PpppBool has = 0;
    CHECK(Pppp_Mods_Contains(mods.handle, "HD", &has) == Pppp_Success);
    CHECK(has == 1);
    CHECK(Pppp_Mods_Contains(mods.handle, "HR", &has) == Pppp_Success);
    CHECK(has == 0);

    double rate = 0.0;
    CHECK(Pppp_Mods_GetClockRate(mods.handle, &rate) == Pppp_Success);
    CHECK(rate == doctest::Approx(1.4).epsilon(1e-12));

    PpppInt32 size = 0;
    CHECK(Pppp_Mods_GetAcronym(mods.handle, 0, 0, &size) == Pppp_BufferSizeQuery);
    std::vector<char> acronym(size);
    CHECK(Pppp_Mods_GetAcronym(mods.handle, 0, &acronym[0], &size) == Pppp_Success);
    CHECK(std::string(&acronym[0]) == "HD");

    PpppUint32 bits = 0;
    CHECK(Pppp_Mods_GetLegacyBits(mods.handle, 0, &bits) == Pppp_Success);
    CHECK(bits == 72u); /* HD | DT */

    CHECK(Pppp_Mods_FromLegacy(mods.handle, 0x48u, 0) == Pppp_Success);
    CHECK(Pppp_Mods_AddClassic(mods.handle) == Pppp_Success);
    CHECK(Pppp_Mods_Contains(mods.handle, "CL", &has) == Pppp_Success);
    CHECK(has == 1);

    CHECK(Pppp_Mods_Parse(mods.handle, "bad") == Pppp_InvalidArgument);
    CHECK(Pppp_Mods_FromLegacy(mods.handle, 0, 4) == Pppp_InvalidArgument);
    CHECK(Pppp_Mods_GetAcronym(mods.handle, 99, 0, &size) == Pppp_InvalidArgument);

    Pppp_Mods_Destroy(mods.handle);
    Pppp_Mods_Destroy(0);
}

TEST_CASE("the C ABI loads the same beatmap from bytes and text") {
    std::ifstream file(PPPP_TEST_RESOURCES "/osu/2785319.osu", std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    PpppNativeBeatmap from_bytes;
    REQUIRE(Pppp_Beatmap_CreateFromBytes(&bytes[0], bytes.size(), &from_bytes) == Pppp_Success);
    CHECK(from_bytes.beatmap_id == 2785319);
    Pppp_Beatmap_Destroy(from_bytes.handle);

    PpppNativeBeatmap from_text;
    REQUIRE(Pppp_Beatmap_CreateFromText(bytes.c_str(), &from_text) == Pppp_Success);
    CHECK(from_text.beatmap_id == 2785319);
    Pppp_Beatmap_Destroy(from_text.handle);
}

TEST_CASE("the C ABI difficulty matches parity for all rulesets") {
    for (int ruleset_id = 0; ruleset_id < 4; ruleset_id++) {
        const char* name = ruleset_id == 0   ? "osu"
                           : ruleset_id == 1 ? "taiko"
                           : ruleset_id == 2 ? "catch"
                                             : "mania";
        const parity::Table rows("difficulty", name);
        for (int i = 0; i < rows.size(); i++) {
            const parity::Row row = rows[i];
            const parity::Row expected = row.object("attributes");

            PpppNativeBeatmap map;
            REQUIRE(Pppp_Beatmap_CreateFromFile(
                        (std::string(PPPP_TEST_RESOURCES) + "/" + row.text("map")).c_str(), &map) ==
                    Pppp_Success);
            PpppNativeRuleset ruleset;
            REQUIRE(Pppp_Ruleset_CreateFromId(ruleset_id, &ruleset) == Pppp_Success);
            PpppNativeMods mods;
            REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
            REQUIRE(Pppp_Mods_Parse(mods.handle, row.text("mods")) == Pppp_Success);

            if (ruleset_id == 0) {
                PpppNativeOsuDifficultyCalculator calc;
                REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) ==
                        Pppp_Success);
                PpppOsuDifficultyAttributes attrs;
                REQUIRE(Pppp_OsuDifficultyCalculator_Calculate(calc.handle, mods.handle, &attrs) ==
                        Pppp_Success);
                CHECK(attrs.star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
                CHECK(attrs.max_combo == expected.integer("max_combo"));
                CHECK(attrs.aim_difficulty ==
                      pppp_test::difficulty_approx(expected.number("aim_difficulty")));
                CHECK(attrs.speed_difficulty ==
                      pppp_test::difficulty_approx(expected.number("speed_difficulty")));
                Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
            } else if (ruleset_id == 1) {
                PpppNativeTaikoDifficultyCalculator calc;
                REQUIRE(Pppp_TaikoDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) ==
                        Pppp_Success);
                PpppTaikoDifficultyAttributes attrs;
                REQUIRE(Pppp_TaikoDifficultyCalculator_Calculate(calc.handle, mods.handle, &attrs) ==
                        Pppp_Success);
                CHECK(attrs.star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
                CHECK(attrs.max_combo == expected.integer("max_combo"));
                CHECK(attrs.mechanical_difficulty ==
                      pppp_test::difficulty_approx(expected.number("mechanical_difficulty")));
                CHECK(attrs.rhythm_difficulty ==
                      pppp_test::difficulty_approx(expected.number("rhythm_difficulty")));
                CHECK(attrs.reading_difficulty ==
                      pppp_test::difficulty_approx(expected.number("reading_difficulty")));
                CHECK(attrs.colour_difficulty ==
                      pppp_test::difficulty_approx(expected.number("colour_difficulty")));
                CHECK(attrs.stamina_difficulty ==
                      pppp_test::difficulty_approx(expected.number("stamina_difficulty")));
                CHECK(attrs.mono_stamina_factor ==
                      pppp_test::difficulty_approx(expected.number("mono_stamina_factor")));
                CHECK(attrs.consistency_factor ==
                      pppp_test::difficulty_approx(expected.number("consistency_factor")));
                CHECK(attrs.stamina_top_strains ==
                      pppp_test::difficulty_approx(expected.number("stamina_top_strains")));
                Pppp_TaikoDifficultyCalculator_Destroy(calc.handle);
            } else if (ruleset_id == 2) {
                PpppNativeCatchDifficultyCalculator calc;
                REQUIRE(Pppp_CatchDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) ==
                        Pppp_Success);
                PpppCatchDifficultyAttributes attrs;
                REQUIRE(Pppp_CatchDifficultyCalculator_Calculate(calc.handle, mods.handle, &attrs) ==
                        Pppp_Success);
                CHECK(attrs.star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
                CHECK(attrs.max_combo == expected.integer("max_combo"));
                Pppp_CatchDifficultyCalculator_Destroy(calc.handle);
            } else {
                PpppNativeManiaDifficultyCalculator calc;
                REQUIRE(Pppp_ManiaDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) ==
                        Pppp_Success);
                PpppManiaDifficultyAttributes attrs;
                REQUIRE(Pppp_ManiaDifficultyCalculator_Calculate(calc.handle, mods.handle, &attrs) ==
                        Pppp_Success);
                CHECK(attrs.star_rating == pppp_test::difficulty_approx(expected.number("star_rating")));
                CHECK(attrs.max_combo == expected.integer("max_combo"));
                Pppp_ManiaDifficultyCalculator_Destroy(calc.handle);
            }

            Pppp_Mods_Destroy(mods.handle);
            Pppp_Ruleset_Destroy(ruleset.handle);
            Pppp_Beatmap_Destroy(map.handle);
        }
    }
}

TEST_CASE("the C ABI calculator applies and clears the clock rate") {
    pppp::Beatmap source;
    pppp_test::load(source, PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu");
    const pppp::Mods dt = pppp_test::parse_mods("DT");
    const pppp::DifficultyAttributes with_rate =
        pppp_test::calculate(pppp::Difficulty().mods(dt).clock_rate(1.5), source);
    const pppp::DifficultyAttributes without_rate = pppp_test::calculate(pppp::Difficulty().mods(dt), source);

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, "DT") == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

    PpppOsuDifficultyAttributes attrs;
    REQUIRE(Pppp_OsuDifficultyCalculator_SetClockRate(calc.handle, 1.5) == Pppp_Success);
    REQUIRE(Pppp_OsuDifficultyCalculator_Calculate(calc.handle, mods.handle, &attrs) == Pppp_Success);
    CHECK(attrs.star_rating == pppp_test::difficulty_approx(with_rate.star_rating()));

    REQUIRE(Pppp_OsuDifficultyCalculator_ClearClockRate(calc.handle) == Pppp_Success);
    REQUIRE(Pppp_OsuDifficultyCalculator_Calculate(calc.handle, mods.handle, &attrs) == Pppp_Success);
    CHECK(attrs.star_rating == pppp_test::difficulty_approx(without_rate.star_rating()));

    Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI calculator rejects another ruleset") {
    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == Pppp_Success);
    PpppNativeRuleset taiko;
    REQUIRE(Pppp_Ruleset_CreateFromId(1, &taiko) == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    CHECK(Pppp_OsuDifficultyCalculator_Create(taiko.handle, map.handle, &calc) == Pppp_UnexpectedRuleset);
    Pppp_Ruleset_Destroy(taiko.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI calculator keeps the beatmap alive") {
    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

    Pppp_Beatmap_Destroy(map.handle);
    PpppOsuDifficultyAttributes attrs;
    CHECK(Pppp_OsuDifficultyCalculator_Calculate(calc.handle, 0, &attrs) == Pppp_Success);
    Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
    Pppp_OsuDifficultyCalculator_Destroy(0);
    Pppp_Ruleset_Destroy(ruleset.handle);
}
