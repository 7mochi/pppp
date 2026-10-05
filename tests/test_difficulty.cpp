// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "common.h"
#include "maps.h"
#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/osu_difficulty_calculator.h"
#include "pppp/performance.h"
#include "pppp/taiko/difficulty/taiko_difficulty_calculator.h"
#include <cstddef>
#include <doctest.h>
#include <vector>

namespace {
    struct OsuRow {
        const char* map;
        const char* mods;
        double star_rating;
        int max_combo;
        double aim_difficulty;
        double aim_difficult_slider_count;
        double speed_difficulty;
        double speed_note_count;
        double flashlight_difficulty;
        double reading_difficulty;
        double slider_factor;
        double aim_top_weighted_slider_factor;
        double speed_top_weighted_slider_factor;
        double aim_difficult_strain_count;
        double speed_difficult_strain_count;
        double reading_difficult_note_count;
        int hit_circle_count;
        int slider_count;
        int spinner_count;
        double nested_score_per_object;
        double legacy_score_base_multiplier;
        double maximum_legacy_combo_score;
    };

    struct TaikoRow {
        const char* map;
        const char* mods;
        double star_rating;
        int max_combo;
        double mechanical_difficulty;
        double rhythm_difficulty;
        double reading_difficulty;
        double colour_difficulty;
        double stamina_difficulty;
        double mono_stamina_factor;
        double consistency_factor;
        double stamina_top_strains;
    };

    struct StarsRow {
        const char* map;
        const char* mods;
        double star_rating;
        int max_combo;
    };

    void check(const OsuRow& row) {
        pppp::Beatmap beatmap;

        INFO(row.map);
        INFO(row.mods);
        pppp_test::load(beatmap, row.map);

        const pppp::Mods mods = pppp_test::parse_mods(row.mods);
        const pppp::DifficultyAttributes tagged =
            pppp_test::calculate(pppp::Difficulty().mods(mods).ruleset(pppp::Ruleset::OSU), beatmap);
        const pppp::osu::difficulty::OsuDifficultyAttributes& attrs = *tagged.osu();

        CHECK(attrs.star_rating == pppp_test::difficulty_approx(row.star_rating));
        CHECK(attrs.max_combo == row.max_combo);
        CHECK(attrs.aim_difficulty == pppp_test::difficulty_approx(row.aim_difficulty));
        CHECK(attrs.aim_difficult_slider_count ==
              pppp_test::difficulty_approx(row.aim_difficult_slider_count));
        CHECK(attrs.speed_difficulty == pppp_test::difficulty_approx(row.speed_difficulty));
        CHECK(attrs.speed_note_count == pppp_test::difficulty_approx(row.speed_note_count));
        CHECK(attrs.flashlight_difficulty == pppp_test::difficulty_approx(row.flashlight_difficulty));
        CHECK(attrs.reading_difficulty == pppp_test::difficulty_approx(row.reading_difficulty));
        CHECK(attrs.slider_factor == pppp_test::difficulty_approx(row.slider_factor));
        CHECK(attrs.aim_top_weighted_slider_factor ==
              pppp_test::difficulty_approx(row.aim_top_weighted_slider_factor));
        CHECK(attrs.speed_top_weighted_slider_factor ==
              pppp_test::difficulty_approx(row.speed_top_weighted_slider_factor));
        CHECK(attrs.aim_difficult_strain_count ==
              pppp_test::difficulty_approx(row.aim_difficult_strain_count));
        CHECK(attrs.speed_difficult_strain_count ==
              pppp_test::difficulty_approx(row.speed_difficult_strain_count));
        CHECK(attrs.reading_difficult_note_count ==
              pppp_test::difficulty_approx(row.reading_difficult_note_count));
        CHECK(attrs.hit_circle_count == row.hit_circle_count);
        CHECK(attrs.slider_count == row.slider_count);
        CHECK(attrs.spinner_count == row.spinner_count);
        CHECK(attrs.nested_score_per_object == pppp_test::difficulty_approx(row.nested_score_per_object));
        CHECK(attrs.legacy_score_base_multiplier ==
              pppp_test::difficulty_approx(row.legacy_score_base_multiplier));
        CHECK(attrs.maximum_legacy_combo_score ==
              pppp_test::difficulty_approx(row.maximum_legacy_combo_score));
    }

