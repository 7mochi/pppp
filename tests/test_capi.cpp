#include "common.h"
#include "maps.h"
#include "parity.h"
#include <doctest.h>
#include <fstream>
#include <iterator>
#include <pppp/capi.h>
#include <string>
#include <vector>

namespace {
    typedef PpppErrorCode (*StrainGetter)(PpppStrains*, double*, PpppInt32*);

    void check_strain_series(StrainGetter getter, PpppStrains* result, const std::vector<double>& expected) {
        PpppInt32 size = 0;
        CHECK(getter(result, 0, &size) == Pppp_BufferSizeQuery);
        CHECK(static_cast<size_t>(size) == expected.size());
        if (!expected.empty()) {
            std::vector<double> values(size);
            CHECK(getter(result, &values[0], &size) == Pppp_Success);
            CHECK(values == expected);
        }
    }

    bool absent(double expected) { return expected != expected; }

    void check_optional(const PpppOptionalDouble& value, double expected) {
        if (absent(expected)) {
            CHECK(value.has_value == 0);
        } else {
            CHECK(value.has_value == 1);
            CHECK(value.value == pppp_test::pp_approx(expected));
        }
    }

    void check_osu_performance(const PpppOsuPerformanceAttributes& pp, const parity::Row& expected) {
        CHECK(pp.total == pppp_test::pp_approx(expected.number("total")));
        CHECK(pp.aim == pppp_test::pp_approx(expected.number("aim")));
        CHECK(pp.speed == pppp_test::pp_approx(expected.number("speed")));
        CHECK(pp.accuracy == pppp_test::pp_approx(expected.number("accuracy")));
        CHECK(pp.flashlight == pppp_test::pp_approx(expected.number("flashlight")));
        CHECK(pp.reading == pppp_test::pp_approx(expected.number("reading")));
        CHECK(pp.effective_miss_count == pppp_test::pp_approx(expected.number("effective_miss_count")));
        CHECK(pp.combo_based_estimated_miss_count ==
              pppp_test::pp_approx(expected.number("combo_based_estimated_miss_count")));
        check_optional(pp.score_based_estimated_miss_count,
                       expected.number("score_based_estimated_miss_count"));
        CHECK(pp.aim_estimated_slider_breaks ==
              pppp_test::pp_approx(expected.number("aim_estimated_slider_breaks")));
        CHECK(pp.speed_estimated_slider_breaks ==
              pppp_test::pp_approx(expected.number("speed_estimated_slider_breaks")));
        check_optional(pp.speed_deviation, expected.number("speed_deviation"));
    }

