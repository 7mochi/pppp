// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/mod.h"

namespace pppp { namespace mods {
    namespace {
        struct LegacyEntry {
            unsigned bits;
            ModId id;
        };

        const LegacyEntry LEGACY_TABLE[] = {
            {LEGACY_MOD_NIGHTCORE, MOD_NC},    {LEGACY_MOD_PERFECT, MOD_PF},
            {LEGACY_MOD_NO_FAIL, MOD_NF},      {LEGACY_MOD_EASY, MOD_EZ},
            {LEGACY_MOD_TOUCH_DEVICE, MOD_TD}, {LEGACY_MOD_HIDDEN, MOD_HD},
            {LEGACY_MOD_HARD_ROCK, MOD_HR},    {LEGACY_MOD_SUDDEN_DEATH, MOD_SD},
            {LEGACY_MOD_DOUBLE_TIME, MOD_DT},  {LEGACY_MOD_RELAX, MOD_RX},
            {LEGACY_MOD_HALF_TIME, MOD_HT},    {LEGACY_MOD_FLASHLIGHT, MOD_FL},
            {LEGACY_MOD_AUTOPLAY, MOD_AT},     {LEGACY_MOD_SPUN_OUT, MOD_SO},
            {LEGACY_MOD_AUTOPILOT, MOD_AP},    {LEGACY_MOD_RANDOM, MOD_RD},
            {LEGACY_MOD_CINEMA, MOD_CN},       {LEGACY_MOD_TARGET, MOD_TP},
            {LEGACY_MOD_SCORE_V2, MOD_SV2},    {LEGACY_MOD_MIRROR, MOD_MR},
            {LEGACY_MOD_KEY1, MOD_1K},         {LEGACY_MOD_KEY2, MOD_2K},
            {LEGACY_MOD_KEY3, MOD_3K},         {LEGACY_MOD_KEY4, MOD_4K},
            {LEGACY_MOD_KEY5, MOD_5K},         {LEGACY_MOD_KEY6, MOD_6K},
            {LEGACY_MOD_KEY7, MOD_7K},         {LEGACY_MOD_KEY8, MOD_8K},
            {LEGACY_MOD_KEY9, MOD_9K},         {LEGACY_MOD_KEY_COOP, MOD_DS},
            {LEGACY_MOD_FADE_IN, MOD_FI}};
        const size_t LEGACY_COUNT = sizeof(LEGACY_TABLE) / sizeof(LEGACY_TABLE[0]);
    } // namespace

    int mod_from_legacy(Mod* out, size_t max_count, unsigned bits) {
        if (out == 0) {
            return -1;
        }

        size_t n = 0;
        unsigned left = bits;
        for (size_t i = 0; i < LEGACY_COUNT; i++) {
            if ((left & LEGACY_TABLE[i].bits) != LEGACY_TABLE[i].bits) {
                continue;
            }
            if (n >= max_count) {
                return -1;
            }
            if (mod_make(&out[n], LEGACY_TABLE[i].id) != 0) {
                return -1;
            }
            n++;
            left &= ~LEGACY_TABLE[i].bits;
        }

        mod_sort(out, n);
        return static_cast<int>(n);
    }

    unsigned mod_to_legacy(const Mod* mods, size_t mod_count) {
        unsigned bits = 0;
        for (size_t i = 0; i < mod_count; i++) {
            for (size_t j = 0; j < LEGACY_COUNT; j++) {
                if (LEGACY_TABLE[j].id == mods[i].id) {
                    bits |= LEGACY_TABLE[j].bits;
                    break;
                }
            }
        }
        return bits;
    }
}} // namespace pppp::mods
