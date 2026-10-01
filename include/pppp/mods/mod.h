// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MODS_MOD_H
#define PPPP_MODS_MOD_H

#include "pppp/config.h" // IWYU pragma: export
#include <cstddef>

namespace pppp { namespace mods {
    const double EASY_ADJUST_RATIO = 0.5;
    const double HARD_ROCK_ADJUST_RATIO = 1.399999976158142; // the float 1.4 widened to double

    enum ModId {
        MOD_EZ = 0,
        MOD_NF,
        MOD_HT,
        MOD_DC,
        MOD_HR,
        MOD_SD,
        MOD_PF,
        MOD_DT,
        MOD_NC,
        MOD_HD,
        MOD_TC,
        MOD_FL,
        MOD_BL,
        MOD_ST,
        MOD_AC,
        MOD_TP,
        MOD_DA,
        MOD_CL,
        MOD_RD,
        MOD_MR,
        MOD_AL,
        MOD_SG,
        MOD_AT,
        MOD_CN,
        MOD_RX,
        MOD_AP,
        MOD_SO,
        MOD_TR,
        MOD_WG,
        MOD_SI,
        MOD_GR,
        MOD_DF,
        MOD_WU,
        MOD_WD,
        MOD_BR,
        MOD_AD,
        MOD_MU,
        MOD_NS,
        MOD_MG,
        MOD_RP,
        MOD_AS,
        MOD_FR,
        MOD_BU,
        MOD_SY,
        MOD_DP,
        MOD_BM,
        MOD_TD,
        MOD_SV2,
        MOD_SW, // osu!taiko
        MOD_SR, // osu!taiko
        MOD_CS, // osu!taiko
        MOD_FF, // osu!catch
        MOD_MF, // osu!catch
        MOD_1K, // osu!mania
        MOD_2K, // osu!mania
        MOD_3K, // osu!mania
        MOD_4K, // osu!mania
        MOD_5K, // osu!mania
        MOD_6K, // osu!mania
        MOD_7K, // osu!mania
        MOD_8K, // osu!mania
        MOD_9K, // osu!mania
        MOD_10K, // osu!mania
        MOD_DS, // osu!mania
        MOD_IN, // osu!mania
        MOD_HO, // osu!mania
        MOD_NR, // osu!mania
        MOD_CO, // osu!mania
        MOD_FI, // osu!mania
        MOD_COUNT
    };

    enum RulesetMask {
        RULESET_OSU = 1u << 0,
        RULESET_TAIKO = 1u << 1,
        RULESET_CATCH = 1u << 2,
        RULESET_MANIA = 1u << 3
    };

    enum ReflectionAxes {
        MOD_REFLECTION_NONE = 0,
        MOD_REFLECTION_HORIZONTAL = 1,
        MOD_REFLECTION_VERTICAL = 2,
        MOD_REFLECTION_BOTH = 3
    };

    struct DifficultySettings {
        double drain_rate;
        double circle_size;
        double overall_difficulty;
        double approach_rate;
        double slider_multiplier;
        double slider_tick_rate;
    };

    struct RateSettings {
        double speed_change;

        RateSettings()
            : speed_change(0.0) {}
    };

    struct VariableRateSettings {
        double initial_rate;

        VariableRateSettings()
            : initial_rate(0.0) {}
    };

    struct RandomSettings {
        nonstd::optional<int> seed;
        double angle_sharpness;

        RandomSettings()
            : seed(),
              // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
              angle_sharpness(0.0) {}
    };

    struct TargetSettings {
        nonstd::optional<int> seed;

        TargetSettings()
            // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
            : seed() {}
    };

    struct MagnetisedSettings {
        double strength;

        MagnetisedSettings()
            : strength(0.0) {}
    };

    struct ScaleSettings {
        double start_scale;

        ScaleSettings()
            : start_scale(0.0) {}
    };

    struct HiddenSettings {
        bool only_fade_approach_circles;

        HiddenSettings()
            : only_fade_approach_circles(false) {}
    };

    struct ClassicSettings {
        bool no_slider_head_accuracy;

        ClassicSettings()
            : no_slider_head_accuracy(false) {}
    };

    struct MirrorSettings {
        int axes;

        MirrorSettings()
            : axes(MOD_REFLECTION_NONE) {}
    };

    struct SimplifiedRhythmSettings {
        bool one_third_conversion;
        bool one_sixth_conversion;
        bool one_eighth_conversion;

        SimplifiedRhythmSettings()
            : one_third_conversion(false),
              one_sixth_conversion(false),
              one_eighth_conversion(false) {}
    };