    void check(const TaikoRow& row) {
        pppp::Beatmap beatmap;

        INFO(row.map);
        INFO(row.mods);
        pppp_test::load(beatmap, row.map);

        const pppp::Mods mods = pppp_test::parse_mods(row.mods);
        const pppp::DifficultyAttributes tagged =
            pppp_test::calculate(pppp::Difficulty().mods(mods).ruleset(pppp::Ruleset::TAIKO), beatmap);
        const pppp::taiko::difficulty::TaikoDifficultyAttributes& attrs = *tagged.taiko();

        CHECK(attrs.star_rating == pppp_test::difficulty_approx(row.star_rating));
        CHECK(attrs.max_combo == row.max_combo);
        CHECK(attrs.mechanical_difficulty == pppp_test::difficulty_approx(row.mechanical_difficulty));
        CHECK(attrs.rhythm_difficulty == pppp_test::difficulty_approx(row.rhythm_difficulty));
        CHECK(attrs.reading_difficulty == pppp_test::difficulty_approx(row.reading_difficulty));
        CHECK(attrs.colour_difficulty == pppp_test::difficulty_approx(row.colour_difficulty));
        CHECK(attrs.stamina_difficulty == pppp_test::difficulty_approx(row.stamina_difficulty));
        CHECK(attrs.mono_stamina_factor == pppp_test::difficulty_approx(row.mono_stamina_factor));
        CHECK(attrs.consistency_factor == pppp_test::difficulty_approx(row.consistency_factor));
        CHECK(attrs.stamina_top_strains == pppp_test::difficulty_approx(row.stamina_top_strains));
    }

    /// Platform-dependent math functions (Pow, Cbrt, Exp, etc) may result in minute differences.
    const double CHECK_PRECISION = 0.00001;

    enum Ruleset { OSU, TAIKO, CATCH, MANIA };

    pppp::DifficultyAttributes calculate(Ruleset ruleset, const StarsRow& row) {
        pppp::Beatmap beatmap;

        pppp_test::load(beatmap, row.map);

        const pppp::Mods mods = pppp_test::parse_mods(row.mods);
        const pppp::DifficultyAttributes attrs = pppp_test::calculate(
            pppp::Difficulty().mods(mods).ruleset(static_cast<pppp::Ruleset::Value>(ruleset)), beatmap);

        REQUIRE(attrs.ruleset() == static_cast<pppp::Ruleset::Value>(ruleset));
        return attrs;
    }

    void check_expected(Ruleset ruleset, const StarsRow& row) {
        INFO(row.map);
        INFO(row.mods);

        const pppp::DifficultyAttributes attrs = calculate(ruleset, row);

        CHECK(pppp_test::distance(attrs.star_rating(), row.star_rating) <= CHECK_PRECISION);
        CHECK(attrs.max_combo() == row.max_combo);
    }

    void check_common(Ruleset ruleset, const StarsRow& row) {
        INFO(row.map);
        INFO(row.mods);

        const pppp::DifficultyAttributes attrs = calculate(ruleset, row);

        CHECK(attrs.star_rating() == pppp_test::difficulty_approx(row.star_rating));
        CHECK(attrs.max_combo() == row.max_combo);
    }
} // namespace

