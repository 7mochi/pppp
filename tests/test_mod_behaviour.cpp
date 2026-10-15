// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "common.h"
#include "maps.h"
#include "pppp/difficulty.h"
#include "pppp/performance.h"
#include <cstddef>
#include <doctest.h>
#include <string>

namespace common = ::pppp::common;

TEST_CASE("the score's mods reach the difficulty when the calculator is given none") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/taiko/1028484.osu");

    const pppp::Mods mods = pppp_test::parse_mods("HR");

    pppp::ScoreInfo score;
    score.mods = mods;
    score.statistics[pppp::HIT_RESULT_GREAT] = 289;
    score.max_combo = pppp_test::calculate(pppp::Difficulty().mods(mods), beatmap).max_combo();
    score.accuracy = 1.0;

    const pppp::PerformanceAttributes from_score =
        pppp_test::calculate(pppp::Performance(beatmap).score(score));
    const pppp::DifficultyAttributes attributes =
        pppp_test::calculate(pppp::Difficulty().mods(mods).ruleset(pppp::Ruleset::TAIKO), beatmap);
    const pppp::PerformanceAttributes given =
        pppp_test::calculate(pppp::Performance(beatmap, attributes).mods(mods).score(score));

    CHECK(from_score.total() == given.total());
}

TEST_CASE("adding classic to legacy mods matches the classic specification") {
    pppp::Beatmap beatmap;
    pppp_test::load(beatmap, PPPP_TEST_RESOURCES "/osu/2785319.osu");

    pppp::mods::Mods from_bits = pppp::mods::Mods::from_legacy(0u, 0);
    REQUIRE_FALSE(from_bits.contains(pppp::mods::MOD_CL));

    from_bits.add_classic();
    REQUIRE(from_bits.contains(pppp::mods::MOD_CL));

    const size_t size = from_bits.size();
    from_bits.add_classic();
    CHECK(from_bits.size() == size);

    const pppp::DifficultyAttributes attributes =
        pppp_test::calculate(pppp::Difficulty().mods(from_bits).ruleset(pppp::Ruleset::OSU), beatmap);

    common::ScoreInfo score;
    score.statistics[common::HIT_RESULT_GREAT] = 601;
    score.statistics[common::HIT_RESULT_SLIDER_TAIL_HIT] = attributes.osu()->slider_count;
    score.max_combo = 909;
    score.accuracy = pppp_test::classic_accuracy(601, 0, 0, 0);

    const pppp::PerformanceAttributes actual =
        pppp_test::calculate(pppp::Performance(beatmap, attributes).mods(from_bits).score(score));
    CHECK(actual.osu()->total == pppp_test::pp_approx(298.5325815982227));

    const pppp::PerformanceAttributes expected = pppp_test::calculate(
        pppp::Performance(beatmap, attributes).mods(pppp_test::parse_mods("CL")).score(score));
    CHECK(actual.osu()->total == expected.osu()->total);
}

namespace {
    struct LegacyCase {
        unsigned legacy;
        pppp::mods::ModId mods[2];
        int count;
    };