    struct DifficultyAdjustSettings {
        // negative means leave the beatmap's value alone
        nonstd::optional<double> circle_size;
        nonstd::optional<double> approach_rate;
        nonstd::optional<double> drain_rate;
        nonstd::optional<double> overall_difficulty;
        nonstd::optional<double> scroll_speed; // osu!taiko
        bool hard_rock_offsets; // osu!catch
        bool extended_limits;

        DifficultyAdjustSettings()
            : circle_size(),
              approach_rate(),
              drain_rate(),
              overall_difficulty(),
              scroll_speed(),
              hard_rock_offsets(false),
              // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
              extended_limits(false) {}
    };

    struct Mod {
        ModId id;
        RateSettings rate; // DT NC HT DC
        VariableRateSettings variable_rate; // WU WD AS
        RandomSettings random; // RD
        TargetSettings target; // TP
        MagnetisedSettings magnetised; // MG
        ScaleSettings scale; // GR DF
        HiddenSettings hidden; // HD
        ClassicSettings classic; // CL
        MirrorSettings mirror; // MR
        SimplifiedRhythmSettings simplified_rhythm; // SR
        DifficultyAdjustSettings difficulty_adjust; // DA

        Mod();
    };

    enum LegacyMod {
        LEGACY_MOD_NO_FAIL = 1u << 0,
        LEGACY_MOD_EASY = 1u << 1,
        LEGACY_MOD_TOUCH_DEVICE = 1u << 2,
        LEGACY_MOD_HIDDEN = 1u << 3,
        LEGACY_MOD_HARD_ROCK = 1u << 4,
        LEGACY_MOD_SUDDEN_DEATH = 1u << 5,
        LEGACY_MOD_DOUBLE_TIME = 1u << 6,
        LEGACY_MOD_RELAX = 1u << 7,
        LEGACY_MOD_HALF_TIME = 1u << 8,
        LEGACY_MOD_NIGHTCORE = (1u << 9) | LEGACY_MOD_DOUBLE_TIME,
        LEGACY_MOD_FLASHLIGHT = 1u << 10,
        LEGACY_MOD_AUTOPLAY = 1u << 11,
        LEGACY_MOD_SPUN_OUT = 1u << 12,
        LEGACY_MOD_AUTOPILOT = 1u << 13,
        LEGACY_MOD_PERFECT = (1u << 14) | LEGACY_MOD_SUDDEN_DEATH,
        LEGACY_MOD_RANDOM = 1u << 21,
        LEGACY_MOD_CINEMA = 1u << 22,
        LEGACY_MOD_TARGET = 1u << 23,
        LEGACY_MOD_SCORE_V2 = 1u << 29,
        LEGACY_MOD_MIRROR = 1u << 30,
        // osu!mania's key counts, stable has no bit for 10K, Invert, Hold Off, No Release or Cover.
        LEGACY_MOD_KEY4 = 1u << 15,
        LEGACY_MOD_KEY5 = 1u << 16,
        LEGACY_MOD_KEY6 = 1u << 17,
        LEGACY_MOD_KEY7 = 1u << 18,
        LEGACY_MOD_KEY8 = 1u << 19,
        LEGACY_MOD_FADE_IN = 1u << 20,
        LEGACY_MOD_KEY9 = 1u << 24,
        LEGACY_MOD_KEY_COOP = 1u << 25, // KeyCoop
        LEGACY_MOD_KEY1 = 1u << 26,
        LEGACY_MOD_KEY3 = 1u << 27,
        LEGACY_MOD_KEY2 = 1u << 28
    };

    const char* mod_acronym(ModId id);
    const char* mod_name(ModId id);

    unsigned mod_rulesets(ModId id);
    bool mod_incompatible(ModId id, ModId other);

    void mod_defaults(ModId id, Mod* out);
    int mod_make(Mod* out, ModId id);
    int mod_from_acronym(Mod* out, const char* acronym);
    int mod_apply_settings(Mod* out, const char* settings);
    int mod_from_acronyms(Mod* out, size_t max_count, const char* acronyms);
    double mod_calculate_rate(const Mod* mods, size_t mod_count);
    void mod_apply_to_difficulty(const Mod* mods, size_t mod_count, DifficultySettings* settings);
    bool mod_has(const Mod* mods, size_t mod_count, ModId id);
    int mod_reflection(const Mod* mods, size_t mod_count);

    int mod_from_legacy(Mod* out, size_t max_count, unsigned bits);
    unsigned mod_to_legacy(const Mod* mods, size_t mod_count);

    int mod_validate(const Mod* mods, size_t mod_count, ModId* first, ModId* second);

    void mod_sort(Mod* mods, size_t mod_count);
}} // namespace pppp::mods

#endif
