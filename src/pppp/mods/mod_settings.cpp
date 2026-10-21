// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/generated_mods.h"
#include "pppp/mods/mod.h"
#include "pppp/mods/mod_setting_value.h"
#include <cstdlib>
#include <cstring>

namespace pppp { namespace mods {

    namespace {
        void set_final_rate(Mod& mod, double value);

        void set_initial_rate(Mod& mod, double value) {
            double rate = mod_variable_rate(mod).initial_rate;
            if (mod.id == MOD_WU) {
                clamp_and_round(&rate, value, 0.5, 1.99, 0.01);
            } else {
                clamp_and_round(&rate, value, 0.51, 2, 0.01);
            }
            if (rate == mod_variable_rate(mod).initial_rate) {
                return;
            }
            mod_variable_rate(mod).initial_rate = rate;

            if (mod.id == MOD_WU) {
                if (rate >= mod_variable_rate(mod).final_rate) {
                    set_final_rate(mod, rate + 0.01);
                }
            } else {
                if (rate <= mod_variable_rate(mod).final_rate) {
                    set_final_rate(mod, rate - 0.01);
                }
            }
        }

        void set_final_rate(Mod& mod, double value) {
            double rate = mod_variable_rate(mod).final_rate;
            if (mod.id == MOD_WU) {
                clamp_and_round(&rate, value, 0.51, 2, 0.01);
            } else {
                clamp_and_round(&rate, value, 0.5, 1.99, 0.01);
            }
            if (rate == mod_variable_rate(mod).final_rate) {
                return;
            }
            mod_variable_rate(mod).final_rate = rate;

            if (mod.id == MOD_WU) {
                if (rate <= mod_variable_rate(mod).initial_rate) {
                    set_initial_rate(mod, rate - 0.01);
                }
            } else {
                if (rate >= mod_variable_rate(mod).initial_rate) {
                    set_initial_rate(mod, rate + 0.01);
                }
            }
        }
    } // namespace

    Mod::Mod()
        : id(MOD_COUNT) {
        mod_defaults(MOD_COUNT, this);
    }

    void mod_defaults(ModId id, Mod* out) {
        if (out == 0) {
            return;
        }
        out->settings = NoSettings();

        switch (id) {
        case MOD_DT:
        case MOD_NC:
            out->settings = RateSettings();
            mod_rate(*out).speed_change = 1.5;
            break;
        case MOD_HT:
        case MOD_DC:
            out->settings = RateSettings();
            mod_rate(*out).speed_change = 0.75;
            break;
        case MOD_WU:
            out->settings = VariableRateSettings();
            mod_variable_rate(*out).initial_rate = 1.0;
            mod_variable_rate(*out).final_rate = 1.5;
            break;
        case MOD_WD:
            out->settings = VariableRateSettings();
            mod_variable_rate(*out).initial_rate = 1.0;
            mod_variable_rate(*out).final_rate = 0.75;
            break;
        case MOD_AS:
            out->settings = VariableRateSettings();
            mod_variable_rate(*out).initial_rate = 1.0;
            break;
        case MOD_RD:
            out->settings = RandomSettings();
            mod_random(*out).seed = -1;
            mod_random(*out).angle_sharpness = 7.0f;
            break;
        case MOD_TP:
            out->settings = TargetSettings();
            mod_target(*out).seed = -1;
            break;
        case MOD_MG:
            out->settings = MagnetisedSettings();
            mod_magnetised(*out).strength = 0.5f;
            break;
        case MOD_GR:
            out->settings = ScaleSettings();
            mod_scale(*out).start_scale = 0.5f;
            break;
        case MOD_DF:
            out->settings = ScaleSettings();
            mod_scale(*out).start_scale = 2.0f;
            break;
        case MOD_HD: out->settings = HiddenSettings(); break;
        case MOD_CL:
            out->settings = ClassicSettings();
            mod_classic(*out).no_slider_head_accuracy = true;
            break;
        case MOD_MR:
            out->settings = MirrorSettings();
            mod_mirror(*out).axes = MOD_REFLECTION_HORIZONTAL;
            break;
        case MOD_SR:
            out->settings = SimplifiedRhythmSettings();
            mod_simplified_rhythm(*out).one_third_conversion = false;
            mod_simplified_rhythm(*out).one_sixth_conversion = true;
            mod_simplified_rhythm(*out).one_eighth_conversion = false;
            break;
        case MOD_DA: out->settings = DifficultyAdjustSettings(); break;
        default: break;
        }
    }