TEST_SUITE("OsuDifficultyCalculatorTest") {
    TEST_CASE("Test") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "NM", 6.5243170265483581, 239},
            {PPPP_TEST_RESOURCES "/osu/zero-length-sliders.osu", "NM", 1.3280410795791415, 54},
            {PPPP_TEST_RESOURCES "/osu/very-fast-slider.osu", "NM", 0.40867325147697559, 4},
            {PPPP_TEST_RESOURCES "/osu/nan-slider.osu", "NM", 0.87058175794353554, 6},
            {PPPP_TEST_RESOURCES "/osu/801165.osu", "NM", 6.3059767387139756, 2359},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(OSU, rows[i]);
        }
    }

    TEST_CASE("TestClockRateAdjusted") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "DT", 9.4677607900646308, 239},
            {PPPP_TEST_RESOURCES "/osu/zero-length-sliders.osu", "DT", 1.6856612715618886, 54},
            {PPPP_TEST_RESOURCES "/osu/very-fast-slider.osu", "DT", 0.53588473186572561, 4},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(OSU, rows[i]);
        }
    }

    TEST_CASE("TestClassicMod") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "CL", 6.5243170265483581, 239},
            {PPPP_TEST_RESOURCES "/osu/zero-length-sliders.osu", "CL", 1.3280410795791415, 54},
            {PPPP_TEST_RESOURCES "/osu/very-fast-slider.osu", "CL", 0.40867325147697559, 4},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(OSU, rows[i]);
        }
    }
}

TEST_SUITE("TaikoDifficultyCalculatorTest") {
    TEST_CASE("Test") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", "NM", 3.3190848563395079, 200},
            {PPPP_TEST_RESOURCES "/taiko/diffcalc-test-strong.osu", "NM", 3.3190848563395079, 200},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(TAIKO, rows[i]);
        }
    }

    TEST_CASE("TestClockRateAdjusted") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", "DT", 4.4551414906554987, 200},
            {PPPP_TEST_RESOURCES "/taiko/diffcalc-test-strong.osu", "DT", 4.4551414906554987, 200},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(TAIKO, rows[i]);
        }
    }
}

TEST_SUITE("CatchDifficultyCalculatorTest") {
    TEST_CASE("Test") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", "NM", 4.039861734717169, 127},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(CATCH, rows[i]);
        }
    }

    TEST_CASE("TestClockRateAdjusted") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", "DT", 5.1527173897800873, 127},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(CATCH, rows[i]);
        }
    }
}

TEST_SUITE("ManiaDifficultyCalculatorTest") {
    TEST_CASE("Test") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", "NM", 2.3493769750220914, 242},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(MANIA, rows[i]);
        }
    }

    TEST_CASE("TestClockRateAdjusted") {
        static const StarsRow rows[] = {
            {PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", "DT", 2.797245912537965, 242},
        };

        for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
            check_expected(MANIA, rows[i]);
        }
    }
}

TEST_CASE("basic_osu") {
    static const OsuRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "NM",
         6.004027372552197,
         909,
         3.27863857424994,
         192.5269999738169,
         2.4917265153109014,
         183.0639785973236,
         0,
         0.8229208521405954,
         0.963038689276571,
         1.524202370856421,
         0.536191641231213,
         124.69544446818438,
         81.74921671931915,
         34.92251595856365,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "HD",
         6.308336834596954,
         909,
         3.27863857424994,
         192.5269999738169,
         2.4917265153109014,
         183.0639785973236,
         0,
         2.1827264342501156,
         0.963038689276571,
         1.524202370856421,
         0.536191641231213,
         124.69544446818438,
         81.74921671931915,
         135.7746987103988,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "HR",
         6.713673504226164,
         909,
         3.799232322249643,
         191.8309507640488,
         2.4917265153109014,
         183.0639785973236,
         0,
         0.9144658746351508,
         0.9475983634088616,
         1.510078933241033,
         0.536191641231213,
         119.62860170592188,
         81.74921671931915,
         38.427842331296304,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "DT",
         8.762958976840826,
         909,
         4.693556954514378,
         207.97415619378023,
         3.674242476685813,
         211.45478779956713,
         0,
         2.0228172499241897,
         0.9674908735064722,
         1.476888448482664,
         0.6387657108874909,
         144.31095827870723,
         86.60969041721593,
         190.90786995621218,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "FL",
         7.036258413665859,
         909,
         3.27863857424994,
         192.5269999738169,
         2.4917265153109014,
         183.0639785973236,
         2.345608737345168,
         0.8229208521405954,
         0.963038689276571,
         1.524202370856421,
         0.536191641231213,
         124.69544446818438,
         81.74921671931915,
         34.92251595856365,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "HD,EZ",
         6.891768477382506,
         909,
         2.7625488821040367,
         196.89037873007746,
         2.3995360680924946,
         192.2649456376246,
         0,
         3.5538685094268883,
         0.9796909646357417,
         1.562739917685155,
         0.5369556982593612,
         129.38126835796155,
         84.40439036292067,
         124.55280093376417,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu",
         "HD,FL",
         7.47402836368438,
         909,
         3.27863857424994,
         192.5269999738169,
         2.4917265153109014,
         183.0639785973236,
         2.6270894946667935,
         2.1827264342501156,
         0.963038689276571,
         1.524202370856421,
         0.536191641231213,
         124.69544446818438,
         81.74921671931915,
         135.7746987103988,
         307,
         293,
         1,
         34.991680532445926,
         5,
         15729840},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(rows[i]);
    }
}