    const LegacyCase OSU_FROM_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_TOUCH_DEVICE, {pppp::mods::MOD_TD, pppp::mods::MOD_TD}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_RELAX, {pppp::mods::MOD_RX, pppp::mods::MOD_RX}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_SPUN_OUT, {pppp::mods::MOD_SO, pppp::mods::MOD_SO}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPILOT, {pppp::mods::MOD_AP, pppp::mods::MOD_AP}, 1},
        {pppp::mods::LEGACY_MOD_TARGET, {pppp::mods::MOD_TP, pppp::mods::MOD_TP}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA, {pppp::mods::MOD_CN, pppp::mods::MOD_CN}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE, {pppp::mods::MOD_NC, pppp::mods::MOD_NC}, 1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT, {pppp::mods::MOD_PF, pppp::mods::MOD_PF}, 1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase OSU_TO_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_TOUCH_DEVICE, {pppp::mods::MOD_TD, pppp::mods::MOD_TD}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_RELAX, {pppp::mods::MOD_RX, pppp::mods::MOD_RX}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_SPUN_OUT, {pppp::mods::MOD_SO, pppp::mods::MOD_SO}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPILOT, {pppp::mods::MOD_AP, pppp::mods::MOD_AP}, 1},
        {pppp::mods::LEGACY_MOD_TARGET, {pppp::mods::MOD_TP, pppp::mods::MOD_TP}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase TAIKO_FROM_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_RELAX, {pppp::mods::MOD_RX, pppp::mods::MOD_RX}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA, {pppp::mods::MOD_CN, pppp::mods::MOD_CN}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE, {pppp::mods::MOD_NC, pppp::mods::MOD_NC}, 1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT, {pppp::mods::MOD_PF, pppp::mods::MOD_PF}, 1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase TAIKO_TO_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_RELAX, {pppp::mods::MOD_RX, pppp::mods::MOD_RX}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase CATCH_FROM_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_RELAX, {pppp::mods::MOD_RX, pppp::mods::MOD_RX}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA, {pppp::mods::MOD_CN, pppp::mods::MOD_CN}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE, {pppp::mods::MOD_NC, pppp::mods::MOD_NC}, 1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT, {pppp::mods::MOD_PF, pppp::mods::MOD_PF}, 1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase CATCH_TO_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_RELAX, {pppp::mods::MOD_RX, pppp::mods::MOD_RX}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase MANIA_FROM_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_KEY4, {pppp::mods::MOD_4K, pppp::mods::MOD_4K}, 1},
        {pppp::mods::LEGACY_MOD_KEY5, {pppp::mods::MOD_5K, pppp::mods::MOD_5K}, 1},
        {pppp::mods::LEGACY_MOD_KEY6, {pppp::mods::MOD_6K, pppp::mods::MOD_6K}, 1},
        {pppp::mods::LEGACY_MOD_KEY7, {pppp::mods::MOD_7K, pppp::mods::MOD_7K}, 1},
        {pppp::mods::LEGACY_MOD_KEY8, {pppp::mods::MOD_8K, pppp::mods::MOD_8K}, 1},
        {pppp::mods::LEGACY_MOD_FADE_IN, {pppp::mods::MOD_FI, pppp::mods::MOD_FI}, 1},
        {pppp::mods::LEGACY_MOD_RANDOM, {pppp::mods::MOD_RD, pppp::mods::MOD_RD}, 1},
        {pppp::mods::LEGACY_MOD_KEY9, {pppp::mods::MOD_9K, pppp::mods::MOD_9K}, 1},
        {pppp::mods::LEGACY_MOD_KEY_COOP, {pppp::mods::MOD_DS, pppp::mods::MOD_DS}, 1},
        {pppp::mods::LEGACY_MOD_KEY1, {pppp::mods::MOD_1K, pppp::mods::MOD_1K}, 1},
        {pppp::mods::LEGACY_MOD_KEY3, {pppp::mods::MOD_3K, pppp::mods::MOD_3K}, 1},
        {pppp::mods::LEGACY_MOD_KEY2, {pppp::mods::MOD_2K, pppp::mods::MOD_2K}, 1},
        {pppp::mods::LEGACY_MOD_MIRROR, {pppp::mods::MOD_MR, pppp::mods::MOD_MR}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA, {pppp::mods::MOD_CN, pppp::mods::MOD_CN}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE, {pppp::mods::MOD_NC, pppp::mods::MOD_NC}, 1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT, {pppp::mods::MOD_PF, pppp::mods::MOD_PF}, 1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    const LegacyCase MANIA_TO_LEGACY[] = {
        {pppp::mods::LEGACY_MOD_NO_FAIL, {pppp::mods::MOD_NF, pppp::mods::MOD_NF}, 1},
        {pppp::mods::LEGACY_MOD_EASY, {pppp::mods::MOD_EZ, pppp::mods::MOD_EZ}, 1},
        {pppp::mods::LEGACY_MOD_HIDDEN, {pppp::mods::MOD_HD, pppp::mods::MOD_HD}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK, {pppp::mods::MOD_HR, pppp::mods::MOD_HR}, 1},
        {pppp::mods::LEGACY_MOD_SUDDEN_DEATH, {pppp::mods::MOD_SD, pppp::mods::MOD_SD}, 1},
        {pppp::mods::LEGACY_MOD_DOUBLE_TIME, {pppp::mods::MOD_DT, pppp::mods::MOD_DT}, 1},
        {pppp::mods::LEGACY_MOD_HALF_TIME, {pppp::mods::MOD_HT, pppp::mods::MOD_HT}, 1},
        {pppp::mods::LEGACY_MOD_FLASHLIGHT, {pppp::mods::MOD_FL, pppp::mods::MOD_FL}, 1},
        {pppp::mods::LEGACY_MOD_AUTOPLAY, {pppp::mods::MOD_AT, pppp::mods::MOD_AT}, 1},
        {pppp::mods::LEGACY_MOD_KEY4, {pppp::mods::MOD_4K, pppp::mods::MOD_4K}, 1},
        {pppp::mods::LEGACY_MOD_KEY5, {pppp::mods::MOD_5K, pppp::mods::MOD_5K}, 1},
        {pppp::mods::LEGACY_MOD_KEY6, {pppp::mods::MOD_6K, pppp::mods::MOD_6K}, 1},
        {pppp::mods::LEGACY_MOD_KEY7, {pppp::mods::MOD_7K, pppp::mods::MOD_7K}, 1},
        {pppp::mods::LEGACY_MOD_KEY8, {pppp::mods::MOD_8K, pppp::mods::MOD_8K}, 1},
        {pppp::mods::LEGACY_MOD_FADE_IN, {pppp::mods::MOD_FI, pppp::mods::MOD_FI}, 1},
        {pppp::mods::LEGACY_MOD_RANDOM, {pppp::mods::MOD_RD, pppp::mods::MOD_RD}, 1},
        {pppp::mods::LEGACY_MOD_KEY9, {pppp::mods::MOD_9K, pppp::mods::MOD_9K}, 1},
        {pppp::mods::LEGACY_MOD_KEY_COOP, {pppp::mods::MOD_DS, pppp::mods::MOD_DS}, 1},
        {pppp::mods::LEGACY_MOD_KEY1, {pppp::mods::MOD_1K, pppp::mods::MOD_1K}, 1},
        {pppp::mods::LEGACY_MOD_KEY3, {pppp::mods::MOD_3K, pppp::mods::MOD_3K}, 1},
        {pppp::mods::LEGACY_MOD_KEY2, {pppp::mods::MOD_2K, pppp::mods::MOD_2K}, 1},
        {pppp::mods::LEGACY_MOD_MIRROR, {pppp::mods::MOD_MR, pppp::mods::MOD_MR}, 1},
        {pppp::mods::LEGACY_MOD_HARD_ROCK | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_HR, pppp::mods::MOD_DT},
         2},
        {pppp::mods::LEGACY_MOD_SCORE_V2, {pppp::mods::MOD_SV2, pppp::mods::MOD_SV2}, 1},
        {pppp::mods::LEGACY_MOD_CINEMA | pppp::mods::LEGACY_MOD_AUTOPLAY,
         {pppp::mods::MOD_CN, pppp::mods::MOD_CN},
         1},
        {pppp::mods::LEGACY_MOD_NIGHTCORE | pppp::mods::LEGACY_MOD_DOUBLE_TIME,
         {pppp::mods::MOD_NC, pppp::mods::MOD_NC},
         1},
        {pppp::mods::LEGACY_MOD_PERFECT | pppp::mods::LEGACY_MOD_SUDDEN_DEATH,
         {pppp::mods::MOD_PF, pppp::mods::MOD_PF},
         1},
    };

    void test_from_legacy(const LegacyCase* cases, size_t count, int ruleset) {
        for (size_t i = 0; i < count; i++) {
            const pppp::mods::Mods mods = pppp::mods::Mods::from_legacy(cases[i].legacy, ruleset);
            INFO(cases[i].legacy);
            REQUIRE(mods.size() == static_cast<size_t>(cases[i].count));
            for (int j = 0; j < cases[i].count; j++) {
                CHECK(mods.contains(cases[i].mods[j]));
            }
        }
    }

    void test_to_legacy(const LegacyCase* cases, size_t count, int ruleset) {
        for (size_t i = 0; i < count; i++) {
            pppp::mods::Mods mods;
            for (int j = 0; j < cases[i].count; j++) {
                mods.push_back(cases[i].mods[j]);
            }
            INFO(cases[i].legacy);
            CHECK(mods.to_legacy(ruleset) == cases[i].legacy);
        }
    }
} // namespace