    void check_osu_performance_row(const parity::Row& row) {
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map_name = row.text("map");
        const char* mods_name = row.text("mods");
        INFO(name << " " << map_name << " " << mods_name);
        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + row.text("map");

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(path.c_str(), &map) == Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, row.text("mods")) == Pppp_Success);

        PpppNativeOsuDifficultyCalculator difficulty_calculator;
        REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &difficulty_calculator) ==
                Pppp_Success);
        PpppOsuDifficultyAttributes difficulty;
        REQUIRE(Pppp_OsuDifficultyCalculator_Calculate(difficulty_calculator.handle, mods.handle,
                                                       &difficulty) == Pppp_Success);

        PpppNativeScoreInfo score = {};
        score.ruleset = ruleset.handle;
        score.beatmap = map.handle;
        score.mods = mods.handle;
        score.count_great = given.integer("great");
        score.count_ok = given.integer("ok");
        score.count_meh = given.integer("meh");
        score.count_miss = given.integer("miss");
        score.count_slider_tail_hit = difficulty.slider_count;
        score.max_combo = given.integer("combo");
        score.accuracy = given.number("accuracy") < 0.0
                             ? pppp_test::classic_accuracy(given.integer("great"), given.integer("ok"),
                                                           given.integer("meh"), given.integer("miss"))
                             : given.number("accuracy");
        if (given.integer("legacy_total_score") != 0) {
            score.legacy_total_score.has_value = 1;
            score.legacy_total_score.value = given.integer("legacy_total_score");
        }

        PpppNativeOsuPerformanceCalculator calculator;
        REQUIRE(Pppp_OsuPerformanceCalculator_Create(&calculator) == Pppp_Success);
        PpppOsuPerformanceAttributes with = {};
        PpppOsuPerformanceAttributes without = {};
        REQUIRE(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &with) ==
                Pppp_Success);
        REQUIRE(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, 0, &without) ==
                Pppp_Success);

        check_osu_performance(with, expected);
        check_osu_performance(without, expected);

        Pppp_OsuPerformanceCalculator_Destroy(calculator.handle);
        Pppp_OsuDifficultyCalculator_Destroy(difficulty_calculator.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    void check_taiko_performance_row(const parity::Row& row) {
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map_name = row.text("map");
        const char* mods_name = row.text("mods");
        INFO(name << " " << map_name << " " << mods_name);
        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + row.text("map");

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(path.c_str(), &map) == Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(1, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, row.text("mods")) == Pppp_Success);

        PpppNativeTaikoDifficultyCalculator difficulty_calculator;
        REQUIRE(Pppp_TaikoDifficultyCalculator_Create(ruleset.handle, map.handle, &difficulty_calculator) ==
                Pppp_Success);
        PpppTaikoDifficultyAttributes difficulty;
        REQUIRE(Pppp_TaikoDifficultyCalculator_Calculate(difficulty_calculator.handle, mods.handle,
                                                         &difficulty) == Pppp_Success);

        const int great = given.integer("great");
        const int ok = given.integer("ok");
        const int miss = given.integer("miss");
        PpppNativeScoreInfo score = {};
        score.ruleset = ruleset.handle;
        score.beatmap = map.handle;
        score.mods = mods.handle;
        score.count_great = great;
        score.count_ok = ok;
        score.count_miss = miss;
        score.max_combo = given.integer("combo") != 0 ? given.integer("combo") : difficulty.max_combo;
        score.accuracy = (great * 300.0 + ok * 150.0) / ((great + ok + miss) * 300.0);

        PpppNativeTaikoPerformanceCalculator calculator;
        REQUIRE(Pppp_TaikoPerformanceCalculator_Create(&calculator) == Pppp_Success);
        PpppTaikoPerformanceAttributes with = {};
        PpppTaikoPerformanceAttributes without = {};
        REQUIRE(Pppp_TaikoPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &with) ==
                Pppp_Success);
        REQUIRE(Pppp_TaikoPerformanceCalculator_Calculate(calculator.handle, &score, 0, &without) ==
                Pppp_Success);

        CHECK(with.total == pppp_test::pp_approx(expected.number("total")));
        CHECK(with.difficulty == pppp_test::pp_approx(expected.number("difficulty")));
        CHECK(with.accuracy == pppp_test::pp_approx(expected.number("accuracy")));
        check_optional(with.estimated_unstable_rate, expected.number("estimated_unstable_rate"));
        CHECK(without.total == pppp_test::pp_approx(expected.number("total")));
        CHECK(without.difficulty == pppp_test::pp_approx(expected.number("difficulty")));
        CHECK(without.accuracy == pppp_test::pp_approx(expected.number("accuracy")));
        check_optional(without.estimated_unstable_rate, expected.number("estimated_unstable_rate"));

        Pppp_TaikoPerformanceCalculator_Destroy(calculator.handle);
        Pppp_TaikoDifficultyCalculator_Destroy(difficulty_calculator.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    void check_catch_performance_row(const parity::Row& row) {
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map_name = row.text("map");
        const char* mods_name = row.text("mods");
        INFO(name << " " << map_name << " " << mods_name);
        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + row.text("map");

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(path.c_str(), &map) == Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(2, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, row.text("mods")) == Pppp_Success);

        PpppNativeCatchDifficultyCalculator difficulty_calculator;
        REQUIRE(Pppp_CatchDifficultyCalculator_Create(ruleset.handle, map.handle, &difficulty_calculator) ==
                Pppp_Success);
        PpppCatchDifficultyAttributes difficulty;
        REQUIRE(Pppp_CatchDifficultyCalculator_Calculate(difficulty_calculator.handle, mods.handle,
                                                         &difficulty) == Pppp_Success);

        const int fruits = given.integer("fruits");
        const int droplets = given.integer("droplets");
        const int tiny_droplets = given.integer("tiny_droplets");
        const int tiny_misses = given.integer("tiny_misses");
        const int miss = given.integer("miss");
        PpppNativeScoreInfo score = {};
        score.ruleset = ruleset.handle;
        score.beatmap = map.handle;
        score.mods = mods.handle;
        score.count_great = fruits;
        score.count_large_tick_hit = droplets;
        score.count_small_tick_hit = tiny_droplets;
        score.count_small_tick_miss = tiny_misses;
        score.count_miss = miss;
        score.max_combo = given.integer("combo") != 0 ? given.integer("combo") : difficulty.max_combo;
        score.accuracy = static_cast<double>(fruits + droplets + tiny_droplets) /
                         (fruits + droplets + tiny_droplets + tiny_misses + miss);

        PpppNativeCatchPerformanceCalculator calculator;
        REQUIRE(Pppp_CatchPerformanceCalculator_Create(&calculator) == Pppp_Success);
        PpppCatchPerformanceAttributes with = {};
        PpppCatchPerformanceAttributes without = {};
        REQUIRE(Pppp_CatchPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &with) ==
                Pppp_Success);
        REQUIRE(Pppp_CatchPerformanceCalculator_Calculate(calculator.handle, &score, 0, &without) ==
                Pppp_Success);

        CHECK(with.total == pppp_test::pp_approx(expected.number("total")));
        CHECK(without.total == pppp_test::pp_approx(expected.number("total")));

        Pppp_CatchPerformanceCalculator_Destroy(calculator.handle);
        Pppp_CatchDifficultyCalculator_Destroy(difficulty_calculator.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    void check_mania_performance_row(const parity::Row& row) {
        const parity::Row given = row.object("score");
        const parity::Row expected = row.object("attributes");
        const char* name = row.text("name");
        const char* map_name = row.text("map");
        const char* mods_name = row.text("mods");
        INFO(name << " " << map_name << " " << mods_name);
        const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + row.text("map");

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(path.c_str(), &map) == Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(3, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, row.text("mods")) == Pppp_Success);

        PpppNativeManiaDifficultyCalculator difficulty_calculator;
        REQUIRE(Pppp_ManiaDifficultyCalculator_Create(ruleset.handle, map.handle, &difficulty_calculator) ==
                Pppp_Success);
        PpppManiaDifficultyAttributes difficulty;
        REQUIRE(Pppp_ManiaDifficultyCalculator_Calculate(difficulty_calculator.handle, mods.handle,
                                                         &difficulty) == Pppp_Success);

        const int perfect = given.integer("perfect");
        const int great = given.integer("great");
        const int good = given.integer("good");
        const int ok = given.integer("ok");
        const int meh = given.integer("meh");
        const int miss = given.integer("miss");
        PpppNativeScoreInfo score = {};
        score.ruleset = ruleset.handle;
        score.beatmap = map.handle;
        score.mods = mods.handle;
        score.count_perfect = perfect;
        score.count_great = great;
        score.count_good = good;
        score.count_ok = ok;
        score.count_meh = meh;
        score.count_miss = miss;
        score.max_combo = given.integer("combo") != 0 ? given.integer("combo") : difficulty.max_combo;
        score.accuracy = (perfect * 305.0 + great * 300.0 + good * 200.0 + ok * 100.0 + meh * 50.0) /
                         ((perfect + great + good + ok + meh + miss) * 305.0);

        PpppNativeManiaPerformanceCalculator calculator;
        REQUIRE(Pppp_ManiaPerformanceCalculator_Create(&calculator) == Pppp_Success);
        PpppManiaPerformanceAttributes with = {};
        PpppManiaPerformanceAttributes without = {};
        REQUIRE(Pppp_ManiaPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &with) ==
                Pppp_Success);
        REQUIRE(Pppp_ManiaPerformanceCalculator_Calculate(calculator.handle, &score, 0, &without) ==
                Pppp_Success);

        CHECK(with.total == pppp_test::pp_approx(expected.number("total")));
        CHECK(with.difficulty == pppp_test::pp_approx(expected.number("difficulty")));
        CHECK(without.total == pppp_test::pp_approx(expected.number("total")));
        CHECK(without.difficulty == pppp_test::pp_approx(expected.number("difficulty")));

        Pppp_ManiaPerformanceCalculator_Destroy(calculator.handle);
        Pppp_ManiaDifficultyCalculator_Destroy(difficulty_calculator.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }
} // namespace

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

