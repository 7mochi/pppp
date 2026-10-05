// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/osu_performance_calculator.h"
#include "pppp/common/score_info.h"
#include "pppp/config.h" // IWYU pragma: export
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/base_performance.h"
#include "pppp/osu/difficulty/osu_legacy_score_miss_calculator.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/osu/osu_hit_windows.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/difficulty_range.h"
#include "pppp/utils/math/csharp.h"
#include "pppp/utils/precision.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty {
    namespace {
        struct OsuPerformanceCalculator {
            const pppp::common::ScoreInfo& score;
            const OsuDifficultyAttributes& attributes;

            bool using_classic_slider_accuracy;
            bool using_score_v2;
            double accuracy;
            int score_max_combo;
            int count_great, count_ok, count_meh, count_miss;

            /// Missed slider ticks that includes missed reverse arrows. Will only be correct on non-classic
            /// scores
            int count_slider_tick_miss;

            /// Amount of missed slider tails that don't break combo. Will only be correct on non-classic
            /// scores
            int count_slider_ends_dropped;

            /// Estimated total amount of combo breaks
            double effective_miss_count;

            double clock_rate;
            double great_hit_window, ok_hit_window, meh_hit_window;
            double overall_difficulty, approach_rate, drain_rate;
            nonstd::optional<double> speed_deviation;
            double aim_estimated_slider_breaks, speed_estimated_slider_breaks;

            OsuPerformanceCalculator(const pppp::common::ScoreInfo& s, const OsuDifficultyAttributes& a)
                : score(s),
                  attributes(a),
                  using_classic_slider_accuracy(false),
                  using_score_v2(false),
                  accuracy(0.0),
                  score_max_combo(0),
                  count_great(0),
                  count_ok(0),
                  count_meh(0),
                  count_miss(0),
                  count_slider_tick_miss(0),
                  count_slider_ends_dropped(0),
                  effective_miss_count(0.0),
                  clock_rate(0.0),
                  great_hit_window(0.0),
                  ok_hit_window(0.0),
                  meh_hit_window(0.0),
                  overall_difficulty(0.0),
                  approach_rate(0.0),
                  drain_rate(0.0),
                  speed_deviation(),
                  aim_estimated_slider_breaks(0.0),
                  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
                  speed_estimated_slider_breaks(0.0) {}

            bool has_mod(pppp::mods::ModId id) const {
                return pppp::mods::mod_has(score.mods.data(), score.mods.size(), id) != 0;
            }
            int total_hits() const { return count_great + count_ok + count_meh + count_miss; }
            int total_successful_hits() const { return count_great + count_ok + count_meh; }
            int total_imperfect_hits() const { return count_ok + count_meh + count_miss; }

            // Miss penalty assumes that a player will miss on the hardest parts of a map,
            // so we use the amount of relatively difficult sections to adjust miss penalty
            // to make it more punishing on maps with lower amount of hard sections.
            double calculate_miss_penalty(double miss_count, double difficult_strain_count) const {
                return 0.93 / (miss_count / (4 * std::log(std::max(1.0, difficult_strain_count))) + 1);
            }

            double get_combo_scaling_factor() const {
                return attributes.max_combo <= 0
                           ? 1.0
                           : std::min(utils::pow(static_cast<double>(score_max_combo), 0.8) /
                                          utils::pow(static_cast<double>(attributes.max_combo), 0.8),
                                      1.0);
            }

            double calculate_combo_based_estimated_miss_count() const {
                if (attributes.slider_count <= 0) {
                    return count_miss;
                }
                double miss_count = count_miss;
                if (using_classic_slider_accuracy) {
                    // If sliders in the map are hard - it's likely for player to drop sliderends
                    // If map has easy sliders - it's more likely for player to sliderbreak
                    double likely_missed_sliderend_portion =
                        0.04 + 0.06 * utils::pow(std::min(attributes.aim_top_weighted_slider_factor, 1.0), 2);

                    // Consider that full combo is maximum combo minus dropped slider tails since they don't
                    // contribute to combo but also don't break it In classic scores we can't know the amount
                    // of dropped sliders so we estimate it
                    double full_combo_threshold =
                        attributes.max_combo -
                        std::min(4 + likely_missed_sliderend_portion * attributes.slider_count,
                                 static_cast<double>(attributes.slider_count));
                    if (score_max_combo < full_combo_threshold) {
                        miss_count =
                            full_combo_threshold / std::max(1.0, static_cast<double>(score_max_combo));
                    }

                    // In classic scores there can't be more misses than a sum of all non-perfect judgements
                    miss_count = std::min(miss_count, static_cast<double>(total_imperfect_hits()));

                    // Every slider has *at least* 2 combo attributed in classic mechanics.
                    // If they broke on a slider with a tick, then this still works since they would have lost
                    // at least 2 combo (the tick and the end) Using this as a max means a score that loses 1
                    // combo on a map can't possibly have been a slider break. It must have been a slider end.
                    int max_possible_slider_breaks =
                        std::min(attributes.slider_count, (attributes.max_combo - score_max_combo) / 2);

                    double slider_breaks = miss_count - count_miss;

                    if (slider_breaks > max_possible_slider_breaks) {
                        miss_count = count_miss + max_possible_slider_breaks;
                    }
                } else {
                    double full_combo_threshold = attributes.max_combo - count_slider_ends_dropped;

                    if (score_max_combo < full_combo_threshold) {
                        miss_count =
                            full_combo_threshold / std::max(1.0, static_cast<double>(score_max_combo));
                    }

                    // Combine regular misses with tick misses since tick misses break combo as well
                    miss_count =
                        std::min(miss_count, static_cast<double>(count_slider_tick_miss + count_miss));
                }
                return miss_count;
            }

            double calculate_estimated_slider_breaks(double top_weighted_slider_factor) const {
                int non_miss_mistakes = count_ok + count_meh;
                if (!using_classic_slider_accuracy || non_miss_mistakes == 0) {
                    return 0;
                }
                double missed_combo_percent =
                    1.0 - static_cast<double>(score_max_combo) / attributes.max_combo;
                double estimated = std::min(static_cast<double>(non_miss_mistakes),
                                            effective_miss_count * top_weighted_slider_factor);

                // Scores with more Oks and Mehs are more likely to have slider breaks.
                // We add an arbitrary value to both sides of the division to make it more stable on extreme
                // ends.
                double non_miss_mistake_adjustment =
                    (non_miss_mistakes - estimated + 4.5) / (non_miss_mistakes + 4);

                // There is a low probability of extra slider breaks on effective miss counts close to 1, as
                // score based calculations are good at indicating if only a single break occurred.
                estimated *= utils::smoothstep(effective_miss_count, 1, 2);
                return estimated * non_miss_mistake_adjustment *
                       utils::logistic(missed_combo_percent, 0.33, 15);
            }

            /// Estimates the player's tap deviation based on the OD, given number of greats, oks,
            /// mehs and misses, assuming the player's mean hit error is 0. The estimation is
            /// consistent in that two SS scores on the same map with the same settings will always
            /// return the same deviation. Misses are ignored because they are usually due to
            /// misaiming. Greats and oks are assumed to follow a normal distribution, whereas mehs
            /// are assumed to follow a uniform distribution.
            bool calculate_deviation(double relevant_great, double relevant_ok, double relevant_meh,
                                     nonstd::optional<double>& out) const {
                if (relevant_great + relevant_ok + relevant_meh <= 0) {
                    return false;
                }
                // The sample proportion of successful hits.
                double n = std::max(1.0, relevant_great + relevant_ok);
                double p = relevant_great / n;

                // 99% critical value for the normal distribution (one-tailed).
                const double z = 2.32634787404;

                // We can be 99% confident that the population proportion is at least this value.
                double p_lower_bound =
                    std::min(p, (n * p + z * z / 2) / (n + z * z) -
                                    z / (n + z * z) * std::sqrt(n * p * (1 - p) + z * z / 4));

                double dev;

                // Tested max precision for the deviation calculation.
                if (p_lower_bound > 0.01) {
                    // Compute deviation assuming greats and oks are normally distributed.
                    dev = great_hit_window / (utils::SQRT2 * utils::erf_inv(p_lower_bound));

                    // Subtract the deviation provided by tails that land outside the ok hit window
                    // from the deviation computed above. This is equivalent to calculating the
                    // deviation of a normal distribution truncated at +-the ok hit window.
                    double ok_hit_window_tail_amount =
                        std::sqrt(2 / utils::PI_D) * ok_hit_window *
                        std::exp(-0.5 * utils::pow(ok_hit_window / dev, 2)) /
                        (dev * utils::erf(ok_hit_window / (utils::SQRT2 * dev)));
                    dev *= std::sqrt(1 - ok_hit_window_tail_amount);
                } else {
                    // A tested limit value for the case of a score only containing oks.
                    dev = ok_hit_window / std::sqrt(3.0);
                }

                // Compute and add the variance for mehs, assuming that they are uniformly distributed.
                double meh_variance = (meh_hit_window * meh_hit_window + ok_hit_window * meh_hit_window +
                                       ok_hit_window * ok_hit_window) /
                                      3;
                dev = std::sqrt(
                    ((relevant_great + relevant_ok) * utils::pow(dev, 2) + relevant_meh * meh_variance) /
                    (relevant_great + relevant_ok + relevant_meh));
                out = dev;
                return true;
            }

            /// Estimates player's deviation on speed notes using calculate_deviation(), assuming
            /// worst-case. Treats all speed notes as hit circles.
            bool calculate_speed_deviation(nonstd::optional<double>& out) const {
                if (total_successful_hits() == 0) {
                    return false;
                }

                // Calculate accuracy assuming the worst case scenario
                double speed_note_count = attributes.speed_note_count;
                speed_note_count += (total_hits() - attributes.speed_note_count) * 0.1;

                // Assume worst case: all mistakes were on speed notes
                double relevant_miss = std::min(static_cast<double>(count_miss), speed_note_count);
                double relevant_meh =
                    std::min(static_cast<double>(count_meh), speed_note_count - relevant_miss);
                double relevant_ok =
                    std::min(static_cast<double>(count_ok), speed_note_count - relevant_miss - relevant_meh);
                double relevant_great =
                    std::max(0.0, speed_note_count - relevant_miss - relevant_meh - relevant_ok);
                return calculate_deviation(relevant_great, relevant_ok, relevant_meh, out);
            }

            // Calculates multiplier for speed to account for improper tapping based on the deviation and
            // speed difficulty
            // https://www.desmos.com/calculator/dmogdhzofn
            double calculate_speed_high_deviation_nerf() const {
                if (!speed_deviation.has_value()) {
                    return 0;
                }
                double speed_value = difficulty::difficulty_to_performance(attributes.speed_difficulty);

                // Decides a point where the PP value achieved compared to the speed deviation is assumed to
                // be tapped improperly. Any PP above this point is considered "excess" speed difficulty.
                // This is used to cause PP above the cutoff to scale logarithmically towards the original
                // speed value thus nerfing the value.
                double excess_speed_difficulty_cutoff =
                    100 + 220 * utils::pow(22 / speed_deviation.value(), 6.5);
                if (speed_value <= excess_speed_difficulty_cutoff) {
                    return 1.0;
                }

                const double scale = 50;
                double adjusted_speed_value =
                    scale * (std::log((speed_value - excess_speed_difficulty_cutoff) / scale + 1) +
                             excess_speed_difficulty_cutoff / scale);

                // 220 UR and less are considered tapped correctly to ensure that normal scores will be
                // punished as little as possible
                double t = 1 - utils::reverse_lerp(speed_deviation.value(), 22.0, 27.0);
                adjusted_speed_value = utils::math::lerp_dotnet(adjusted_speed_value, speed_value, t);
                return adjusted_speed_value / speed_value;
            }

            /// Calculates a visibility bonus that is applicable to Traceable.
            double calculate_traceable_bonus(double slider_factor) const {
                // We want to reward slider aim less, more so at lower AR
                double high_ar_factor = 0.5 + (utils::pow(slider_factor, 6) / 2);
                double low_ar_factor = utils::pow(slider_factor, 6);

                // Start from normal curve, rewarding lower AR up to AR7
                double bonus = 0.0275;
                bonus += 0.025 * (12.0 - std::max(approach_rate, 7.0)) * high_ar_factor;
                if (approach_rate < 7) {
                    // For AR up to 0 - reduce reward for very low ARs when object is visible
                    bonus += 0.025 * (7.0 - std::max(approach_rate, 0.0)) * low_ar_factor;
                }
                if (approach_rate < 0) {
                    // Starting from AR0 - cap values so they won't grow to infinity
                    bonus += 0.025 * (1 - utils::pow(1.5, approach_rate)) * low_ar_factor;
                }
                return bonus;
            }

            double compute_aim_value() const {
                if (has_mod(pppp::mods::MOD_AP)) {
                    return 0.0;
                }
                double aim_difficulty = attributes.aim_difficulty;
                if (attributes.slider_count > 0 && attributes.aim_difficult_slider_count > 0) {
                    double estimate_improperly_followed_difficult_sliders;
                    if (using_classic_slider_accuracy) {
                        // When the score is considered classic (regardless if it was made on old client or
                        // not) we consider all missing combo to be dropped difficult sliders
                        int maximum_possible_dropped_sliders = total_imperfect_hits();
                        estimate_improperly_followed_difficult_sliders = utils::math::clamp(
                            static_cast<double>(std::min(maximum_possible_dropped_sliders,
                                                         attributes.max_combo - score_max_combo)),
                            0.0, attributes.aim_difficult_slider_count);
                    } else {
                        // We add tick misses here since they too mean that the player didn't follow the
                        // slider properly We however aren't adding misses here because missing slider heads
                        // has a harsh penalty by itself and doesn't mean that the rest of the slider wasn't
                        // followed properly
                        estimate_improperly_followed_difficult_sliders = utils::math::clamp(
                            static_cast<double>(count_slider_ends_dropped + count_slider_tick_miss), 0.0,
                            attributes.aim_difficult_slider_count);
                    }
                    double slider_nerf_factor =
                        (1 - attributes.slider_factor) *
                            utils::pow(1 - estimate_improperly_followed_difficult_sliders /
                                               attributes.aim_difficult_slider_count,
                                       3) +
                        attributes.slider_factor;
                    aim_difficulty *= slider_nerf_factor;
                }
                double value = difficulty::difficulty_to_performance(aim_difficulty);
                int th = total_hits();
                double length_bonus = 0.95 + 0.35 * std::min(1.0, th / 2000.0) +
                                      (th > 2000 ? std::log10(th / 2000.0) * 0.5 : 0.0);
                value *= length_bonus;
                if (effective_miss_count > 0) {
                    double relevant_miss_count =
                        std::min(effective_miss_count + aim_estimated_slider_breaks,
                                 static_cast<double>(total_imperfect_hits() + count_slider_tick_miss));
                    value *=
                        calculate_miss_penalty(relevant_miss_count, attributes.aim_difficult_strain_count);
                }
                // TC bonuses are excluded when blinds is present as the increased visual difficulty is
                // unimportant when notes cannot be seen.
                if (has_mod(pppp::mods::MOD_BL)) {
                    value *=
                        1.3 + (th * (0.0016 / (1 + 2 * effective_miss_count)) * utils::pow(accuracy, 16)) *
                                  (1 - 0.003 * drain_rate * drain_rate);
                } else if (has_mod(pppp::mods::MOD_TC)) {
                    value *= 1.0 + calculate_traceable_bonus(attributes.slider_factor);
                }
                value *= accuracy;
                return value;
            }

            double compute_speed_value() const {
                if (has_mod(pppp::mods::MOD_RX) || !speed_deviation.has_value()) {
                    return 0.0;
                }
                double value = difficulty::difficulty_to_performance(attributes.speed_difficulty);
                if (effective_miss_count > 0) {
                    double relevant_miss_count =
                        std::min(effective_miss_count + speed_estimated_slider_breaks,
                                 static_cast<double>(total_imperfect_hits() + count_slider_tick_miss));
                    value *=
                        calculate_miss_penalty(relevant_miss_count, attributes.speed_difficult_strain_count);
                }
                if (has_mod(pppp::mods::MOD_BL)) {
                    // Increasing the speed value by object count for Blinds isn't ideal, so the minimum buff
                    // is given.
                    value *= 1.12;
                }
                value *= calculate_speed_high_deviation_nerf();

                // An effective hit window is created based on the speed SR. The higher the speed difficulty,
                // the shorter the hit window. For example, a speed SR of 4.0 leads to an effective hit window
                // of 20ms, which is OD 10.
                double effective_hit_window = 20 * utils::pow(4 / attributes.speed_difficulty, 0.35);

                // Find the proportion of 300s on speed notes assuming the hit window was the effective hit
                // window.
                double effective_accuracy = utils::erf(effective_hit_window / speed_deviation.value());

                // Scale speed value by normalized accuracy.
                value *= utils::pow(effective_accuracy, 2);
                return value;
            }

            double compute_accuracy_value() const {
                if (has_mod(pppp::mods::MOD_RX)) {
                    return 0.0;
                }
                // This percentage only considers HitCircles of any value - in this part of the calculation we
                // focus on hitting the timing hit window.
                double better_accuracy_percentage;
                int amount_hit_objects_with_accuracy = attributes.hit_circle_count;
                if (!using_classic_slider_accuracy || using_score_v2) {
                    amount_hit_objects_with_accuracy += attributes.slider_count;
                }
                if (amount_hit_objects_with_accuracy > 0) {
                    better_accuracy_percentage =
                        ((count_great - std::max(total_hits() - amount_hit_objects_with_accuracy, 0)) * 6 +
                         count_ok * 2 + count_meh) /
                        static_cast<double>(amount_hit_objects_with_accuracy * 6);
                } else {
                    better_accuracy_percentage = 0;
                }

                // It is possible to reach a negative accuracy with this formula. Cap it at zero - zero
                // points.
                if (better_accuracy_percentage < 0) {
                    better_accuracy_percentage = 0;
                }

                // Lots of arbitrary values from testing.
                // Considering to use derivation from perfect accuracy in a probabilistic manner - assume
                // normal distribution.
                double value = utils::pow(1.52163, overall_difficulty) *
                               utils::pow(better_accuracy_percentage, 24) * 2.83;

                // Bonus for many hitcircles - it's harder to keep good accuracy up for longer.
                value *= amount_hit_objects_with_accuracy < 1000
                             ? utils::pow(amount_hit_objects_with_accuracy / 1000.0, 0.3)
                             : utils::pow(amount_hit_objects_with_accuracy / 1000.0, 0.1);
                if (has_mod(pppp::mods::MOD_BL)) {
                    // Increasing the accuracy value by object count for Blinds isn't ideal, so the minimum
                    // buff is given.
                    value *= 1.14;
                } else if (has_mod(pppp::mods::MOD_TC)) {
                    // Decrease bonus for AR > 10
                    value *= 1 + 0.08 * utils::reverse_lerp(approach_rate, 11.5, 10);
                }
                return value;
            }

            double compute_flashlight_value() const {
                if (!has_mod(pppp::mods::MOD_FL)) {
                    return 0.0;
                }
                double value =
                    difficulty::flashlight_difficulty_to_performance(attributes.flashlight_difficulty);

                // Penalize misses by assessing # of misses relative to the total # of objects. Default a 3%
                // reduction for any # of misses.
                if (effective_miss_count > 0) {
                    value *= 0.97 * utils::pow(1 - utils::pow(effective_miss_count / total_hits(), 0.775),
                                               utils::pow(effective_miss_count, .875));
                }
                value *= get_combo_scaling_factor();

                // Scale the flashlight value with accuracy _slightly_.
                value *= 0.5 + accuracy / 2.0;
                return value;
            }

            double compute_reading_value() const {
                double value = difficulty::difficulty_to_performance(attributes.reading_difficulty);
                if (effective_miss_count > 0) {
                    value *= calculate_miss_penalty(effective_miss_count + aim_estimated_slider_breaks,
                                                    attributes.reading_difficult_note_count);
                }
                // Scale the reading value with accuracy _harshly_.
                value *= utils::pow(accuracy, 3);
                return value;
            }
        };

    } // namespace

    Status calculate_performance(OsuPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                 const OsuDifficultyAttributes& attributes,
                                 const pppp::beatmaps::Beatmap& beatmap) {
        out = OsuPerformanceAttributes();
        OsuPerformanceCalculator c(score, attributes);

        c.using_classic_slider_accuracy = false;
        for (size_t i = 0; i < score.mods.size(); i++) {
            if (score.mods[i].id == pppp::mods::MOD_CL && score.mods[i].classic.no_slider_head_accuracy) {
                c.using_classic_slider_accuracy = true;
            }
        }
        c.using_score_v2 = c.has_mod(pppp::mods::MOD_SV2);

        c.accuracy = utils::math::clamp(score.accuracy, 0.0, 1.0);
        c.score_max_combo = std::min(std::max(score.max_combo, 0), attributes.max_combo);
        c.count_great = score.statistics[pppp::common::HIT_RESULT_GREAT];
        c.count_ok = score.statistics[pppp::common::HIT_RESULT_OK];
        c.count_meh = score.statistics[pppp::common::HIT_RESULT_MEH];
        c.count_miss = score.statistics[pppp::common::HIT_RESULT_MISS];
        c.count_slider_ends_dropped =
            attributes.slider_count - score.statistics[pppp::common::HIT_RESULT_SLIDER_TAIL_HIT];
        c.count_slider_tick_miss = score.statistics[pppp::common::HIT_RESULT_LARGE_TICK_MISS];
        c.effective_miss_count = c.count_miss;

        pppp::mods::DifficultySettings ds;
        ds.drain_rate = static_cast<float>(beatmap.difficulty.drain_rate);
        ds.circle_size = static_cast<float>(beatmap.difficulty.circle_size);
        ds.overall_difficulty = static_cast<float>(beatmap.difficulty.overall_difficulty);
        ds.approach_rate = static_cast<float>(beatmap.difficulty.approach_rate);
        ds.slider_multiplier = beatmap.difficulty.slider_multiplier;
        ds.slider_tick_rate = beatmap.difficulty.slider_tick_rate;
        pppp::mods::mod_apply_to_difficulty(score.mods.data(), score.mods.size(), &ds);
        c.clock_rate = pppp::mods::mod_calculate_rate(score.mods.data(), score.mods.size());

        pppp::osu::OsuHitWindows hit_windows;
        hit_windows.set_difficulty(static_cast<float>(ds.overall_difficulty));
        c.great_hit_window = hit_windows.window_for(pppp::common::HIT_RESULT_GREAT) / c.clock_rate;
        c.ok_hit_window = hit_windows.window_for(pppp::common::HIT_RESULT_OK) / c.clock_rate;
        c.meh_hit_window = hit_windows.window_for(pppp::common::HIT_RESULT_MEH) / c.clock_rate;

        {
            double preempt =
                utils::difficulty_range(static_cast<float>(ds.approach_rate), object::PREEMPT_MAX,
                                        object::PREEMPT_MID, object::PREEMPT_MIN) /
                c.clock_rate;
            c.approach_rate = utils::inverse_difficulty_range(preempt, object::PREEMPT_MAX,
                                                              object::PREEMPT_MID, object::PREEMPT_MIN);
        }
        c.overall_difficulty = (79.5 - c.great_hit_window) / 6;
        c.drain_rate = static_cast<float>(ds.drain_rate);

        double combo_based = c.calculate_combo_based_estimated_miss_count();
        out.combo_based_estimated_miss_count = combo_based;
        if (c.using_classic_slider_accuracy && !c.using_score_v2 && score.legacy_total_score.has_value()) {
            double score_based = OsuLegacyScoreMissCalculator(score, attributes).calculate();
            out.score_based_estimated_miss_count = score_based;
            c.effective_miss_count = score_based;
        } else {
            c.effective_miss_count = combo_based;
        }
        c.effective_miss_count = std::max(static_cast<double>(c.count_miss), c.effective_miss_count);
        c.effective_miss_count = std::min(static_cast<double>(c.total_hits()), c.effective_miss_count);
        c.effective_miss_count = std::max(0.0, c.effective_miss_count);

        c.aim_estimated_slider_breaks = 0;
        c.speed_estimated_slider_breaks = 0;
        if (c.effective_miss_count > 0) {
            c.aim_estimated_slider_breaks =
                c.calculate_estimated_slider_breaks(attributes.aim_top_weighted_slider_factor);
            c.speed_estimated_slider_breaks =
                c.calculate_estimated_slider_breaks(attributes.speed_top_weighted_slider_factor);
        }

        double multiplier = difficulty::PERFORMANCE_BASE_MULTIPLIER;
        if (c.has_mod(pppp::mods::MOD_NF)) {
            multiplier *= std::max(0.90, 1.0 - 0.02 * c.effective_miss_count);
        }
        if (c.has_mod(pppp::mods::MOD_SO) && c.total_hits() > 0) {
            multiplier *=
                1.0 - utils::pow(static_cast<double>(attributes.spinner_count) / c.total_hits(), 0.85);
        }
        if (c.has_mod(pppp::mods::MOD_RX)) {
            // https://www.desmos.com/calculator/vspzsop6td
            // we use OD13.3 as maximum since it's the value at which great hitwidow becomes 0
            // this is well beyond currently maximum achievable OD which is 12.17 (DTx2 + DA with OD11)
            double ok_multiplier =
                0.75 * std::max(0.0, c.overall_difficulty > 0.0 ? 1 - c.overall_difficulty / 13.33 : 1.0);
            double meh_multiplier = std::max(
                0.0, c.overall_difficulty > 0.0 ? 1 - utils::pow(c.overall_difficulty / 13.33, 5) : 1.0);

            // As we're adding Oks and Mehs to an approximated number of combo breaks the result can be higher
            // than total hits in specific scenarios (which breaks some calculations) so we need to clamp it.
            c.effective_miss_count =
                std::min(c.effective_miss_count + c.count_ok * ok_multiplier + c.count_meh * meh_multiplier,
                         static_cast<double>(c.total_hits()));
        }

        c.calculate_speed_deviation(c.speed_deviation);

        double aim = c.compute_aim_value();
        double speed = c.compute_speed_value();
        double acc = c.compute_accuracy_value();
        double reading = c.compute_reading_value();
        double flashlight = c.compute_flashlight_value();
        double cognition = difficulty::sum_cognition_difficulty(reading, flashlight);
        double total =
            utils::norm(difficulty::PERFORMANCE_NORM_EXPONENT, aim, speed, acc, cognition) * multiplier;

        out.aim = aim;
        out.speed = speed;
        out.accuracy = acc;
        out.flashlight = flashlight;
        out.reading = reading;
        out.effective_miss_count = c.effective_miss_count;
        out.aim_estimated_slider_breaks = c.aim_estimated_slider_breaks;
        out.speed_estimated_slider_breaks = c.speed_estimated_slider_breaks;
        out.speed_deviation = c.speed_deviation;
        out.total = total;
        return StatusCode::OK;
    }
}}} // namespace pppp::osu::difficulty