TEST_CASE("osu! converts legacy mods the way its ruleset does") {
    test_from_legacy(OSU_FROM_LEGACY, sizeof(OSU_FROM_LEGACY) / sizeof(OSU_FROM_LEGACY[0]), 0);
    test_to_legacy(OSU_TO_LEGACY, sizeof(OSU_TO_LEGACY) / sizeof(OSU_TO_LEGACY[0]), 0);
}

TEST_CASE("osu!taiko converts legacy mods the way its ruleset does") {
    test_from_legacy(TAIKO_FROM_LEGACY, sizeof(TAIKO_FROM_LEGACY) / sizeof(TAIKO_FROM_LEGACY[0]), 1);
    test_to_legacy(TAIKO_TO_LEGACY, sizeof(TAIKO_TO_LEGACY) / sizeof(TAIKO_TO_LEGACY[0]), 1);
}

TEST_CASE("osu!catch converts legacy mods the way its ruleset does") {
    test_from_legacy(CATCH_FROM_LEGACY, sizeof(CATCH_FROM_LEGACY) / sizeof(CATCH_FROM_LEGACY[0]), 2);
    test_to_legacy(CATCH_TO_LEGACY, sizeof(CATCH_TO_LEGACY) / sizeof(CATCH_TO_LEGACY[0]), 2);
}

