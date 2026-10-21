// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/generated_mods.h"
#include "pppp/mods/mod.h"

namespace pppp { namespace mods {
    namespace {
        bool has_flag(unsigned mods, unsigned flag) { return (mods & flag) == flag; }

        struct Output {
            Mod* out;
            size_t max_count;
            size_t count;
            bool overflow;

            void yield(ModId id) {
                if (count >= max_count) {
                    overflow = true;
                    return;
                }
                mod_make(&out[count], id);
                count++;
            }
        };
    } // namespace

    int mod_from_legacy(Mod* out, size_t max_count, unsigned mods, int ruleset) {
        if (out == 0 || ruleset < 0 || ruleset > 3) {
            return -1;
        }

        Output result = {out, max_count, 0, false};

        if (has_flag(mods, LEGACY_MOD_NIGHTCORE)) {
            result.yield(MOD_NC);
        } else if (has_flag(mods, LEGACY_MOD_DOUBLE_TIME)) {
            result.yield(MOD_DT);
        }

        if (has_flag(mods, LEGACY_MOD_PERFECT)) {
            result.yield(MOD_PF);
        } else if (has_flag(mods, LEGACY_MOD_SUDDEN_DEATH)) {
            result.yield(MOD_SD);
        }

        if (ruleset == 0 && has_flag(mods, LEGACY_MOD_AUTOPILOT)) {
            result.yield(MOD_AP);
        }

        if (has_flag(mods, LEGACY_MOD_CINEMA)) {
            result.yield(MOD_CN);
        } else if (has_flag(mods, LEGACY_MOD_AUTOPLAY)) {
            result.yield(MOD_AT);
        }

        if (has_flag(mods, LEGACY_MOD_EASY)) {
            result.yield(MOD_EZ);
        }

        if (ruleset == 3 && has_flag(mods, LEGACY_MOD_FADE_IN)) {
            result.yield(MOD_FI);
        }

        if (has_flag(mods, LEGACY_MOD_FLASHLIGHT)) {
            result.yield(MOD_FL);
        }

        if (has_flag(mods, LEGACY_MOD_HALF_TIME)) {
            result.yield(MOD_HT);
        }

        if (has_flag(mods, LEGACY_MOD_HARD_ROCK)) {
            result.yield(MOD_HR);
        }

        if (has_flag(mods, LEGACY_MOD_HIDDEN)) {
            result.yield(MOD_HD);
        }

        if (ruleset == 3) {
            if (has_flag(mods, LEGACY_MOD_KEY1)) {
                result.yield(MOD_1K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY2)) {
                result.yield(MOD_2K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY3)) {
                result.yield(MOD_3K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY4)) {
                result.yield(MOD_4K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY5)) {
                result.yield(MOD_5K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY6)) {
                result.yield(MOD_6K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY7)) {
                result.yield(MOD_7K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY8)) {
                result.yield(MOD_8K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY9)) {
                result.yield(MOD_9K);
            }

            if (has_flag(mods, LEGACY_MOD_KEY_COOP)) {
                result.yield(MOD_DS);
            }
        }

        if (has_flag(mods, LEGACY_MOD_NO_FAIL)) {
            result.yield(MOD_NF);
        }

        if (ruleset == 3) {
            if (has_flag(mods, LEGACY_MOD_RANDOM)) {
                result.yield(MOD_RD);
            }

            if (has_flag(mods, LEGACY_MOD_MIRROR)) {
                result.yield(MOD_MR);
            }
        } else if (has_flag(mods, LEGACY_MOD_RELAX)) {
            result.yield(MOD_RX);
        }

        if (ruleset == 0) {
            if (has_flag(mods, LEGACY_MOD_SPUN_OUT)) {
                result.yield(MOD_SO);
            }

            if (has_flag(mods, LEGACY_MOD_TARGET)) {
                result.yield(MOD_TP);
            }

            if (has_flag(mods, LEGACY_MOD_TOUCH_DEVICE)) {
                result.yield(MOD_TD);
            }
        }

        if (has_flag(mods, LEGACY_MOD_SCORE_V2)) {
            result.yield(MOD_SV2);
        }

        return result.overflow ? -1 : static_cast<int>(result.count);
    }

    unsigned mod_to_legacy(const Mod* mods, size_t mod_count, int ruleset) {
        if (ruleset < 0 || ruleset > 3) {
            return 0;
        }

        unsigned value = 0;
        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id < 0 || mods[i].id >= MOD_COUNT) {
                continue;
            }
            value |= generated::mod_def(mods[i].id).legacy_bits[ruleset];
        }

        if (ruleset == 3) {
            for (size_t i = 0; i < mod_count; i++) {
                if (mods[i].id == MOD_FI) {
                    value &= ~static_cast<unsigned>(LEGACY_MOD_HIDDEN);
                }
            }
        }

        return value;
    }
}} // namespace pppp::mods
