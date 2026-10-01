// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/legacy_score_simulator.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/math/legacy_score.h"
#include "pppp/utils/math/uint128.h"

namespace pppp { namespace osu { namespace difficulty {
    LegacyScoreAttributes::LegacyScoreAttributes()
        : accuracy_score(0.0),
          combo_score(0.0),
          bonus_score_ratio(0.0),
          bonus_score(0),
          max_combo(0) {}

    namespace {
        double calculate_spinner_score(double duration_ms) {
            const int spin_score = 100;
            const int bonus_spin_score = 1000;

            // The spinner object applies a lenience because gameplay mechanics differ from osu-stable.
            // We'll redo the calculations to match osu-stable here...
            double seconds = duration_ms / 1000;

            // The total amount of half spins possible for the entire spinner.
            int total_half_spins_possible = static_cast<int>(seconds * MAXIMUM_ROTATIONS_PER_SECOND * 2);
            // The amount of half spins that are required to successfully complete the spinner (i.e. get a
            // 300).
            int half_spins_required_for_completion = static_cast<int>(seconds * MINIMUM_ROTATIONS_PER_SECOND);
            // To be able to receive bonus points, the spinner must be rotated another 1.5 times.
            int half_spins_required_before_bonus = half_spins_required_for_completion + 3;
            pppp_int64 score = 0;
            int full_spins = total_half_spins_possible / 2;

            // Normal spin score
            score += static_cast<pppp_int64>(spin_score) * full_spins;
            int bonus_spins = (total_half_spins_possible - half_spins_required_before_bonus) / 2;

            // Reduce amount of bonus spins because we want to represent the more average case, rather than
            // the best one.
            bonus_spins = bonus_spins > full_spins / 2 ? bonus_spins - full_spins / 2 : 0;
            if (bonus_spins < 0) {
                bonus_spins = 0;
            }
            score += static_cast<pppp_int64>(bonus_spin_score) * bonus_spins;
            return static_cast<double>(score);
        }
    } // namespace

    double calculate_nested_score_per_object(const OsuBeatmap& pb, int object_count) {
        const double big_tick_score = 30;
        const double small_tick_score = 10;
        int big_ticks = 0, small_ticks = 0;
        double spinner_score = 0;
        for (size_t i = 0; i < pb.objects.size(); i++) {
            const object::OsuHitObject& ho = pb.objects[i];
            if (ho.slider >= 0) {
                const object::PlayableSlider& sl = pb.sliders[ho.slider];
                // 1 for head, 1 for tail, plus repeats
                big_ticks += 2 + (sl.slides - 1);
                for (size_t e = 0; e < sl.events.size(); e++) {
                    if (sl.events[e].type == pppp::beatmaps::SLIDER_EVENT_TICK) {
                        small_ticks++;
                    }
                }
            } else if (ho.type & 8) {
                spinner_score += calculate_spinner_score(ho.end_time - ho.time);
            }
        }
        double slider_score = big_ticks * big_tick_score + small_ticks * small_tick_score;
        return (slider_score + spinner_score) / object_count;
    }