    int mod_make(Mod* out, ModId id) {
        if (out == 0 || id < 0 || id >= MOD_COUNT) {
            return -1;
        }
        out->id = id;
        mod_defaults(id, out);
        return 0;
    }

    int mod_apply_settings(Mod* out, const char* settings) {
        if (out == 0 || settings == 0) {
            return -1;
        }

        char buf[512];
        std::strncpy(buf, settings, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;

        for (char* kv = std::strtok(buf, ":"); kv; kv = std::strtok(0, ":")) {
            char* eq = std::strchr(kv, '=');
            if (eq == 0) {
                return -1;
            }
            *eq = 0;
            if (generated::apply_setting(out, kv, eq + 1) != 0) {
                return -1;
            }
        }

        mod_apply_setting_rules(out);
        return 0;
    }

    void mod_apply_setting_rules(Mod* mod) {
        if (mod == 0 || mod->id < 0 || mod->id >= MOD_COUNT) {
            return;
        }

        if (mod->id == MOD_WU || mod->id == MOD_WD) {
            const Mod given = *mod;
            Mod defaults;
            mod_make(&defaults, mod->id);
            mod_variable_rate(*mod).initial_rate = mod_variable_rate(defaults).initial_rate;
            mod_variable_rate(*mod).final_rate = mod_variable_rate(defaults).final_rate;
            set_initial_rate(*mod, mod_variable_rate(given).initial_rate);
            set_final_rate(*mod, mod_variable_rate(given).final_rate);
            return;
        }

        generated::clamp_settings(mod);
    }

    int mod_from_acronyms(Mod* out, size_t max_count, const char* acronyms) {
        if (out == 0 || acronyms == 0) {
            return -1;
        }
        if (!*acronyms || !std::strcmp(acronyms, "NM")) {
            return 0;
        }

        char buf[1024];
        std::strncpy(buf, acronyms, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;

        char* pieces[64];
        size_t n = 0;
        bool too_many = false;
        for (char* tok = std::strtok(buf, ","); tok; tok = std::strtok(0, ",")) {
            if (n >= sizeof(pieces) / sizeof(pieces[0])) {
                too_many = true;
                break;
            }
            pieces[n++] = tok;
        }

        if (too_many || n > max_count) {
            return -1;
        }

        const size_t MAX_ACRONYM_LENGTH = 3;
        size_t written = 0;
        for (size_t i = 0; i < n; i++) {
            char* colon = std::strchr(pieces[i], ':');
            if (colon != 0) {
                *colon = 0;
                if (written >= max_count || mod_from_acronym(&out[written], pieces[i]) != 0 ||
                    mod_apply_settings(&out[written], colon + 1) != 0) {
                    return -1;
                }
                written++;
                continue;
            }

            const char* at = pieces[i];
            while (*at) {
                Mod mod;
                size_t used = 0;
                for (size_t length = MAX_ACRONYM_LENGTH; length > 0 && used == 0; length--) {
                    if (std::strlen(at) < length) {
                        continue;
                    }
                    char acronym[MAX_ACRONYM_LENGTH + 1];
                    std::memcpy(acronym, at, length);
                    acronym[length] = 0;
                    if (mod_from_acronym(&mod, acronym) == 0) {
                        used = length;
                    }
                }
                if (used == 0 || written >= max_count) {
                    return -1;
                }
                out[written++] = mod;
                at += used;
            }
        }
        return static_cast<int>(written);
    }

}} // namespace pppp::mods