TEST_CASE("basic_taiko") {
    static const TaikoRow rows[] = {
        {PPPP_TEST_RESOURCES "/taiko/1028484.osu", "NM", 2.9144215641617333, 289, 2.757718537279857,
         0.15668593055086977, 1.70963310068196E-05, 0.6655024155624444, 2.0922161217174127,
         2.585220903145618E-07, 0.6314855249538754, 59.377534920268296},
        {PPPP_TEST_RESOURCES "/taiko/1028484.osu", "HR", 2.9957310404617963, 289, 2.3526849330602797,
         0.134624894028623, 0.5084212133728931, 0.5677582700493313, 1.7849266630109486, 2.585220903145618E-07,
         0.6322607099238484, 59.377534920268296},
        {PPPP_TEST_RESOURCES "/taiko/1028484.osu", "DT", 4.051329399964723, 289, 3.2898979490906894,
         0.570672991636439, 0.19075845923759416, 0.7218921897845048, 2.568005759306185, 2.465693827167051E-07,
         0.6202990548778025, 63.34988960849165},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(rows[i]);
    }
}

TEST_CASE("convert_taiko") {
    static const TaikoRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "NM", 4.752572620626138, 908, 3.068521344030717,
         0.6028550725402863, 1.0811962040551348, 0.8456679526084883, 2.2228533914222286,
         0.0014311041774359668, 0.6875624057993176, 189.58797576615262},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "HR", 5.275959467694453, 908, 3.1896721231569485,
         0.640061492397074, 1.4462258521404312, 0.8790564547089896, 2.3106156684479586, 0.0014311041774359668,
         0.679047690894666, 189.58797576615262},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "DT", 7.116103934899722, 908, 4.350893046373321,
         0.9584853559461861, 1.8067255325802154, 1.0979910141819806, 3.252902032191341, 0.0014418086037955797,
         0.6748291725050156, 233.31840028238065},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check(rows[i]);
    }
}

TEST_CASE("osu!catch keeps a juice stream's ticks where osu! drops them") {
    static const StarsRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/nan-slider.osu", "NM", 1.831445790658667, 13},
        {PPPP_TEST_RESOURCES "/osu/nan-slider.osu", "DT", 1.89801005149296, 13},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check_common(CATCH, rows[i]);
    }
}

TEST_CASE("basic_catch") {
    static const StarsRow rows[] = {
        {PPPP_TEST_RESOURCES "/fruits/2118524.osu", "NM", 3.2340182503279706, 730},
        {PPPP_TEST_RESOURCES "/fruits/2118524.osu", "HR", 4.308291009137178, 730},
        {PPPP_TEST_RESOURCES "/fruits/2118524.osu", "EZ", 4.059198145823293, 730},
        {PPPP_TEST_RESOURCES "/fruits/2118524.osu", "DT", 4.6192881825873275, 730},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check_common(CATCH, rows[i]);
    }
}

TEST_CASE("convert_catch") {
    static const StarsRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "NM", 4.526991300645072, 908},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "HR", 5.0738627744810545, 908},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "EZ", 3.590187752268528, 908},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "DT", 6.151552522578919, 908},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check_common(CATCH, rows[i]);
    }
}

TEST_CASE("basic_mania") {
    static const StarsRow rows[] = {
        {PPPP_TEST_RESOURCES "/mania/1638954.osu", "NM", 3.358304846842773, 956},
        {PPPP_TEST_RESOURCES "/mania/1638954.osu", "DT", 4.6072892053157295, 956},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check_common(MANIA, rows[i]);
    }
}