    namespace {
        int calculate_difficulty_peppy_stars(double drain_rate, double overall_difficulty, double circle_size,
                                             int object_count, int drain_length) {
            // WARNING: DO NOT TOUCH IF YOU DO NOT KNOW WHAT YOU ARE DOING
            //
            // It so happens that in stable, due to .NET Framework internals, float math would be performed
            // using x87 registers and opcodes.
            // .NET (Core) however uses SSE instructions on 32- and 64-bit words.
            // x87 registers are _80 bits_ wide. Which is notably wider than _both_ float and double.
            // Therefore, on a significant number of beatmaps, the rounding would not produce correct values.
            //
            // Thus, to crudely - but, seemingly *mostly* accurately, after checking across all ranked maps -
            // emulate this, use `decimal`, which is slow, but has bigger precision than `double`. At the time
            // of writing, there is _one_ ranked exception to this - namely
            // https://osu.ppy.sh/beatmapsets/1156087#osu/2625853 - but it is considered an "acceptable
            // casualty", since in that case scores aren't inflated by _that_ much compared to others.

            const pppp_uint64 SCALE = static_cast<pppp_uint64>(100000) * static_cast<pppp_uint64>(1000000000);

            pppp_uint64 ratio_num, ratio_den;
            if (drain_length == 0) {
                ratio_num = 16;
                ratio_den = 1;
            } else if (drain_length < 0) {
                ratio_num = 0;
                ratio_den = 1;
            } else {
                ratio_num = static_cast<pppp_uint64>(object_count) * 8;
                ratio_den = static_cast<pppp_uint64>(drain_length);
                if (ratio_num > 16 * ratio_den) {
                    ratio_num = 16;
                    ratio_den = 1;
                }
            }

            // Notably, THE `double` CASTS BELOW ARE IMPORTANT AND MUST REMAIN.
            // Their goal is to trick the compiler / runtime into NOT promoting from single-precision float,
            // as doing so would prompt it to attempt to "silently" fix the single-precision values when
            // converting to decimal, which is NOT what the x87 FPU does.

            double values[3] = {drain_rate, overall_difficulty, circle_size};
            utils::Uint128 positive = utils::math::u128_from(0), negative = utils::math::u128_from(0);
            for (int i = 0; i < 3; i++) {
                bool neg = false;
                pppp_uint64 magnitude = utils::math::decimal15_scaled(values[i], &neg);
                if (neg) {
                    negative = utils::math::u128_add(negative, utils::math::u128_from(magnitude));
                } else {
                    positive = utils::math::u128_add(positive, utils::math::u128_from(magnitude));
                }
            }

            utils::Uint128 a = utils::math::u128_add(utils::math::u128_mul(positive, ratio_den),
                                                     utils::math::u128_mul64(ratio_num, SCALE));
            utils::Uint128 b = utils::math::u128_mul(negative, ratio_den);
            if (utils::math::u128_cmp(b, a) > 0) {
                return 0;
            }
            utils::Uint128 num = utils::math::u128_mul(utils::math::u128_sub(a, b), 5);
            utils::Uint128 den = utils::math::u128_mul(utils::math::u128_mul64(SCALE, ratio_den), 38);
            utils::Uint128 q, rem;
            utils::math::u128_divmod(num, den, &q, &rem);

            utils::Uint128 twice_rem = utils::math::u128_shl1(rem);
            int cmp = utils::math::u128_cmp(twice_rem, den);
            if (cmp > 0 || (cmp == 0 && utils::math::u128_is_odd(q))) {
                q = utils::math::u128_add(q, utils::math::u128_from(1));
            }
            return static_cast<int>(q.lo);
        }

        int raw_drain_length(const pppp::beatmaps::Beatmap& beatmap) {
            if (beatmap.hit_objects.empty()) {
                return 0;
            }
            int break_length = 0;
            for (size_t b = 0; b < beatmap.breaks.size(); b++) {
                double start = beatmap.breaks[b].start_time;
                double end = beatmap.breaks[b].end_time > start ? beatmap.breaks[b].end_time
                                                                : start; // decoder: end = max(start, end)
                break_length +=
                    utils::math::round_half_even_int(end) - utils::math::round_half_even_int(start);
            }
            return (utils::math::round_half_even_int(beatmap.hit_objects.back().start_time) -
                    utils::math::round_half_even_int(beatmap.hit_objects.front().start_time) - break_length) /
                   1000;
        }
    } // namespace

    int peppy_stars(const pppp::beatmaps::Beatmap& beatmap) {
        int object_count = static_cast<int>(beatmap.hit_objects.size());
        int drain_length = raw_drain_length(beatmap);

        return calculate_difficulty_peppy_stars(static_cast<float>(beatmap.difficulty.drain_rate),
                                                static_cast<float>(beatmap.difficulty.overall_difficulty),
                                                static_cast<float>(beatmap.difficulty.circle_size),
                                                object_count, drain_length);
    }

    namespace {
        struct Simulator {
            int legacy_bonus_score;
            int standardised_bonus_score;
            int combo;
            double score_multiplier;
            LegacyScoreAttributes attributes;

            Simulator()
                : legacy_bonus_score(0),
                  standardised_bonus_score(0),
                  combo(0),
                  score_multiplier(0.0) {}

            void simulate_hit(int score_increase, bool increase_combo, bool add_score_combo_multiplier,
                              bool is_bonus, int standardised_bonus) {
                if (add_score_combo_multiplier) {
                    int c = combo - 1 > 0 ? combo - 1 : 0;
                    // NOLINTNEXTLINE(bugprone-integer-division)
                    attributes.combo_score += static_cast<int>(c * (score_increase / 25 * score_multiplier));
                }
                if (is_bonus) {
                    legacy_bonus_score += score_increase;
                    standardised_bonus_score += standardised_bonus;
                } else {
                    attributes.accuracy_score += score_increase;
                }
                if (increase_combo) {
                    combo++;
                }
            }

