// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/taiko_performance_calculator.h"
#include "pppp/config.h" // IWYU pragma: export
#include "pppp/taiko/taiko_beatmap.h"
#include "pppp/taiko/taiko_hit_windows.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace taiko { namespace difficulty {
    namespace {
        struct TaikoPerformanceCalculator {
            const pppp::common::ScoreInfo& score;
            const TaikoDifficultyAttributes& attributes;

            int count_great;
            int count_ok;
            int count_meh;
            int count_miss;
            nonstd::optional<double> estimated_unstable_rate;
            double clock_rate;
            double great_hit_window;
            double total_difficult_hits;

            TaikoPerformanceCalculator(const pppp::common::ScoreInfo& s, const TaikoDifficultyAttributes& a)
                : score(s),
                  attributes(a),
                  count_great(0),
                  count_ok(0),
                  count_meh(0),
                  count_miss(0),
                  estimated_unstable_rate(),
                  clock_rate(0.0),
                  great_hit_window(0.0),
                  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
                  total_difficult_hits(0.0) {}

            int total_hits() const { return count_great + count_ok + count_meh + count_miss; }

            double compute_deviation_upper_bound(double accuracy) const {
                // 99% critical value for the normal distribution (one-tailed).
                const double z = 2.32634787404;

                double n = total_hits();

                // Proportion of greats hit.
                double p = accuracy;

                // We can be 99% confident that p is at least this value.
                double p_lower_bound = (n * p + z * z / 2) / (n + z * z) -
                                       z / (n + z * z) * std::sqrt(n * p * (1 - p) + z * z / 4);

                // We can be 99% confident that the deviation is not higher than:
                return great_hit_window / (pppp::utils::SQRT2 * pppp::utils::erf_inv(p_lower_bound));
            }

            double compute_difficulty_value(bool is_convert, bool is_classic) const {
                if (!estimated_unstable_rate.has_value() || total_difficult_hits == 0) {
                    return 0.0;
                }

                // The estimated unstable rate for 100% accuracy, at which all rhythm difficulty has been
                // played successfully.
                double rhythm_expected_unstable_rate = compute_deviation_upper_bound(1.0) * 10.0;

                // The unstable rate at which it can be assumed all rhythm difficulty has been ignored.
                // 0.8 represents 80% of total hits being greats, or 90% accuracy in-game
                double rhythm_maximum_unstable_rate = compute_deviation_upper_bound(0.8) * 10.0;

                // The fraction of star rating made up by rhythm difficulty, normalised to represent
                // rhythm's perceived contribution to star rating.
                double rhythm_factor = pppp::utils::reverse_lerp(
                    attributes.rhythm_difficulty / attributes.star_rating, 0.15, 0.4);

                // A penalty removing improperly played rhythm difficulty from star rating based on estimated
                // unstable rate.
                double rhythm_penalty =
                    1.0 - pppp::utils::logistic(
                              estimated_unstable_rate.value(),
                              (rhythm_expected_unstable_rate + rhythm_maximum_unstable_rate) / 2.0,
                              10.0 / (rhythm_maximum_unstable_rate - rhythm_expected_unstable_rate),
                              0.25 * pppp::utils::pow(rhythm_factor, 3));

                double base_difficulty =
                    5.0 * std::max(1.0, attributes.star_rating * rhythm_penalty / 0.110) - 4.0;
                double difficulty_value = std::min(pppp::utils::pow(base_difficulty, 3) / 69052.51,
                                                   pppp::utils::pow(base_difficulty, 2.25) / 1250.0);

                difficulty_value *= 1.0 + 0.10 * std::max(0.0, attributes.star_rating - 10.0);

                // Applies a bonus to maps with more total difficulty.
                double length_bonus = 1.0 + 0.25 * total_difficult_hits / (total_difficult_hits + 4000.0);
                difficulty_value *= length_bonus;

                // Scales miss penalty by the total difficult hits of a map, making misses more punishing on
                // maps with less total difficulty.
                double miss_penalty = 0.97 + 0.03 * total_difficult_hits / (total_difficult_hits + 1500.0);
                difficulty_value *= pppp::utils::pow(miss_penalty, count_miss);

                if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_HD)) {
                    double hidden_bonus = is_convert ? 0.025 : 0.1;

                    // Hidden+flashlight plays are excluded from reading-based penalties to hidden.
                    if (!pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_FL)) {
                        // A penalty is applied to the bonus for hidden on non-classic scores, as the
                        // playfield can be made wider to make fast reading easier.
                        if (!is_classic) {
                            hidden_bonus *= 0.2;
                        }
                        // A penalty is applied to classic easy+hidden scores, as notes disappear later making
                        // fast reading easier.
                        if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_EZ) &&
                            is_classic) {
                            hidden_bonus *= 0.5;
                        }
                    }

                    difficulty_value *= 1.0 + hidden_bonus;
                }

                if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_FL)) {
                    difficulty_value *= std::max(
                        1.0, 1.050 - std::min(attributes.mono_stamina_factor / 50.0, 1.0) * length_bonus);
                }

                // Scale accuracy more harshly on nearly-completely mono (single coloured) speed maps.
                double mono_acc_scaling_exponent = 2.0 + attributes.mono_stamina_factor;
                double mono_acc_scaling_shift = 500.0 - 100.0 * (attributes.mono_stamina_factor * 3.0);

                return difficulty_value *
                       pppp::utils::pow(
                           pppp::utils::erf(mono_acc_scaling_shift /
                                            (pppp::utils::SQRT2 * estimated_unstable_rate.value())),
                           mono_acc_scaling_exponent);
            }

            double compute_accuracy_value(bool is_convert) const {
                if (great_hit_window <= 0 || !estimated_unstable_rate.has_value()) {
                    return 0.0;
                }

                double accuracy_value = 470.0 * pppp::utils::pow(0.9885, estimated_unstable_rate.value());

                // Scales up the bonus for lower unstable rate as star rating increases.
                accuracy_value *= 1.0 + pppp::utils::pow(50.0 / estimated_unstable_rate.value(), 2) *
                                            pppp::utils::pow(attributes.star_rating, 2.8) / 600.0;

                if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_HD) &&
                    !is_convert) {
                    accuracy_value *= 1.075;
                }

                // Applies a bonus to maps with more total difficulty, calculating this with a map's total
                // hits and consistency factor.
                accuracy_value *= 1.0 + 0.3 * total_difficult_hits / (total_difficult_hits + 4000.0);

                // Applies a bonus to maps with more total memory required with HDFL.
                double memory_length_bonus = std::min(1.15, pppp::utils::pow(total_hits() / 1500.0, 0.3));

                if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_FL) &&
                    pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_HD) &&
                    !is_convert) {
                    accuracy_value *= std::max(1.0, 1.05 * memory_length_bonus);
                }

                return accuracy_value;
            }
        };
    } // namespace

    Status calculate_performance(TaikoPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                 const TaikoDifficultyAttributes& attributes,
                                 const pppp::beatmaps::Beatmap& beatmap) {
        out = TaikoPerformanceAttributes();

        TaikoPerformanceCalculator c(score, attributes);

        c.count_great = score.statistics[pppp::common::HIT_RESULT_GREAT];
        c.count_ok = score.statistics[pppp::common::HIT_RESULT_OK];
        c.count_meh = score.statistics[pppp::common::HIT_RESULT_MEH];
        c.count_miss = std::max(0, score.statistics[pppp::common::HIT_RESULT_MISS]);

        c.clock_rate = pppp::mods::mod_calculate_rate(score.mods.data(), score.mods.size());

        ModdedDifficulty difficulty = modded_difficulty(beatmap, score.mods.data(), score.mods.size());

        TaikoHitWindows hit_windows;
        hit_windows.set_difficulty(difficulty.overall_difficulty);
        c.great_hit_window = hit_windows.great / c.clock_rate;

        if (c.count_great != 0 && c.great_hit_window > 0) {
            c.estimated_unstable_rate =
                c.compute_deviation_upper_bound(c.count_great / static_cast<double>(c.total_hits())) * 10.0;
        }

        // Total difficult hits measures the total difficulty of a map based on its consistency factor.
        c.total_difficult_hits = c.total_hits() * attributes.consistency_factor;

        // Converts and the classic mod are detected and omitted from mod-specific bonuses due to the scope
        // of current difficulty calculation.
        bool is_convert = beatmap.mode != 1;
        bool is_classic = pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_CL);

        double difficulty_value = c.compute_difficulty_value(is_convert, is_classic) * 1.08;
        double accuracy_value = c.compute_accuracy_value(is_convert) * 1.1;

        out.difficulty = difficulty_value;
        out.accuracy = accuracy_value;
        out.total = difficulty_value + accuracy_value;
        out.estimated_unstable_rate = c.estimated_unstable_rate;

        return StatusCode::OK;
    }
}}} // namespace pppp::taiko::difficulty