TEST_CASE("the C ABI timed result matches the C++ calculator") {
    pppp::Beatmap source;
    pppp_test::load(source, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    const std::vector<pppp::TimedDifficultyAttributes> expected =
        pppp_test::calculate_timed(pppp::Difficulty().mods(pppp_test::parse_mods("HD,DT")), source);

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/2785319.osu", &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, "HD,DT") == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

    PpppTimedResult* result = 0;
    REQUIRE(Pppp_OsuDifficultyCalculator_CalculateTimed(calc.handle, mods.handle, &result) == Pppp_Success);

    PpppInt32 count = 0;
    CHECK(Pppp_TimedResult_GetCount(result, &count) == Pppp_Success);
    CHECK(static_cast<size_t>(count) == expected.size());

    PpppInt32 size = 0;
    CHECK(Pppp_TimedResult_GetOsu(result, 0, &size) == Pppp_BufferSizeQuery);
    CHECK(size == count);
    std::vector<PpppTimedOsuDifficultyAttributes> buffer(size);
    CHECK(Pppp_TimedResult_GetOsu(result, &buffer[0], &size) == Pppp_Success);
    CHECK(buffer[0].time == expected[0].time);
    CHECK(buffer[0].attributes.star_rating ==
          pppp_test::difficulty_approx(expected[0].attributes.star_rating()));
    CHECK(buffer.back().attributes.max_combo == expected.back().attributes.max_combo());

    CHECK(Pppp_TimedResult_GetTaiko(result, 0, &size) == Pppp_UnexpectedRuleset);
    Pppp_TimedResult_Destroy(result);
    Pppp_TimedResult_Destroy(0);
    Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI timed result is empty for a beatmap without hit objects") {
    const char* text = "osu file format v14\n[General]\nMode:0\n";

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromText(text, &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

    PpppTimedResult* result = 0;
    REQUIRE(Pppp_OsuDifficultyCalculator_CalculateTimed(calc.handle, 0, &result) == Pppp_Success);

    PpppInt32 count = -1;
    CHECK(Pppp_TimedResult_GetCount(result, &count) == Pppp_Success);
    CHECK(count == 0);

    PpppInt32 size = -1;
    CHECK(Pppp_TimedResult_GetOsu(result, 0, &size) == Pppp_BufferSizeQuery);
    CHECK(size == 0);

    Pppp_TimedResult_Destroy(result);
    Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI timed result exposes the other rulesets") {
    {
        pppp::Beatmap source;
        pppp_test::load(source, PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu");
        const std::vector<pppp::TimedDifficultyAttributes> expected =
            pppp_test::calculate_timed(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), source);

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", &map) ==
                Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(1, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, "DT") == Pppp_Success);
        PpppNativeTaikoDifficultyCalculator calc;
        REQUIRE(Pppp_TaikoDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

        PpppTimedResult* result = 0;
        REQUIRE(Pppp_TaikoDifficultyCalculator_CalculateTimed(calc.handle, mods.handle, &result) ==
                Pppp_Success);

        PpppInt32 size = 0;
        CHECK(Pppp_TimedResult_GetTaiko(result, 0, &size) == Pppp_BufferSizeQuery);
        REQUIRE(static_cast<size_t>(size) == expected.size());
        std::vector<PpppTimedTaikoDifficultyAttributes> buffer(size);
        CHECK(Pppp_TimedResult_GetTaiko(result, &buffer[0], &size) == Pppp_Success);
        CHECK(buffer.back().time == expected.back().time);
        CHECK(buffer.back().attributes.star_rating ==
              pppp_test::difficulty_approx(expected.back().attributes.star_rating()));
        CHECK(buffer.back().attributes.max_combo == expected.back().attributes.max_combo());
        CHECK(Pppp_TimedResult_GetOsu(result, 0, &size) == Pppp_UnexpectedRuleset);

        Pppp_TimedResult_Destroy(result);
        Pppp_TaikoDifficultyCalculator_Destroy(calc.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    {
        pppp::Beatmap source;
        pppp_test::load(source, PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu");
        const std::vector<pppp::TimedDifficultyAttributes> expected =
            pppp_test::calculate_timed(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), source);

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", &map) ==
                Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(2, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, "DT") == Pppp_Success);
        PpppNativeCatchDifficultyCalculator calc;
        REQUIRE(Pppp_CatchDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

        PpppTimedResult* result = 0;
        REQUIRE(Pppp_CatchDifficultyCalculator_CalculateTimed(calc.handle, mods.handle, &result) ==
                Pppp_Success);

        PpppInt32 size = 0;
        CHECK(Pppp_TimedResult_GetCatch(result, 0, &size) == Pppp_BufferSizeQuery);
        REQUIRE(static_cast<size_t>(size) == expected.size());
        std::vector<PpppTimedCatchDifficultyAttributes> buffer(size);
        CHECK(Pppp_TimedResult_GetCatch(result, &buffer[0], &size) == Pppp_Success);
        CHECK(buffer.back().time == expected.back().time);
        CHECK(buffer.back().attributes.star_rating ==
              pppp_test::difficulty_approx(expected.back().attributes.star_rating()));
        CHECK(buffer.back().attributes.max_combo == expected.back().attributes.max_combo());
        CHECK(Pppp_TimedResult_GetTaiko(result, 0, &size) == Pppp_UnexpectedRuleset);

        Pppp_TimedResult_Destroy(result);
        Pppp_CatchDifficultyCalculator_Destroy(calc.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    {
        pppp::Beatmap source;
        pppp_test::load(source, PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu");
        const std::vector<pppp::TimedDifficultyAttributes> expected =
            pppp_test::calculate_timed(pppp::Difficulty(), source);

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", &map) ==
                Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(3, &ruleset) == Pppp_Success);
        PpppNativeManiaDifficultyCalculator calc;
        REQUIRE(Pppp_ManiaDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

        PpppTimedResult* result = 0;
        REQUIRE(Pppp_ManiaDifficultyCalculator_CalculateTimed(calc.handle, 0, &result) == Pppp_Success);

        PpppInt32 size = 0;
        CHECK(Pppp_TimedResult_GetMania(result, 0, &size) == Pppp_BufferSizeQuery);
        REQUIRE(static_cast<size_t>(size) == expected.size());
        std::vector<PpppTimedManiaDifficultyAttributes> buffer(size);
        CHECK(Pppp_TimedResult_GetMania(result, &buffer[0], &size) == Pppp_Success);
        CHECK(buffer.back().time == expected.back().time);
        CHECK(buffer.back().attributes.star_rating ==
              pppp_test::difficulty_approx(expected.back().attributes.star_rating()));
        CHECK(buffer.back().attributes.max_combo == expected.back().attributes.max_combo());
        CHECK(Pppp_TimedResult_GetCatch(result, 0, &size) == Pppp_UnexpectedRuleset);

        Pppp_TimedResult_Destroy(result);
        Pppp_ManiaDifficultyCalculator_Destroy(calc.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }
}

TEST_CASE("the C ABI strains match the C++ calculator") {
    pppp::Beatmap source;
    pppp_test::load(source, PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu");
    const pppp::Strains expected =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("HD,FL")), source);
    REQUIRE(expected.osu());

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, "HD,FL") == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

    PpppStrains* result = 0;
    REQUIRE(Pppp_OsuDifficultyCalculator_CalculateStrains(calc.handle, mods.handle, &result) == Pppp_Success);

    PpppInt32 ruleset_id = -1;
    CHECK(Pppp_Strains_GetRuleset(result, &ruleset_id) == Pppp_Success);
    CHECK(ruleset_id == 0);
    double start = 0.0;
    double section = 0.0;
    CHECK(Pppp_Strains_GetStartTime(result, &start) == Pppp_Success);
    CHECK(Pppp_Strains_GetSectionLength(result, &section) == Pppp_Success);
    CHECK(start == expected.start_time());
    CHECK(section == expected.section_length());

    check_strain_series(Pppp_Strains_GetOsuAim, result, expected.osu()->aim);
    check_strain_series(Pppp_Strains_GetOsuAimNoSliders, result, expected.osu()->aim_no_sliders);
    check_strain_series(Pppp_Strains_GetOsuSpeed, result, expected.osu()->speed);
    check_strain_series(Pppp_Strains_GetOsuReading, result, expected.osu()->reading);
    check_strain_series(Pppp_Strains_GetOsuFlashlight, result, expected.osu()->flashlight);

    PpppInt32 size = 0;
    CHECK(Pppp_Strains_GetTaikoColour(result, 0, &size) == Pppp_UnexpectedRuleset);
    Pppp_Strains_Destroy(result);
    Pppp_Strains_Destroy(0);
    Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI strains expose an empty flashlight without FL") {
    pppp::Beatmap source;
    pppp_test::load(source, PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu");
    const pppp::Strains expected =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("HD")), source);
    REQUIRE(expected.osu());
    REQUIRE(expected.osu()->flashlight.empty());

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, "HD") == Pppp_Success);
    PpppNativeOsuDifficultyCalculator calc;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

    PpppStrains* result = 0;
    REQUIRE(Pppp_OsuDifficultyCalculator_CalculateStrains(calc.handle, mods.handle, &result) == Pppp_Success);

    PpppInt32 size = -1;
    CHECK(Pppp_Strains_GetOsuFlashlight(result, 0, &size) == Pppp_BufferSizeQuery);
    CHECK(size == 0);

    Pppp_Strains_Destroy(result);
    Pppp_OsuDifficultyCalculator_Destroy(calc.handle);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI strains expose taiko, catch and mania") {
    {
        pppp::Beatmap source;
        pppp_test::load(source, PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu");
        const pppp::Strains expected =
            pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("HD,DT")), source);
        REQUIRE(expected.taiko());

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", &map) ==
                Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(1, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, "HD,DT") == Pppp_Success);
        PpppNativeTaikoDifficultyCalculator calc;
        REQUIRE(Pppp_TaikoDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

        PpppStrains* result = 0;
        REQUIRE(Pppp_TaikoDifficultyCalculator_CalculateStrains(calc.handle, mods.handle, &result) ==
                Pppp_Success);

        PpppInt32 ruleset_id = -1;
        CHECK(Pppp_Strains_GetRuleset(result, &ruleset_id) == Pppp_Success);
        CHECK(ruleset_id == 1);
        check_strain_series(Pppp_Strains_GetTaikoColour, result, expected.taiko()->colour);
        check_strain_series(Pppp_Strains_GetTaikoReading, result, expected.taiko()->reading);
        check_strain_series(Pppp_Strains_GetTaikoRhythm, result, expected.taiko()->rhythm);
        check_strain_series(Pppp_Strains_GetTaikoStamina, result, expected.taiko()->stamina);
        check_strain_series(Pppp_Strains_GetTaikoSingleColourStamina, result,
                            expected.taiko()->single_colour_stamina);

        PpppInt32 size = 0;
        CHECK(Pppp_Strains_GetOsuAim(result, 0, &size) == Pppp_UnexpectedRuleset);

        Pppp_Strains_Destroy(result);
        Pppp_TaikoDifficultyCalculator_Destroy(calc.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    {
        pppp::Beatmap source;
        pppp_test::load(source, PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu");
        const pppp::Strains expected =
            pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("HD,DT")), source);
        REQUIRE(expected.fruits());

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", &map) ==
                Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(2, &ruleset) == Pppp_Success);
        PpppNativeMods mods;
        REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
        REQUIRE(Pppp_Mods_Parse(mods.handle, "HD,DT") == Pppp_Success);
        PpppNativeCatchDifficultyCalculator calc;
        REQUIRE(Pppp_CatchDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

        PpppStrains* result = 0;
        REQUIRE(Pppp_CatchDifficultyCalculator_CalculateStrains(calc.handle, mods.handle, &result) ==
                Pppp_Success);

        PpppInt32 ruleset_id = -1;
        CHECK(Pppp_Strains_GetRuleset(result, &ruleset_id) == Pppp_Success);
        CHECK(ruleset_id == 2);
        check_strain_series(Pppp_Strains_GetCatchMovement, result, expected.fruits()->movement);

        PpppInt32 size = 0;
        CHECK(Pppp_Strains_GetTaikoColour(result, 0, &size) == Pppp_UnexpectedRuleset);

        Pppp_Strains_Destroy(result);
        Pppp_CatchDifficultyCalculator_Destroy(calc.handle);
        Pppp_Mods_Destroy(mods.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }

    {
        pppp::Beatmap source;
        pppp_test::load(source, PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu");
        const pppp::Strains expected = pppp_test::strains(pppp::Difficulty(), source);
        REQUIRE(expected.mania());

        PpppNativeBeatmap map;
        REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", &map) ==
                Pppp_Success);
        PpppNativeRuleset ruleset;
        REQUIRE(Pppp_Ruleset_CreateFromId(3, &ruleset) == Pppp_Success);
        PpppNativeManiaDifficultyCalculator calc;
        REQUIRE(Pppp_ManiaDifficultyCalculator_Create(ruleset.handle, map.handle, &calc) == Pppp_Success);

        PpppStrains* result = 0;
        REQUIRE(Pppp_ManiaDifficultyCalculator_CalculateStrains(calc.handle, 0, &result) == Pppp_Success);

        PpppInt32 ruleset_id = -1;
        CHECK(Pppp_Strains_GetRuleset(result, &ruleset_id) == Pppp_Success);
        CHECK(ruleset_id == 3);
        check_strain_series(Pppp_Strains_GetManiaStrain, result, expected.mania()->strain);

        PpppInt32 size = 0;
        CHECK(Pppp_Strains_GetCatchMovement(result, 0, &size) == Pppp_UnexpectedRuleset);

        Pppp_Strains_Destroy(result);
        Pppp_ManiaDifficultyCalculator_Destroy(calc.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(map.handle);
    }
}

TEST_CASE("the C ABI performance matches parity with and without difficulty") {
    const parity::Table osu("performance", "osu");
    REQUIRE(osu.size() > 0);
    for (int i = 0; i < osu.size(); i++) {
        check_osu_performance_row(osu[i]);
    }

    const parity::Table taiko("performance", "taiko");
    REQUIRE(taiko.size() > 0);
    for (int i = 0; i < taiko.size(); i++) {
        check_taiko_performance_row(taiko[i]);
    }

    const parity::Table fruits("performance", "catch");
    REQUIRE(fruits.size() > 0);
    for (int i = 0; i < fruits.size(); i++) {
        check_catch_performance_row(fruits[i]);
    }

    const parity::Table mania("performance", "mania");
    REQUIRE(mania.size() > 0);
    for (int i = 0; i < mania.size(); i++) {
        check_mania_performance_row(mania[i]);
    }
}

TEST_CASE("the C ABI performance rejects null and wrong-ruleset inputs") {
    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(PPPP_TEST_RESOURCES "/osu/2785319.osu", &map) == Pppp_Success);
    PpppNativeRuleset osu;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &osu) == Pppp_Success);
    PpppNativeRuleset taiko;
    REQUIRE(Pppp_Ruleset_CreateFromId(1, &taiko) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);

    PpppNativeOsuPerformanceCalculator calculator;
    REQUIRE(Pppp_OsuPerformanceCalculator_Create(&calculator) == Pppp_Success);

    PpppNativeScoreInfo score = {};
    score.ruleset = osu.handle;
    score.beatmap = map.handle;
    score.mods = mods.handle;

    PpppOsuPerformanceAttributes out = {};
    CHECK(Pppp_OsuPerformanceCalculator_Calculate(0, &score, 0, &out) == Pppp_InvalidArgument);
    CHECK(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, 0, 0, &out) == Pppp_InvalidArgument);
    CHECK(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, 0, 0) == Pppp_InvalidArgument);

    score.ruleset = 0;
    CHECK(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, 0, &out) ==
          Pppp_InvalidArgument);
    score.ruleset = osu.handle;
    score.beatmap = 0;
    CHECK(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, 0, &out) ==
          Pppp_InvalidArgument);
    score.beatmap = map.handle;

    score.ruleset = taiko.handle;
    CHECK(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, 0, &out) ==
          Pppp_UnexpectedRuleset);

    CHECK(Pppp_OsuPerformanceCalculator_Create(0) == Pppp_InvalidArgument);

    Pppp_OsuPerformanceCalculator_Destroy(calculator.handle);
    Pppp_OsuPerformanceCalculator_Destroy(0);
    Pppp_TaikoPerformanceCalculator_Destroy(0);
    Pppp_CatchPerformanceCalculator_Destroy(0);
    Pppp_ManiaPerformanceCalculator_Destroy(0);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(taiko.handle);
    Pppp_Ruleset_Destroy(osu.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI performance honours the optional legacy total score") {
    const parity::Table rows("performance", "osu");
    int legacy_index = -1;
    for (int i = 0; i < rows.size(); i++) {
        const parity::Row given = rows[i].object("score");
        const parity::Row expected = rows[i].object("attributes");
        if (given.integer("legacy_total_score") != 0 &&
            !absent(expected.number("score_based_estimated_miss_count"))) {
            legacy_index = i;
            break;
        }
    }
    REQUIRE(legacy_index >= 0);

    const parity::Row legacy = rows[legacy_index];
    const parity::Row given = legacy.object("score");
    const parity::Row expected = legacy.object("attributes");
    const char* name = legacy.text("name");
    const char* map_name = legacy.text("map");
    const char* mods_name = legacy.text("mods");
    INFO(name << " " << map_name << " " << mods_name);
    const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + legacy.text("map");

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(path.c_str(), &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, legacy.text("mods")) == Pppp_Success);

    PpppNativeOsuDifficultyCalculator difficulty_calculator;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &difficulty_calculator) ==
            Pppp_Success);
    PpppOsuDifficultyAttributes difficulty;
    REQUIRE(Pppp_OsuDifficultyCalculator_Calculate(difficulty_calculator.handle, mods.handle, &difficulty) ==
            Pppp_Success);

    PpppNativeScoreInfo score = {};
    score.ruleset = ruleset.handle;
    score.beatmap = map.handle;
    score.mods = mods.handle;
    score.count_great = given.integer("great");
    score.count_ok = given.integer("ok");
    score.count_meh = given.integer("meh");
    score.count_miss = given.integer("miss");
    score.count_slider_tail_hit = difficulty.slider_count;
    score.max_combo = given.integer("combo");
    score.accuracy = given.number("accuracy") < 0.0
                         ? pppp_test::classic_accuracy(given.integer("great"), given.integer("ok"),
                                                       given.integer("meh"), given.integer("miss"))
                         : given.number("accuracy");

    PpppNativeOsuPerformanceCalculator calculator;
    REQUIRE(Pppp_OsuPerformanceCalculator_Create(&calculator) == Pppp_Success);

    PpppOsuPerformanceAttributes with_legacy = {};
    score.legacy_total_score.has_value = 1;
    score.legacy_total_score.value = given.integer("legacy_total_score");
    REQUIRE(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &with_legacy) ==
            Pppp_Success);
    CHECK(with_legacy.score_based_estimated_miss_count.has_value == 1);
    CHECK(with_legacy.score_based_estimated_miss_count.value ==
          pppp_test::pp_approx(expected.number("score_based_estimated_miss_count")));

    PpppOsuPerformanceAttributes without_legacy = {};
    score.legacy_total_score.has_value = 0;
    score.legacy_total_score.value = 0;
    REQUIRE(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty,
                                                    &without_legacy) == Pppp_Success);
    CHECK(without_legacy.score_based_estimated_miss_count.has_value == 0);

    Pppp_OsuPerformanceCalculator_Destroy(calculator.handle);
    Pppp_OsuDifficultyCalculator_Destroy(difficulty_calculator.handle);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}

