// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/mod.h"
#include "pppp/utils/parse.h"
#include <cstdlib>
#include <cstring>

namespace pppp { namespace mods {

    namespace {
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
                if (!std::strcmp(key, "adjust_pitch")) {
                    return utils::parse_bool(val, &b); // audio only
                }
                break;

            case MOD_WU:
            case MOD_WD:
                if (!std::strcmp(key, "initial_rate")) {
                    st.variable_rate.initial_rate = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "final_rate")) {
                    return 0;
                }
                if (!std::strcmp(key, "adjust_pitch")) {
                    return utils::parse_bool(val, &b); // audio only
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
                    st.random.angle_sharpness = std::atof(val);
                    return 0;
                }
                break;

            case MOD_TP:
                if (!std::strcmp(key, "seed")) {
                    st.target.seed = std::atoi(val);
                    return 0;
                }
                if (!std::strcmp(key, "metronome")) {
                    return utils::parse_bool(val, &b); // audio only
                }
                break;

            case MOD_MG:
                if (!std::strcmp(key, "attraction_strength")) {
                    st.magnetised.strength = std::atof(val);
                    return 0;
                }
                break;

            case MOD_GR:
            case MOD_DF:
                if (!std::strcmp(key, "start_scale")) {
                    st.scale.start_scale = std::atof(val);
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
                    st.difficulty_adjust.circle_size = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "approach_rate")) {
                    st.difficulty_adjust.approach_rate = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "drain_rate")) {
                    st.difficulty_adjust.drain_rate = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "overall_difficulty")) {
                    st.difficulty_adjust.overall_difficulty = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "scroll_speed")) {
                    st.difficulty_adjust.scroll_speed = std::atof(val);
                    return 0;
                }
                if (!std::strcmp(key, "hard_rock_offsets")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.difficulty_adjust.hard_rock_offsets = b;
                    return 0;
                }
                if (!std::strcmp(key, "extended_limits")) {
                    if (utils::parse_bool(val, &b)) {
                        return -1;
                    }
                    st.difficulty_adjust.extended_limits = b;
                    return 0;
                }
                break;

            case MOD_MU:
                if (!std::strcmp(key, "metronome")) {
                    return utils::parse_bool(val, &b); // audio only
                }
                break;

            default: break;
            }

            return -1;
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
        case MOD_WD:
        case MOD_AS: out->variable_rate.initial_rate = 1.0; break;
        case MOD_RD:
            out->random.seed = -1;
            out->random.angle_sharpness = 7.0;
            break;
        case MOD_TP: out->target.seed = -1; break;
        case MOD_MG: out->magnetised.strength = 0.5; break;
        case MOD_GR: out->scale.start_scale = 0.5; break;
        case MOD_DF: out->scale.start_scale = 2.0; break;
        case MOD_CL: out->classic.no_slider_head_accuracy = true; break;
        case MOD_MR: out->mirror.axes = MOD_REFLECTION_HORIZONTAL; break;
        case MOD_DA:
            // negative means leave the beatmap's value alone
            out->difficulty_adjust.circle_size = -1.0;
            out->difficulty_adjust.approach_rate = -1.0;
            out->difficulty_adjust.drain_rate = -1.0;
            out->difficulty_adjust.overall_difficulty = -1.0;
            out->difficulty_adjust.scroll_speed = -1.0;
            out->difficulty_adjust.hard_rock_offsets = false;
            break;
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
        return 0;
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

            // a run of acronyms written together, the way osu! names a combination
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
