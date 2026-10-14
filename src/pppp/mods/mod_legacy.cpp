// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

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
        unsigned value = 0;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case MOD_NF: value |= LEGACY_MOD_NO_FAIL; break;
            case MOD_EZ: value |= LEGACY_MOD_EASY; break;
            case MOD_HD:
            case MOD_FI: value |= LEGACY_MOD_HIDDEN; break;
            case MOD_HR: value |= LEGACY_MOD_HARD_ROCK; break;
            case MOD_PF: value |= LEGACY_MOD_PERFECT | LEGACY_MOD_SUDDEN_DEATH; break;
            case MOD_SD: value |= LEGACY_MOD_SUDDEN_DEATH; break;
            case MOD_NC: value |= LEGACY_MOD_NIGHTCORE | LEGACY_MOD_DOUBLE_TIME; break;
            case MOD_DT: value |= LEGACY_MOD_DOUBLE_TIME; break;
            case MOD_RX: value |= LEGACY_MOD_RELAX; break;
            case MOD_HT: value |= LEGACY_MOD_HALF_TIME; break;
            case MOD_FL: value |= LEGACY_MOD_FLASHLIGHT; break;
            case MOD_CN: value |= LEGACY_MOD_CINEMA | LEGACY_MOD_AUTOPLAY; break;
            case MOD_AT: value |= LEGACY_MOD_AUTOPLAY; break;
            case MOD_SV2: value |= LEGACY_MOD_SCORE_V2; break;
            default: break;
            }
        }

        for (size_t i = 0; i < mod_count; i++) {
            if (ruleset == 0) {
                switch (mods[i].id) {
                case MOD_AP: value |= LEGACY_MOD_AUTOPILOT; break;
                case MOD_SO: value |= LEGACY_MOD_SPUN_OUT; break;
                case MOD_TP: value |= LEGACY_MOD_TARGET; break;
                case MOD_TD: value |= LEGACY_MOD_TOUCH_DEVICE; break;
                default: break;
                }
            } else if (ruleset == 3) {
                switch (mods[i].id) {
                case MOD_1K: value |= LEGACY_MOD_KEY1; break;
                case MOD_2K: value |= LEGACY_MOD_KEY2; break;
                case MOD_3K: value |= LEGACY_MOD_KEY3; break;
                case MOD_4K: value |= LEGACY_MOD_KEY4; break;
                case MOD_5K: value |= LEGACY_MOD_KEY5; break;
                case MOD_6K: value |= LEGACY_MOD_KEY6; break;
                case MOD_7K: value |= LEGACY_MOD_KEY7; break;
                case MOD_8K: value |= LEGACY_MOD_KEY8; break;
                case MOD_9K: value |= LEGACY_MOD_KEY9; break;
                case MOD_DS: value |= LEGACY_MOD_KEY_COOP; break;
                case MOD_FI:
                    value |= LEGACY_MOD_FADE_IN;
                    value &= ~static_cast<unsigned>(
                        LEGACY_MOD_HIDDEN); // this is toggled on in the base call due // to inheritance, but
                                            // we don't want that.
                    break;
                case MOD_MR: value |= LEGACY_MOD_MIRROR; break;
                case MOD_RD: value |= LEGACY_MOD_RANDOM; break;
                default: break;
                }
            }
        }

        return value;
    }
}} // namespace pppp::mods
