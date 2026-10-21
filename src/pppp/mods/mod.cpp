// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mods/mod.h"
#include "pppp/mods/generated_mods.h"
#include "pppp/mods/mod_setting_value.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cstring>

namespace pppp { namespace mods {
    namespace math = ::pppp::utils::math;

    const char* mod_acronym(ModId id) {
        return (id >= 0 && id < MOD_COUNT) ? generated::mod_def(id).acronym : 0;
    }

    unsigned mod_rulesets(ModId id) {
        return (id >= 0 && id < MOD_COUNT) ? generated::mod_def(id).rulesets : 0;
    }

    const char* mod_name(ModId id) { return (id >= 0 && id < MOD_COUNT) ? generated::mod_def(id).name : 0; }

    bool mod_incompatible(ModId id, ModId other) {
        if (id < 0 || id >= MOD_COUNT || other < 0 || other >= MOD_COUNT) {
            return 0;
        }
        const generated::ModDefRow& def = generated::mod_def(id);

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
            if (std::strcmp(generated::mod_def(static_cast<ModId>(i)).acronym, acronym) == 0) {
                return mod_make(out, static_cast<ModId>(i));
            }
        }
        return -1;
    }

    namespace {
        double time_ramp_apply_to_rate(const Mod& mod, double time, double rate) {
            double amount = (time - mod.time_ramp.begin_ramp_time) /
                            std::max(1.0, mod.time_ramp.final_rate_time - mod.time_ramp.begin_ramp_time);
            double ramp = mod_variable_rate(mod).initial_rate +
                          (mod_variable_rate(mod).final_rate - mod_variable_rate(mod).initial_rate) *
                              math::clamp(amount, 0, 1);

            // round the end result to match SpeedChange's precision, in case this is called
            // externally.
            return rate * math::round2_to_even(ramp);
        }

    } // namespace

    double mod_calculate_rate(const Mod* mods, size_t mod_count) {
        double rate = 1.0;
        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case MOD_DT:
            case MOD_NC:
            case MOD_HT:
            case MOD_DC: rate *= mod_rate(mods[i]).speed_change; break;
            case MOD_WU:
            case MOD_WD: rate = time_ramp_apply_to_rate(mods[i], 0, rate); break;
            case MOD_AS: rate *= mod_variable_rate(mods[i]).initial_rate; break;
            default: break;
            }
        }
        return rate;
    }

    void mod_time_ramp_apply_to_beatmap(Mod* mod, bool any_objects, double first_object_start,
                                        double last_object_end) {
        double first_object = any_objects ? first_object_start : 0;
        double last_object = any_objects ? last_object_end : 0;

        mod->time_ramp.begin_ramp_time = first_object;
        mod->time_ramp.final_rate_time = first_object + FINAL_RATE_PROGRESS * (last_object - first_object);
    }

    void mod_apply_to_difficulty(const Mod* mods, size_t mod_count, DifficultySettings* settings) {
        if (settings == 0) {
            return;
        }

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case MOD_DA: {
                const DifficultyAdjustSettings& st = mod_difficulty_adjust(mods[i]);
                if (st.drain_rate.has_value()) {
                    settings->drain_rate = clamp_to_extended_limits(st.drain_rate.value(), 0, 10,
                                                                    nonstd::optional<float>(), 11.0f);
                }
                if (st.overall_difficulty.has_value()) {
                    settings->overall_difficulty = clamp_to_extended_limits(
                        st.overall_difficulty.value(), 0, 10, nonstd::optional<float>(), 11.0f);
                }
                if (st.circle_size.has_value()) {
                    settings->circle_size = clamp_to_extended_limits(st.circle_size.value(), 0, 10,
                                                                     nonstd::optional<float>(), 11.0f);
                }
                if (st.approach_rate.has_value()) {
                    settings->approach_rate =
                        clamp_to_extended_limits(st.approach_rate.value(), 0, 10, -10.0f, 11.0f);
                }
                break;
            }
            case MOD_HR:
                settings->drain_rate = std::min(settings->drain_rate * HARD_ROCK_ADJUST_RATIO, 10.0f);

                settings->overall_difficulty =
                    std::min(settings->overall_difficulty * HARD_ROCK_ADJUST_RATIO, 10.0f);
                settings->circle_size =
                    std::min(settings->circle_size * 1.3f, 10.0f); // CS uses a custom 1.3 ratio.
                settings->approach_rate = std::min(settings->approach_rate * HARD_ROCK_ADJUST_RATIO, 10.0f);
                break;
            case MOD_TP:
                // Decrease AR to increase preempt time
                settings->approach_rate *= 0.5f;
                break;
            case MOD_EZ:
                settings->circle_size *= EASY_ADJUST_RATIO;
                settings->approach_rate *= EASY_ADJUST_RATIO;
                settings->drain_rate *= EASY_ADJUST_RATIO;

                settings->overall_difficulty *= EASY_ADJUST_RATIO;
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
                const int mirror = mod_mirror(mods[i]).axes;
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
}} // namespace pppp::mods