TEST_CASE("convert_mania") {
    static const StarsRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "NM", 3.2033142085672255, 1381},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "DT", 4.2934063021960185, 1381},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        check_common(MANIA, rows[i]);
    }
}

TEST_CASE("an empty beatmap has default attributes") {
    const pppp::beatmaps::Beatmap empty;
    pppp::osu::difficulty::OsuDifficultyAttributes attrs;

    REQUIRE(pppp::osu::difficulty::calculate_difficulty(attrs, empty, 0, 0).ok());
    CHECK(attrs.star_rating == 0.0);
    CHECK(attrs.slider_factor == 0.0);
}

TEST_CASE("every ruleset rejects a null mod list with a count") {
    const pppp::beatmaps::Beatmap empty;
    pppp::osu::difficulty::OsuDifficultyAttributes osu;
    pppp::taiko::difficulty::TaikoDifficultyAttributes taiko;
    pppp::fruits::difficulty::CatchDifficultyAttributes fruits;
    pppp::mania::difficulty::ManiaDifficultyAttributes mania;

    CHECK(pppp::osu::difficulty::calculate_difficulty(osu, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(pppp::taiko::difficulty::calculate_difficulty(taiko, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(pppp::fruits::difficulty::calculate_difficulty(fruits, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(pppp::mania::difficulty::calculate_difficulty(mania, empty, 0, 2).code() ==
          pppp::StatusCode::INVALID_ARGUMENT);
}

TEST_CASE("the facade reports a calculation that could not run") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    beatmap.slider_path.recompute = 0;

    const pppp::Mods mirror = pppp_test::parse_mods("MR");
    const pppp::Difficulty difficulty = pppp::Difficulty().mods(mirror);
    pppp::DifficultyAttributes attributes;
    std::vector<pppp::TimedDifficultyAttributes> timed;
    pppp::Strains strains;
    CHECK(difficulty.calculate(beatmap, attributes).code() == pppp::StatusCode::NO_SLIDER_PATH_BACKEND);
    CHECK(difficulty.calculate_timed(beatmap, timed).code() == pppp::StatusCode::NO_SLIDER_PATH_BACKEND);
    CHECK(difficulty.strains(beatmap, strains).code() == pppp::StatusCode::NO_SLIDER_PATH_BACKEND);

    pppp::ScoreInfo score;
    score.mods = mirror;
    pppp::PerformanceAttributes performance;
    CHECK(pppp::Performance(beatmap).score(score).calculate(performance).code() ==
          pppp::StatusCode::NO_SLIDER_PATH_BACKEND);

    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): the ruleset outside the four is the case.
    const pppp::Difficulty unknown = pppp::Difficulty().ruleset(static_cast<pppp::Ruleset::Value>(4));
    CHECK(unknown.calculate(beatmap, attributes).code() == pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(unknown.calculate_timed(beatmap, timed).code() == pppp::StatusCode::INVALID_ARGUMENT);
    CHECK(unknown.strains(beatmap, strains).code() == pppp::StatusCode::INVALID_ARGUMENT);
}

namespace {
    struct TimedRow {
        const char* map;
        const char* mods;
        int ruleset;
        size_t count;
        double middle_time;
        double middle_stars;
        int middle_combo;
        double last_time;
        double last_stars;
        int last_combo;
    };
} // namespace

TEST_CASE("the timed attributes match upstream's CalculateTimed") {
    static const TimedRow rows[] = {
        {PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu", "NM", 0, 124, 18000, 5.727266936277771, 63, 103000,
         6.524323005451468, 239},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "HD,DT", 0, 601, 61309.235290751734, 8.272126401328721, 433,
         115486.23529075173, 8.898179651893287, 909},
        {PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu", "DT", 1, 238, 21624, 4.397186455763557, 118, 53000,
         4.455142137225538, 200},
        {PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu", "DT", 2, 93, 14500, 4.27412411307246, 47, 45250,
         5.152717389780087, 127},
        {PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu", "NM", 3, 137, 16250, 1.8228476946125378, 69, 30500,
         2.3493769750220914, 242},
        {PPPP_TEST_RESOURCES "/osu/2785319.osu", "4K", 3, 838, 61662, 2.5718820820666117, 530, 115486,
         2.653795415351293, 1072},
    };

    for (size_t r = 0; r < sizeof(rows) / sizeof(rows[0]); r++) {
        const TimedRow& row = rows[r];
        INFO(row.map);
        INFO(row.mods);
        pppp::Beatmap beatmap;
        pppp_test::load(beatmap, row.map);

        const std::vector<pppp::TimedDifficultyAttributes> timed =
            pppp_test::calculate_timed(pppp::Difficulty()
                                           .mods(pppp_test::parse_mods(row.mods))
                                           .ruleset(static_cast<pppp::Ruleset::Value>(row.ruleset)),
                                       beatmap);

        REQUIRE(timed.size() == row.count);
        const pppp::TimedDifficultyAttributes& middle = timed[row.count / 2];
        const pppp::TimedDifficultyAttributes& last = timed[row.count - 1];
        CHECK(middle.time == row.middle_time);
        CHECK(middle.attributes.star_rating() == pppp_test::difficulty_approx(row.middle_stars));
        CHECK(middle.attributes.max_combo() == row.middle_combo);
        CHECK(last.time == row.last_time);
        CHECK(last.attributes.star_rating() == pppp_test::difficulty_approx(row.last_stars));
        CHECK(last.attributes.max_combo() == row.last_combo);
        CHECK(last.attributes.ruleset() == static_cast<pppp::Ruleset::Value>(row.ruleset));
    }
}

TEST_CASE("the first timed taiko entries keep upstream's NaN") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu");

    const std::vector<pppp::TimedDifficultyAttributes> timed =
        pppp_test::calculate_timed(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), beatmap);

    REQUIRE(!timed.empty());
    REQUIRE(timed[0].attributes.taiko());
    CHECK(timed[0].time == 0.0);
    CHECK(timed[0].attributes.star_rating() == 0.0);
    CHECK(timed[0].attributes.max_combo() == 1);
    const double mechanical = timed[0].attributes.taiko()->mechanical_difficulty;
    CHECK(mechanical != mechanical);
}

TEST_CASE("a beatmap without hit objects has no timed attributes") {
    const pppp::beatmaps::Beatmap empty;
    CHECK(pppp_test::calculate_timed(pppp::Difficulty(), empty).empty());
}

TEST_CASE("the osu! strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/diffcalc-test.osu");

    const pppp::Strains strains =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("HD,FL")), beatmap);
    REQUIRE(strains.osu());
    const pppp::OsuStrains& osu = *strains.osu();
    CHECK(strains.start_time() == 800.0);
    CHECK(strains.section_length() == 400.0);
    REQUIRE(osu.aim.size() == 254);
    REQUIRE(osu.aim_no_sliders.size() == 254);
    REQUIRE(osu.speed.size() == 254);
    REQUIRE(osu.reading.size() == 254);
    REQUIRE(osu.flashlight.size() == 254);
    CHECK(osu.aim[46] == pppp_test::difficulty_approx(522.4551066294925));
    CHECK(osu.aim_no_sliders[46] == pppp_test::difficulty_approx(522.4551066294925));
    CHECK(osu.speed[19] == pppp_test::difficulty_approx(62.17504943719141));
    CHECK(osu.reading[46] == pppp_test::difficulty_approx(268.69741068992795));
    CHECK(osu.reading[253] == pppp_test::difficulty_approx(12.417220542954208));
    CHECK(osu.flashlight[18] == pppp_test::difficulty_approx(41.02690817661656));

    pppp::Beatmap rated;
    pppp_test::load(rated, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    const pppp::Strains dt = pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), rated);
    REQUIRE(dt.osu());
    CHECK(dt.start_time() == 2400.0);
    CHECK(dt.section_length() == 600.0);
    REQUIRE(dt.osu()->aim.size() == 189);
    CHECK(dt.osu()->aim[161] == pppp_test::difficulty_approx(549.3785675344396));
    CHECK(dt.osu()->aim[188] == pppp_test::difficulty_approx(360.47840960759805));
    CHECK(dt.osu()->speed[130] == pppp_test::difficulty_approx(264.03121219093333));
    CHECK(dt.osu()->flashlight.empty());
}

