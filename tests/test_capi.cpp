#include <cstring>
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
