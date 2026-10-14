// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/mod.h"
#include "pppp/mods/mod_setting_value.h"
#include "pppp/utils/parse.h"
#include <cstdlib>
#include <cstring>

namespace pppp { namespace mods {

    namespace {
        struct IgnoredSetting {
            ModId id;
            const char* key;
            bool is_bool;
        };

        const IgnoredSetting IGNORED_SETTINGS[] = {
            {MOD_EZ, "retries", false},
            {MOD_HT, "adjust_pitch", true},
            {MOD_DT, "adjust_pitch", true},
            {MOD_SD, "restart", true},
            {MOD_SD, "fail_on_slider_tail", true},
            {MOD_PF, "restart", true},
            {MOD_PF, "require_perfect_hits", true},
            {MOD_FL, "size_multiplier", false},
            {MOD_FL, "combo_based_size", true},
            {MOD_FL, "follow_delay", false},
            {MOD_AC, "restart", true},
            {MOD_AC, "minimum_accuracy", false},
            {MOD_AC, "accuracy_judge_mode", false},
            {MOD_TP, "metronome", true},
            {MOD_DA, "extended_limits", true},
            {MOD_CL, "classic_note_lock", true},
            {MOD_CL, "always_play_tail_sample", true},
            {MOD_CL, "fade_hit_circle_early", true},
            {MOD_CL, "classic_health", true},
            {MOD_WG, "strength", false},
            {MOD_WU, "adjust_pitch", true},
            {MOD_WD, "adjust_pitch", true},
            {MOD_BR, "spin_speed", false},
            {MOD_BR, "direction", false},
            {MOD_AD, "scale", false},
            {MOD_AD, "style", false},
            {MOD_MU, "inverse_muting", true},
            {MOD_MU, "enable_metronome", true},
            {MOD_MU, "mute_combo_count", false},
            {MOD_MU, "affects_hit_sounds", true},
            {MOD_NS, "hidden_combo_count", false},
            {MOD_RP, "repulsion_strength", false},
            {MOD_AS, "adjust_pitch", true},
            {MOD_DP, "max_depth", false},
            {MOD_DP, "show_approach_circles", true},
            {MOD_BM, "max_size_combo_count", false},
            {MOD_BM, "max_cursor_size", false},
            {MOD_CO, "coverage", false},
            {MOD_CO, "direction", false},
        };

        int apply_ignored_setting(ModId id, const char* key, const char* val) {
            for (size_t i = 0; i < sizeof(IGNORED_SETTINGS) / sizeof(IGNORED_SETTINGS[0]); i++) {
                const IgnoredSetting& setting = IGNORED_SETTINGS[i];
                if (setting.id != id || std::strcmp(setting.key, key) != 0) {
                    continue;
                }
                bool b;
                return setting.is_bool ? utils::parse_bool(val, &b) : 0;
            }
            return -1;
        }

        void set_final_rate(Mod& mod, double value);

        void set_initial_rate(Mod& mod, double value) {
            double rate = mod.variable_rate.initial_rate;
            if (mod.id == MOD_WU) {
                clamp_and_round(&rate, value, 0.5, 1.99, 0.01);
            } else {
                clamp_and_round(&rate, value, 0.51, 2, 0.01);
            }
            if (rate == mod.variable_rate.initial_rate) {
                return;
            }
            mod.variable_rate.initial_rate = rate;

            if (mod.id == MOD_WU) {
                if (rate >= mod.variable_rate.final_rate) {
                    set_final_rate(mod, rate + 0.01);
                }
            } else {
                if (rate <= mod.variable_rate.final_rate) {
                    set_final_rate(mod, rate - 0.01);
                }
            }
        }

        void set_final_rate(Mod& mod, double value) {
            double rate = mod.variable_rate.final_rate;
            if (mod.id == MOD_WU) {
                clamp_and_round(&rate, value, 0.51, 2, 0.01);
            } else {
                clamp_and_round(&rate, value, 0.5, 1.99, 0.01);
            }
            if (rate == mod.variable_rate.final_rate) {
                return;
            }
            mod.variable_rate.final_rate = rate;

            if (mod.id == MOD_WU) {
                if (rate <= mod.variable_rate.initial_rate) {
                    set_initial_rate(mod, rate - 0.01);
                }
            } else {
                if (rate >= mod.variable_rate.initial_rate) {
                    set_initial_rate(mod, rate + 0.01);
                }
            }
        }