TEST_CASE("osu!mania converts legacy mods the way its ruleset does") {
    test_from_legacy(MANIA_FROM_LEGACY, sizeof(MANIA_FROM_LEGACY) / sizeof(MANIA_FROM_LEGACY[0]), 3);
    test_to_legacy(MANIA_TO_LEGACY, sizeof(MANIA_TO_LEGACY) / sizeof(MANIA_TO_LEGACY[0]), 3);
}

namespace {
    struct SettingCase {
        pppp::mods::ModId id;
        double given;
        double expected;
    };

    // Expected values are osu!framework 2026.728.1's BindableDouble/BindableFloat assigned the given
    // value with each mod's MinValue, MaxValue and Precision.
    const SettingCase SETTING_CASES[] = {
        {pppp::mods::MOD_DT, 1.234, 1.23}, {pppp::mods::MOD_DT, 1.245, 1.24},
        {pppp::mods::MOD_DT, 1.255, 1.26}, {pppp::mods::MOD_DT, 2.5, 2},
        {pppp::mods::MOD_DT, 1, 1.01},
    };

    pppp::mods::Mod hand_built(const SettingCase& c) {
        pppp::mods::Mod mod;
        pppp::mods::mod_make(&mod, c.id);
        mod.rate.speed_change = c.given;
        return mod;
    }
} // namespace

TEST_CASE("a mod built by hand keeps the value its bindable would") {
    for (size_t i = 0; i < sizeof(SETTING_CASES) / sizeof(SETTING_CASES[0]); i++) {
        CAPTURE(SETTING_CASES[i].given);
        const pppp::mods::Mod mod = hand_built(SETTING_CASES[i]);

        pppp::mods::Mods pushed;
        pushed.push_back(mod);
        CHECK(pushed[0].rate.speed_change == SETTING_CASES[i].expected);

        const pppp::mods::Mods ranged(&mod, &mod + 1);
        CHECK(ranged[0].rate.speed_change == SETTING_CASES[i].expected);
    }

    pppp::mods::Mod magnetised;
    pppp::mods::mod_make(&magnetised, pppp::mods::MOD_MG);
    magnetised.magnetised.strength = 0.63f;
    pppp::mods::Mods pushed;
    pushed.push_back(magnetised);
    CHECK(pushed[0].magnetised.strength == 0.65f);
}

TEST_CASE("an empty value for an ignored setting is refused") {
    pppp::Mods mods;
    CHECK_FALSE(mods.parse("CO:coverage=").ok());
}

TEST_CASE("a mod list of more than 64 pieces is refused") {
    std::string specification;
    for (int i = 0; i < 65; i++) {
        if (i != 0) {
            specification += ",";
        }
        specification += "HD";
    }
    pppp::Mods mods;
    CHECK_FALSE(mods.parse(specification.c_str()).ok());
}