TEST_CASE("the osu!taiko strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/diffcalc-test.osu");

    const pppp::Strains strains =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), beatmap);
    REQUIRE(strains.taiko());
    const pppp::TaikoStrains& taiko = *strains.taiko();
    CHECK(strains.start_time() == 0.0);
    CHECK(strains.section_length() == 600.0);
    REQUIRE(taiko.colour.size() == 89);
    REQUIRE(taiko.reading.size() == 89);
    REQUIRE(taiko.rhythm.size() == 89);
    REQUIRE(taiko.stamina.size() == 89);
    REQUIRE(taiko.single_colour_stamina.size() == 89);
    CHECK(taiko.colour[29] == pppp_test::difficulty_approx(2.604853902561832));
    CHECK(taiko.reading[32] == pppp_test::difficulty_approx(0.0001525556772378032));
    CHECK(taiko.rhythm[40] == pppp_test::difficulty_approx(0.13536813853766497));
    CHECK(taiko.stamina[32] == pppp_test::difficulty_approx(8.807433788330115));
    CHECK(taiko.single_colour_stamina[6] == pppp_test::difficulty_approx(4.473586847131765));
    CHECK(taiko.stamina[88] == pppp_test::difficulty_approx(1.324675633775491));
}

TEST_CASE("the osu!catch strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/fruits/diffcalc-test.osu");

    const pppp::Strains strains =
        pppp_test::strains(pppp::Difficulty().mods(pppp_test::parse_mods("DT")), beatmap);
    REQUIRE(strains.fruits());
    CHECK(strains.start_time() == 0.0);
    CHECK(strains.section_length() == 1125.0);
    REQUIRE(strains.fruits()->movement.size() == 41);
    CHECK(strains.fruits()->movement[11] == pppp_test::difficulty_approx(0.35301054964344125));
    CHECK(strains.fruits()->movement[40] == pppp_test::difficulty_approx(0.028384444121763497));
}