        int apply_setting(Mod* mod, const char* key, const char* val) {
            Mod& st = *mod;
            bool b;

            switch (mod->id) {
            case MOD_DT:
            case MOD_NC:
            case MOD_HT:
            case MOD_DC:
                if (!std::strcmp(key, "speed_change")) {
                    st.rate.speed_change = std::atof(val);
                    return 0;
                }
                break;

            case MOD_WU:
            case MOD_WD:
                if (!std::strcmp(key, "initial_rate")) {
                    st.variable_rate.initial_rate = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "final_rate")) {
                    st.variable_rate.final_rate = std::atof(val);
                    return 0;
                }
                break;

            case MOD_AS:
                if (!std::strcmp(key, "initial_rate")) {
                    st.variable_rate.initial_rate = std::atof(val);
                    return 0;
                }
                break;

            case MOD_RD:
                if (!std::strcmp(key, "seed")) {
                    st.random.seed = std::atoi(val);
                    return 0;
                }
                if (!std::strcmp(key, "angle_sharpness")) {
                    st.random.angle_sharpness = static_cast<float>(std::atof(val));
                    return 0;
                }
                break;

            case MOD_TP:
                if (!std::strcmp(key, "seed")) {
                    st.target.seed = std::atoi(val);
                    return 0;
                }
                break;

            case MOD_MG:
                if (!std::strcmp(key, "attraction_strength")) {
                    st.magnetised.strength = static_cast<float>(std::atof(val));
                    return 0;
                }
                break;

            case MOD_GR:
            case MOD_DF:
                if (!std::strcmp(key, "start_scale")) {
                    st.scale.start_scale = static_cast<float>(std::atof(val));
                    return 0;
                }
                break;

            case MOD_HD:
                if (!std::strcmp(key, "only_fade_approach_circles")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.hidden.only_fade_approach_circles = b;
                    return 0;
                }
                break;

            case MOD_CL:
                if (!std::strcmp(key, "no_slider_head_accuracy")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.classic.no_slider_head_accuracy = b;
                    return 0;
                }
                break;

            case MOD_MR:
                if (!std::strcmp(key, "reflection")) {
                    if (!std::strcmp(val, "Horizontal") || !std::strcmp(val, "0")) {
                        st.mirror.axes = MOD_REFLECTION_HORIZONTAL;
                    } else if (!std::strcmp(val, "Vertical") || !std::strcmp(val, "1")) {
                        st.mirror.axes = MOD_REFLECTION_VERTICAL;
                    } else if (!std::strcmp(val, "Both") || !std::strcmp(val, "2")) {
                        st.mirror.axes = MOD_REFLECTION_BOTH;
                    } else {
                        return -1;
                    }
                    return 0;
                }
                break;

            case MOD_SR:
                if (!std::strcmp(key, "one_third_conversion")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.simplified_rhythm.one_third_conversion = b;
                    return 0;
                }
                if (!std::strcmp(key, "one_sixth_conversion")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.simplified_rhythm.one_sixth_conversion = b;
                    return 0;
                }
                if (!std::strcmp(key, "one_eighth_conversion")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.simplified_rhythm.one_eighth_conversion = b;
                    return 0;
                }
                break;
            case MOD_DA:
                if (!std::strcmp(key, "circle_size")) {
                    st.difficulty_adjust.circle_size = static_cast<float>(std::atof(val));
                    return 0;
                }
                if (!std::strcmp(key, "approach_rate")) {
                    st.difficulty_adjust.approach_rate = static_cast<float>(std::atof(val));
                    return 0;
                }
                if (!std::strcmp(key, "drain_rate")) {
                    st.difficulty_adjust.drain_rate = static_cast<float>(std::atof(val));
                    return 0;
                }
                if (!std::strcmp(key, "overall_difficulty")) {
                    st.difficulty_adjust.overall_difficulty = static_cast<float>(std::atof(val));
                    return 0;
                }
                if (!std::strcmp(key, "scroll_speed")) {
                    st.difficulty_adjust.scroll_speed = static_cast<float>(std::atof(val));
                    return 0;
                }
                if (!std::strcmp(key, "hard_rock_offsets")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.difficulty_adjust.hard_rock_offsets = b;
                    return 0;
                }
                break;

            default: break;
            }

