// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/mod.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cstring>

#define PPPP_MOD_INCOMPATIBLE(list) list, sizeof(list) / sizeof((list)[0])

namespace pppp { namespace mods {

    namespace {
        struct ModDef {
            const char* acronym;
            const char* name;
            unsigned rulesets;
            const ModId* incompatible;
            unsigned incompatible_count;
        };

        const ModId INCOMPATIBLE_EZ[] = {MOD_HR, MOD_AC, MOD_DA};
        const ModId INCOMPATIBLE_NF[] = {MOD_SD, MOD_PF, MOD_AC, MOD_CN};
        const ModId INCOMPATIBLE_HT[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_WU, MOD_WD, MOD_AS};
        const ModId INCOMPATIBLE_DC[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_WU, MOD_WD, MOD_AS};
        const ModId INCOMPATIBLE_HR[] = {MOD_EZ, MOD_DA, MOD_MR};
        const ModId INCOMPATIBLE_SD[] = {MOD_NF, MOD_PF, MOD_TP, MOD_CN};
        const ModId INCOMPATIBLE_PF[] = {MOD_NF, MOD_SD, MOD_AC, MOD_CN};
        const ModId INCOMPATIBLE_DT[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_WU, MOD_WD, MOD_AS};
        const ModId INCOMPATIBLE_NC[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_WU, MOD_WD, MOD_AS};
        const ModId INCOMPATIBLE_HD[] = {MOD_TC, MOD_SI, MOD_AD, MOD_FR, MOD_DP, MOD_CO, MOD_FI, MOD_FL};
        const ModId INCOMPATIBLE_TC[] = {MOD_HD, MOD_TP, MOD_SI, MOD_GR, MOD_DF, MOD_DP};
        const ModId INCOMPATIBLE_FL[] = {MOD_BL, MOD_BM, MOD_CO, MOD_FI, MOD_HD};
        const ModId INCOMPATIBLE_BL[] = {MOD_FL};
        const ModId INCOMPATIBLE_ST[] = {MOD_TP, MOD_CL};
        const ModId INCOMPATIBLE_AC[] = {MOD_EZ, MOD_NF, MOD_PF, MOD_CN};
        const ModId INCOMPATIBLE_TP[] = {MOD_SD, MOD_TC, MOD_ST, MOD_DA, MOD_RD, MOD_SO, MOD_AD, MOD_DP};
        const ModId INCOMPATIBLE_DA[] = {MOD_EZ, MOD_HR, MOD_TP};
        const ModId INCOMPATIBLE_CL[] = {MOD_ST};
        const ModId INCOMPATIBLE_RD[] = {MOD_TP, MOD_SW};
        const ModId INCOMPATIBLE_MR[] = {MOD_HR};
        const ModId INCOMPATIBLE_AL[] = {MOD_SG, MOD_AT, MOD_CN, MOD_RX};
        const ModId INCOMPATIBLE_SG[] = {MOD_AL, MOD_AT, MOD_CN, MOD_RX};
        const ModId INCOMPATIBLE_AT[] = {MOD_AL, MOD_SG, MOD_CN, MOD_RX, MOD_AP, MOD_SO,
                                         MOD_MG, MOD_RP, MOD_AS, MOD_TD, MOD_MF};
        const ModId INCOMPATIBLE_CN[] = {MOD_NF, MOD_SD, MOD_PF, MOD_AC, MOD_AL, MOD_SG, MOD_AT, MOD_CN,
                                         MOD_RX, MOD_AP, MOD_SO, MOD_MG, MOD_RP, MOD_AS, MOD_TD, MOD_MF};
        const ModId INCOMPATIBLE_RX[] = {MOD_AL, MOD_SG, MOD_AT, MOD_CN, MOD_AP, MOD_MG, MOD_MF};
        const ModId INCOMPATIBLE_AP[] = {MOD_AT, MOD_CN, MOD_RX, MOD_SO, MOD_MG, MOD_RP, MOD_TD};
        const ModId INCOMPATIBLE_SO[] = {MOD_TP, MOD_AT, MOD_CN, MOD_AP};
        const ModId INCOMPATIBLE_TR[] = {MOD_WG, MOD_MG, MOD_RP, MOD_FR, MOD_DP};
        const ModId INCOMPATIBLE_WG[] = {MOD_TR, MOD_MG, MOD_RP, MOD_DP};
        const ModId INCOMPATIBLE_SI[] = {MOD_HD, MOD_TC, MOD_GR, MOD_DF, MOD_AD, MOD_DP};
        const ModId INCOMPATIBLE_GR[] = {MOD_TC, MOD_SI, MOD_GR, MOD_DF, MOD_AD, MOD_DP};
        const ModId INCOMPATIBLE_DF[] = {MOD_TC, MOD_SI, MOD_GR, MOD_DF, MOD_AD, MOD_DP};
        const ModId INCOMPATIBLE_WU[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_WD, MOD_AS};
        const ModId INCOMPATIBLE_WD[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_WU, MOD_AS};
        const ModId INCOMPATIBLE_BR[] = {MOD_BU};
        const ModId INCOMPATIBLE_AD[] = {MOD_HD, MOD_TP, MOD_SI, MOD_GR, MOD_DF, MOD_FR};
        const ModId INCOMPATIBLE_NS[] = {MOD_BM};
        const ModId INCOMPATIBLE_MG[] = {MOD_AT, MOD_CN, MOD_RX, MOD_AP, MOD_TR,
                                         MOD_WG, MOD_RP, MOD_BU, MOD_DP};
        const ModId INCOMPATIBLE_RP[] = {MOD_AT, MOD_CN, MOD_AP, MOD_TR, MOD_WG, MOD_MG, MOD_BU, MOD_DP};
        const ModId INCOMPATIBLE_AS[] = {MOD_HT, MOD_DC, MOD_DT, MOD_NC, MOD_AT, MOD_CN, MOD_WU, MOD_WD};
        const ModId INCOMPATIBLE_FR[] = {MOD_HD, MOD_TR, MOD_AD, MOD_DP};
        const ModId INCOMPATIBLE_BU[] = {MOD_BR, MOD_MG, MOD_RP};
        const ModId INCOMPATIBLE_DP[] = {MOD_HD, MOD_TC, MOD_TP, MOD_TR, MOD_WG, MOD_SI,
                                         MOD_GR, MOD_DF, MOD_MG, MOD_RP, MOD_FR, MOD_DP};
        const ModId INCOMPATIBLE_BM[] = {MOD_FL, MOD_NS, MOD_TD};
        const ModId INCOMPATIBLE_TD[] = {MOD_AT, MOD_CN, MOD_AP, MOD_BM};
        const ModId INCOMPATIBLE_SW[] = {MOD_RD};
        const ModId INCOMPATIBLE_MF[] = {MOD_AT, MOD_RX, MOD_CN};
        const ModId INCOMPATIBLE_1K[] = {MOD_2K, MOD_3K, MOD_4K, MOD_5K, MOD_6K,
                                         MOD_7K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_2K[] = {MOD_1K, MOD_3K, MOD_4K, MOD_5K, MOD_6K,
                                         MOD_7K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_3K[] = {MOD_1K, MOD_2K, MOD_4K, MOD_5K, MOD_6K,
                                         MOD_7K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_4K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_5K, MOD_6K,
                                         MOD_7K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_5K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_4K, MOD_6K,
                                         MOD_7K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_6K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_4K, MOD_5K,
                                         MOD_7K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_7K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_4K, MOD_5K,
                                         MOD_6K, MOD_8K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_8K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_4K, MOD_5K,
                                         MOD_6K, MOD_7K, MOD_9K, MOD_10K};
        const ModId INCOMPATIBLE_9K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_4K, MOD_5K,
                                         MOD_6K, MOD_7K, MOD_8K, MOD_10K};
        const ModId INCOMPATIBLE_10K[] = {MOD_1K, MOD_2K, MOD_3K, MOD_4K, MOD_5K,
                                          MOD_6K, MOD_7K, MOD_8K, MOD_9K};
        const ModId INCOMPATIBLE_IN[] = {MOD_HO, MOD_NR};
        const ModId INCOMPATIBLE_HO[] = {MOD_IN, MOD_NR};
        const ModId INCOMPATIBLE_NR[] = {MOD_IN, MOD_HO};
        const ModId INCOMPATIBLE_CO[] = {MOD_HD, MOD_FL, MOD_FI};
        const ModId INCOMPATIBLE_FI[] = {MOD_HD, MOD_FL, MOD_CO};

        const ModDef MOD_DEFS[MOD_COUNT] = {
            {"EZ", "Easy", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_EZ)},
            {"NF", "No Fail", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_NF)},
            {"HT", "Half Time", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_HT)},
            {"DC", "Daycore", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_DC)},
            {"HR", "Hard Rock", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_HR)},
            {"SD", "Sudden Death", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_SD)},
            {"PF", "Perfect", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_PF)},
            {"DT", "Double Time", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_DT)},
            {"NC", "Nightcore", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_NC)},
            {"HD", "Hidden", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_HD)},
            {"TC", "Traceable", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_TC)},
            {"FL", "Flashlight", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_FL)},
            {"BL", "Blinds", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_BL)},
            {"ST", "Strict Tracking", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_ST)},
            {"AC", "Accuracy Challenge", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_AC)},
            {"TP", "Target Practice", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_TP)},
            {"DA", "Difficulty Adjust", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_DA)},
            {"CL", "Classic", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_CL)},
            {"RD", "Random", RULESET_OSU | RULESET_TAIKO | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_RD)},
            {"MR", "Mirror", RULESET_OSU | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_MR)},
            {"AL", "Alternate", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_AL)},
            {"SG", "Single Tap", RULESET_OSU | RULESET_TAIKO, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_SG)},
            {"AT", "Autoplay", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_AT)},
            {"CN", "Cinema", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_CN)},
            {"RX", "Relax", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_RX)},
            {"AP", "Autopilot", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_AP)},
            {"SO", "Spun Out", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_SO)},
            {"TR", "Transform", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_TR)},
            {"WG", "Wiggle", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_WG)},
            {"SI", "Spin In", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_SI)},
            {"GR", "Grow", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_GR)},
            {"DF", "Deflate", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_DF)},
            {"WU", "Wind Up", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_WU)},
            {"WD", "Wind Down", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_WD)},
            {"BR", "Barrel Roll", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_BR)},
            {"AD", "Approach Different", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_AD)},
            {"MU", "Muted", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA, 0, 0},
            {"NS", "No Scope", RULESET_OSU | RULESET_CATCH, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_NS)},
            {"MG", "Magnetised", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_MG)},
            {"RP", "Repel", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_RP)},
            {"AS", "Adaptive Speed", RULESET_OSU | RULESET_TAIKO | RULESET_MANIA,
             PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_AS)},
            {"FR", "Freeze Frame", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_FR)},
            {"BU", "Bubbles", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_BU)},
            {"SY", "Synesthesia", RULESET_OSU | RULESET_CATCH, 0, 0},
            {"DP", "Depth", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_DP)},
            {"BM", "Bloom", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_BM)},
            {"TD", "Touch Device", RULESET_OSU, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_TD)},
            {"SV2", "Score V2", RULESET_OSU | RULESET_TAIKO | RULESET_CATCH | RULESET_MANIA, 0, 0},
            {"SW", "Swap", RULESET_TAIKO, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_SW)},
            {"SR", "Simplified Rhythm", RULESET_TAIKO, 0, 0},
            {"CS", "Constant Speed", RULESET_TAIKO | RULESET_MANIA, 0, 0},
            {"FF", "Floating Fruits", RULESET_CATCH, 0, 0},
            {"MF", "Moving Fast", RULESET_CATCH, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_MF)},
            {"1K", "One Key", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_1K)},
            {"2K", "Two Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_2K)},
            {"3K", "Three Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_3K)},
            {"4K", "Four Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_4K)},
            {"5K", "Five Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_5K)},
            {"6K", "Six Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_6K)},
            {"7K", "Seven Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_7K)},
            {"8K", "Eight Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_8K)},
            {"9K", "Nine Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_9K)},
            {"10K", "Ten Keys", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_10K)},
            {"DS", "Dual Stages", RULESET_MANIA, 0, 0},
            {"IN", "Invert", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_IN)},
            {"HO", "Hold Off", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_HO)},
            {"NR", "No Release", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_NR)},
            {"CO", "Cover", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_CO)},
            {"FI", "Fade In", RULESET_MANIA, PPPP_MOD_INCOMPATIBLE(INCOMPATIBLE_FI)},
        };
    } // namespace

    const char* mod_acronym(ModId id) { return (id >= 0 && id < MOD_COUNT) ? MOD_DEFS[id].acronym : 0; }

    unsigned mod_rulesets(ModId id) { return (id >= 0 && id < MOD_COUNT) ? MOD_DEFS[id].rulesets : 0; }

    const char* mod_name(ModId id) { return (id >= 0 && id < MOD_COUNT) ? MOD_DEFS[id].name : 0; }

    bool mod_incompatible(ModId id, ModId other) {
        if (id < 0 || id >= MOD_COUNT || other < 0 || other >= MOD_COUNT) {
            return 0;
        }
        const ModDef& def = MOD_DEFS[id];

        for (unsigned i = 0; i < def.incompatible_count; i++) {
            if (def.incompatible[i] == other) {
                return 1;
            }
        }
        return 0;
    }

    int mod_from_acronym(Mod* out, const char* acronym) {
        if (out == 0 || acronym == 0) {
            return -1;
        }

        for (int i = 0; i < MOD_COUNT; i++) {
            if (std::strcmp(MOD_DEFS[i].acronym, acronym) == 0) {
                return mod_make(out, static_cast<ModId>(i));
            }
        }
        return -1;
    }

    double mod_calculate_rate(const Mod* mods, size_t mod_count) {
        double rate = 1.0;
        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case MOD_DT:
            case MOD_NC: rate *= mods[i].rate.speed_change > 0.0 ? mods[i].rate.speed_change : 1.5; break;
            case MOD_HT:
            case MOD_DC: rate *= mods[i].rate.speed_change > 0.0 ? mods[i].rate.speed_change : 0.75; break;
            case MOD_WU:
            case MOD_WD: rate *= utils::math::round2_to_even(mods[i].variable_rate.initial_rate); break;
            case MOD_AS: rate *= mods[i].variable_rate.initial_rate; break;
            default: break;
            }
        }
        return rate;
    }

    void mod_apply_to_difficulty(const Mod* mods, size_t mod_count, DifficultySettings* settings) {
        if (settings == 0) {
            return;
        }

        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id != MOD_DA) {
                continue;
            }
            const DifficultyAdjustSettings& st = mods[i].difficulty_adjust;
            if (st.circle_size.value() >= 0.0) {
                settings->circle_size = utils::f32(st.circle_size.value());
            }
            if (st.approach_rate.value() >= 0.0) {
                settings->approach_rate = utils::f32(st.approach_rate.value());
            }
            if (st.drain_rate.value() >= 0.0) {
                settings->drain_rate = utils::f32(st.drain_rate.value());
            }
            if (st.overall_difficulty.value() >= 0.0) {
                settings->overall_difficulty = utils::f32(st.overall_difficulty.value());
            }
        }

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case MOD_HR:
                settings->drain_rate =
                    std::min(utils::f32(utils::f32(settings->drain_rate) * HARD_ROCK_ADJUST_RATIO), 10.0);
                settings->overall_difficulty = std::min(
                    utils::f32(utils::f32(settings->overall_difficulty) * HARD_ROCK_ADJUST_RATIO), 10.0);
                // CS uses a custom 1.3 ratio.
                settings->circle_size =
                    std::min(utils::f32(utils::f32(settings->circle_size) * 1.2999999523162842), 10.0);
                settings->approach_rate =
                    std::min(utils::f32(utils::f32(settings->approach_rate) * HARD_ROCK_ADJUST_RATIO), 10.0);
                break;
            case MOD_TP:
                // Decrease AR to increase preempt time
                settings->approach_rate = utils::f32(utils::f32(settings->approach_rate) * 0.5);
                break;
            case MOD_EZ:
                settings->circle_size = utils::f32(utils::f32(settings->circle_size) * EASY_ADJUST_RATIO);
                settings->approach_rate = utils::f32(utils::f32(settings->approach_rate) * EASY_ADJUST_RATIO);
                settings->drain_rate = utils::f32(utils::f32(settings->drain_rate) * EASY_ADJUST_RATIO);
                settings->overall_difficulty =
                    utils::f32(utils::f32(settings->overall_difficulty) * EASY_ADJUST_RATIO);
                break;
            default: break;
            }
        }
    }

    bool mod_has(const Mod* mods, size_t mod_count, ModId id) {
        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id == id) {
                return 1;
            }
        }
        return 0;
    }

    int mod_reflection(const Mod* mods, size_t mod_count) {
        int axes = MOD_REFLECTION_NONE;
        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id == MOD_MR) {
                const int mirror = mods[i].mirror.axes;
                axes ^= mirror != MOD_REFLECTION_NONE ? mirror : MOD_REFLECTION_HORIZONTAL;
            } else if (mods[i].id == MOD_HR) {
                axes ^= MOD_REFLECTION_VERTICAL;
            }
        }
        return axes;
    }

    int mod_validate(const Mod* mods, size_t mod_count, ModId* first, ModId* second) {
        for (size_t i = 0; i < mod_count; i++) {
            for (size_t j = i + 1; j < mod_count; j++) {
                if (mods[i].id != mods[j].id && !mod_incompatible(mods[i].id, mods[j].id)) {
                    continue;
                }
                if (first) {
                    *first = mods[i].id;
                }
                if (second) {
                    *second = mods[j].id;
                }
                return -1;
            }
        }
        return 0;
    }

    void mod_sort(Mod* mods, size_t mod_count) {
        for (size_t i = 1; i < mod_count; i++) {
            const Mod key = mods[i];
            size_t j = i;
            while (j > 0 && mods[j - 1].id > key.id) {
                mods[j] = mods[j - 1];
                j--;
            }
            mods[j] = key;
        }
    }
}} // namespace pppp::mods