TEST_CASE("the osu!mania strain graph matches upstream_ref --strain-graph") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/mania/diffcalc-test.osu");

    const pppp::Strains strains = pppp_test::strains(pppp::Difficulty(), beatmap);
    REQUIRE(strains.mania());
    CHECK(strains.start_time() == 400.0);
    CHECK(strains.section_length() == 400.0);
    REQUIRE(strains.mania()->strain.size() == 76);
    CHECK(strains.mania()->strain[70] == pppp_test::difficulty_approx(16.8859847732691));
    CHECK(strains.mania()->strain[75] == pppp_test::difficulty_approx(16.363963256682354));

    pppp::Beatmap convert;
    pppp_test::load(convert, PPPP_TEST_RESOURCES "/osu/2785319.osu");
    const pppp::Strains keys = pppp_test::strains(
        pppp::Difficulty().mods(pppp_test::parse_mods("4K")).ruleset(pppp::Ruleset::MANIA), convert);
    REQUIRE(keys.mania());
    CHECK(keys.start_time() == 2800.0);
    REQUIRE(keys.mania()->strain.size() == 282);
    CHECK(keys.mania()->strain[195] == pppp_test::difficulty_approx(15.725755965531633));
}

TEST_CASE("a beatmap without hit objects has upstream's strain graph") {
    const pppp::beatmaps::Beatmap empty;

    const pppp::Strains osu = pppp_test::strains(pppp::Difficulty(), empty);
    REQUIRE(osu.osu());
    CHECK(osu.section_length() == 400.0);
    CHECK(osu.osu()->aim.empty());

    const pppp::Strains taiko = pppp_test::strains(pppp::Difficulty().ruleset(pppp::Ruleset::TAIKO), empty);
    REQUIRE(taiko.taiko());
    REQUIRE(taiko.taiko()->colour.size() == 1);
    CHECK(taiko.taiko()->colour[0] == 0.0);
}