            return apply_ignored_setting(mod->id, key, val);
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
        out->rate = RateSettings();
        out->variable_rate = VariableRateSettings();
        out->random = RandomSettings();
        out->target = TargetSettings();
        out->magnetised = MagnetisedSettings();
        out->scale = ScaleSettings();
        out->hidden = HiddenSettings();
        out->classic = ClassicSettings();
        out->mirror = MirrorSettings();
        out->simplified_rhythm = SimplifiedRhythmSettings();
        out->difficulty_adjust = DifficultyAdjustSettings();

        switch (id) {
        case MOD_DT:
        case MOD_NC: out->rate.speed_change = 1.5; break;
        case MOD_HT:
        case MOD_DC: out->rate.speed_change = 0.75; break;
        case MOD_WU:
            out->variable_rate.initial_rate = 1.0;
            out->variable_rate.final_rate = 1.5;
            break;
        case MOD_WD:
            out->variable_rate.initial_rate = 1.0;
            out->variable_rate.final_rate = 0.75;
            break;
        case MOD_AS: out->variable_rate.initial_rate = 1.0; break;
        case MOD_RD:
            out->random.seed = -1;
            out->random.angle_sharpness = 7.0f;
            break;
        case MOD_TP: out->target.seed = -1; break;
        case MOD_MG: out->magnetised.strength = 0.5f; break;
        case MOD_GR: out->scale.start_scale = 0.5f; break;
        case MOD_DF: out->scale.start_scale = 2.0f; break;
        case MOD_CL: out->classic.no_slider_head_accuracy = true; break;
        case MOD_MR: out->mirror.axes = MOD_REFLECTION_HORIZONTAL; break;
        case MOD_SR:
            out->simplified_rhythm.one_third_conversion = false;
            out->simplified_rhythm.one_sixth_conversion = true;
            out->simplified_rhythm.one_eighth_conversion = false;
            break;
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
            if (apply_setting(out, kv, eq + 1) != 0) {
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

        const Mod given = *mod;
        Mod defaults;
        mod_make(&defaults, mod->id);

        switch (mod->id) {
        case MOD_DT:
        case MOD_NC:
            mod->rate.speed_change = defaults.rate.speed_change;
            clamp_and_round(&mod->rate.speed_change, given.rate.speed_change, 1.01, 2, 0.01);
            break;
        case MOD_HT:
        case MOD_DC:
            mod->rate.speed_change = defaults.rate.speed_change;
            clamp_and_round(&mod->rate.speed_change, given.rate.speed_change, 0.5, 0.99, 0.01);
            break;
        case MOD_WU:
        case MOD_WD:
            mod->variable_rate.initial_rate = defaults.variable_rate.initial_rate;
            mod->variable_rate.final_rate = defaults.variable_rate.final_rate;
            set_initial_rate(*mod, given.variable_rate.initial_rate);
            set_final_rate(*mod, given.variable_rate.final_rate);
            break;
        case MOD_AS:
            mod->variable_rate.initial_rate = defaults.variable_rate.initial_rate;
            clamp_and_round(&mod->variable_rate.initial_rate, given.variable_rate.initial_rate, 0.5, 2, 0.01);
            break;
        case MOD_RD:
            mod->random.angle_sharpness = defaults.random.angle_sharpness;
            clamp_and_round(&mod->random.angle_sharpness, given.random.angle_sharpness, 1.0f, 10.0f, 0.1f);
            break;
        case MOD_MG:
            mod->magnetised.strength = defaults.magnetised.strength;
            clamp_and_round(&mod->magnetised.strength, given.magnetised.strength, 0.05f, 1.0f, 0.05f);
            break;
        case MOD_GR:
            mod->scale.start_scale = defaults.scale.start_scale;
            clamp_and_round(&mod->scale.start_scale, given.scale.start_scale, 0.0f, 0.99f, 0.01f);
            break;
        case MOD_DF:
            mod->scale.start_scale = defaults.scale.start_scale;
            clamp_and_round(&mod->scale.start_scale, given.scale.start_scale, 1.0f, 25.0f, 0.1f);
            break;
        default: break;
        }
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
        for (char* tok = std::strtok(buf, ","); tok && n < 64; tok = std::strtok(0, ",")) {
            pieces[n++] = tok;
        }

        if (n > max_count) {
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