            void spinner(double duration_ms) {
                // The spinner object applies a lenience because gameplay mechanics differ from osu-stable.
                // We'll redo the calculations to match osu-stable here...
                double seconds = duration_ms / 1000;
                // The total amount of half spins possible for the entire spinner.
                int total_half_spins_possible = static_cast<int>(seconds * MAXIMUM_ROTATIONS_PER_SECOND * 2);
                // The amount of half spins that are required to successfully complete the spinner (i.e. get a
                // 300).
                int half_spins_required_for_completion =
                    static_cast<int>(seconds * MINIMUM_ROTATIONS_PER_SECOND);
                // To be able to receive bonus points, the spinner must be rotated another 1.5 times.
                int half_spins_required_before_bonus = half_spins_required_for_completion + 3;
                for (int i = 0; i <= total_half_spins_possible; i++) {
                    if (i > half_spins_required_before_bonus &&
                        (i - half_spins_required_before_bonus) % 2 == 0) {
                        simulate_hit(1100, false, false, true, 50);
                    } else if (i > 1 && i % 2 == 0) {
                        simulate_hit(100, false, false, true, 10);
                    }
                }
                simulate_hit(300, true, true, false, 0);
            }
        };
    } // namespace

    LegacyScoreAttributes simulate(const pppp::beatmaps::Beatmap& beatmap, const OsuBeatmap& pb) {
        Simulator sim;
        sim.score_multiplier = peppy_stars(beatmap);

        for (size_t i = 0; i < pb.objects.size(); i++) {
            const object::OsuHitObject& ho = pb.objects[i];
            if (ho.slider >= 0) {
                const object::PlayableSlider& sl = pb.sliders[ho.slider];
                for (size_t e = 0; e < sl.events.size(); e++) {
                    switch (sl.events[e].type) {
                    case pppp::beatmaps::SLIDER_EVENT_HEAD:
                    case pppp::beatmaps::SLIDER_EVENT_TAIL:
                    case pppp::beatmaps::SLIDER_EVENT_REPEAT:
                        sim.simulate_hit(30, true, false, false, 0);
                        break;
                    case pppp::beatmaps::SLIDER_EVENT_TICK:
                        sim.simulate_hit(10, true, false, false, 0);
                        break;
                    case pppp::beatmaps::SLIDER_EVENT_LEGACY_LAST_TICK: break;
                    }
                }
                sim.simulate_hit(300, false, true, false, 0);
            } else if (ho.type & 8) {
                sim.spinner(ho.end_time - ho.time);
            } else {
                sim.simulate_hit(300, true, true, false, 0);
            }
        }
        sim.attributes.bonus_score_ratio =
            sim.legacy_bonus_score == 0
                ? 0
                : static_cast<double>(sim.standardised_bonus_score) / sim.legacy_bonus_score;
        sim.attributes.bonus_score = sim.legacy_bonus_score;
        sim.attributes.max_combo = sim.combo;
        return sim.attributes;
    }

    double get_legacy_score_multiplier(const pppp::mods::Mod* mods, size_t mod_count) {
        bool score_v2 = pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_SV2);

        double multiplier = 1.0;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_NF: multiplier *= score_v2 ? 1.0 : 0.5; break;
            case pppp::mods::MOD_EZ: multiplier *= 0.5; break;
            case pppp::mods::MOD_HT:
            case pppp::mods::MOD_DC: multiplier *= 0.3; break;
            case pppp::mods::MOD_HD: multiplier *= 1.06; break;
            case pppp::mods::MOD_HR: multiplier *= score_v2 ? 1.10 : 1.06; break;
            case pppp::mods::MOD_DT:
            case pppp::mods::MOD_NC: multiplier *= score_v2 ? 1.20 : 1.12; break;
            case pppp::mods::MOD_FL: multiplier *= 1.12; break;
            case pppp::mods::MOD_SO: multiplier *= 0.9; break;
            case pppp::mods::MOD_RX:
            case pppp::mods::MOD_AP: return 0;
            default: break;
            }
        }

        return multiplier;
    }
}}} // namespace pppp::osu::difficulty