TEST_CASE("the C ABI performance treats a null mods handle as empty") {
    const parity::Table rows("performance", "osu");
    const parity::Row row = rows[0];
    const parity::Row given = row.object("score");
    const parity::Row expected = row.object("attributes");
    const char* name = row.text("name");
    const char* map_name = row.text("map");
    const char* mods_name = row.text("mods");
    INFO(name << " " << map_name << " " << mods_name);
    const std::string path = std::string(PPPP_TEST_RESOURCES) + "/" + row.text("map");

    PpppNativeBeatmap map;
    REQUIRE(Pppp_Beatmap_CreateFromFile(path.c_str(), &map) == Pppp_Success);
    PpppNativeRuleset ruleset;
    REQUIRE(Pppp_Ruleset_CreateFromId(0, &ruleset) == Pppp_Success);
    PpppNativeMods mods;
    REQUIRE(Pppp_Mods_Create(&mods) == Pppp_Success);
    REQUIRE(Pppp_Mods_Parse(mods.handle, row.text("mods")) == Pppp_Success);

    PpppNativeOsuDifficultyCalculator difficulty_calculator;
    REQUIRE(Pppp_OsuDifficultyCalculator_Create(ruleset.handle, map.handle, &difficulty_calculator) ==
            Pppp_Success);
    PpppOsuDifficultyAttributes difficulty;
    REQUIRE(Pppp_OsuDifficultyCalculator_Calculate(difficulty_calculator.handle, mods.handle, &difficulty) ==
            Pppp_Success);

    PpppNativeScoreInfo score = {};
    score.ruleset = ruleset.handle;
    score.beatmap = map.handle;
    score.mods = mods.handle;
    score.count_great = given.integer("great");
    score.count_ok = given.integer("ok");
    score.count_meh = given.integer("meh");
    score.count_miss = given.integer("miss");
    score.count_slider_tail_hit = difficulty.slider_count;
    score.max_combo = given.integer("combo");
    score.accuracy = given.number("accuracy") < 0.0
                         ? pppp_test::classic_accuracy(given.integer("great"), given.integer("ok"),
                                                       given.integer("meh"), given.integer("miss"))
                         : given.number("accuracy");

    PpppNativeOsuPerformanceCalculator calculator;
    REQUIRE(Pppp_OsuPerformanceCalculator_Create(&calculator) == Pppp_Success);

    PpppOsuPerformanceAttributes with_mods = {};
    REQUIRE(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &with_mods) ==
            Pppp_Success);

    score.mods = 0;
    PpppOsuPerformanceAttributes without_mods = {};
    REQUIRE(Pppp_OsuPerformanceCalculator_Calculate(calculator.handle, &score, &difficulty, &without_mods) ==
            Pppp_Success);

    CHECK(with_mods.total == pppp_test::pp_approx(expected.number("total")));
    CHECK(without_mods.total == pppp_test::pp_approx(expected.number("total")));
    check_osu_performance(with_mods, expected);
    check_osu_performance(without_mods, expected);

    Pppp_OsuPerformanceCalculator_Destroy(calculator.handle);
    Pppp_OsuDifficultyCalculator_Destroy(difficulty_calculator.handle);
    Pppp_Mods_Destroy(mods.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(map.handle);
}
