// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/speed/rhythm_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <climits>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace speed {
    namespace {
        /// An island is a group of consecutive objects with the same delta time.
        struct Island {
            /// Delta time of every object in this island
            int delta;

            /// How long the island is
            int delta_count;

            /// How many times island already occured
            int occurrences;

            Island(int d)
                : delta(std::max(d, static_cast<int>(preprocessing::MIN_DELTA_TIME))),
                  delta_count(1),
                  occurrences(1) {}

            void add_delta(int d) {
                if (delta == INT_MAX) {
                    delta = std::max(d, static_cast<int>(preprocessing::MIN_DELTA_TIME));
                }
                delta_count++;
            }

            bool is_similar_polarity(const Island& other, double epsilon) const {
                // single delta islands shouldn't be compared
                if (delta_count <= 1 || other.delta_count <= 1) {
                    return false;
                }
                return std::fabs(static_cast<double>(delta) - other.delta) < epsilon &&
                       delta_count % 2 == other.delta_count % 2;
            }

            bool almost_equals(const Island& other, double epsilon) const {
                return std::fabs(static_cast<double>(delta) - other.delta) < epsilon &&
                       delta_count == other.delta_count;
            }
        };

        double get_effective_difficulty(double delta_difference_ratio) {
            const double rhythm_ratio_difficulty_multiplier = 26.0;

            // Take only the fractional part of the value since we're only interested in punishing
            // multiples
            double delta_difference_fraction = delta_difference_ratio - std::floor(delta_difference_ratio);

            return 1.0 + rhythm_ratio_difficulty_multiplier *
                             std::min(0.5, utils::smoothstep_bell_curve(delta_difference_fraction, 0.5, 0.5));
        }

    } // namespace

    double RhythmEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current) {
        if (current.base_is_spinner) {
            return 0.0;
        }

        const int history_time_max = 5 * 1000; // 5 seconds
        const int history_objects_max = 32;
        const double rhythm_overall_multiplier = 0.95;

        double rhythm_complexity_sum = 0.0;
        double delta_difference_epsilon = current.hit_window_great * 0.3;

        Island island(INT_MAX);
        Island previous_island(INT_MAX);
        std::vector<Island> islands;

        double start_difficulty =
            0.0; // store the difficulty of the current start of an island to buff for tighter rhythms

        bool first_delta_switch = false;

        int historical_note_count = std::min(current.index, history_objects_max);

        int rhythm_start = 0;
        while (rhythm_start < historical_note_count - 2 &&
               current.start_time - static_cast<const preprocessing::OsuDifficultyHitObject*>(
                                        current.previous(rhythm_start))
                                        ->start_time <
                   history_time_max) {
            rhythm_start++;
        }

        const preprocessing::OsuDifficultyHitObject* prev_obj =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(rhythm_start));
        const preprocessing::OsuDifficultyHitObject* prev_prev_obj =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(rhythm_start + 1));

        // we go from the furthest object back to the current one
        for (int i = rhythm_start; i > 0; i--) {
            const preprocessing::OsuDifficultyHitObject* curr_obj =
                static_cast<const preprocessing::OsuDifficultyHitObject*>(current.previous(i - 1));

            if (curr_obj->base_is_spinner) {
                continue;
            }

            // scales note 0 to 1 from history to now
            double time_decay =
                (history_time_max - (current.start_time - curr_obj->start_time)) / history_time_max;
            double note_decay = static_cast<double>(historical_note_count - i) / historical_note_count;
            double curr_historical_decay = std::min(note_decay, time_decay);

            // Use custom cap value to ensure that at this point delta time is actually zero
            const double delta_min_value = 1e-7;
            double curr_delta = std::max(curr_obj->delta_time, delta_min_value);
            double prev_delta = std::max(prev_obj->delta_time, delta_min_value);

            double delta_difference = std::fabs(prev_delta - curr_delta);

            // Make sure to always have the current island initialised - if we don't do it here it will
            // only initialise on the next rhythm change
            if (island.delta == INT_MAX) {
                island = Island(static_cast<int>(curr_delta));
            }

            // calculate how much current delta difference deserves a rhythm bonus
            // this function is meant to reduce rhythm bonus for deltas that are multiples of each other
            // (i.e 100 and 200)
            double delta_difference_ratio =
                std::max(prev_delta, curr_delta) / std::min(prev_delta, curr_delta);

            // reduce ratio bonus if delta difference is too big
            double difference_multiplier = utils::math::clamp(2.0 - delta_difference_ratio / 8.0, 0.0, 1.0);

            double window_penalty = utils::math::clamp(
                (delta_difference - delta_difference_epsilon) / delta_difference_epsilon, 0.0, 1.0);

            double effective_difficulty =
                get_effective_difficulty(delta_difference_ratio) * window_penalty * difference_multiplier;

            // if previous object is a slider it might be easier to tap since you don't have to do a whole
            // tapping motion while a full deltatime might end up some weird ratio the "unpress->tap"
            // motion might be simple for example a slider-circle-circle pattern should be evaluated as a
            // regular triple and not as a single->double
            if (prev_obj->base_is_slider) {
                double slider_lazy_end_delta = curr_obj->minimum_jump_time;
                double slider_lazy_delta_difference_ratio =
                    std::max(slider_lazy_end_delta, curr_delta) / std::min(slider_lazy_end_delta, curr_delta);

                double slider_real_end_delta = curr_obj->last_object_end_delta_time;
                double slider_real_delta_difference_ratio =
                    std::max(slider_real_end_delta, curr_delta) / std::min(slider_real_end_delta, curr_delta);

                double slider_effective_difficulty =
                    std::min(get_effective_difficulty(slider_lazy_delta_difference_ratio),
                             get_effective_difficulty(slider_real_delta_difference_ratio));
                effective_difficulty = std::min(slider_effective_difficulty, effective_difficulty);
            }

            if (delta_difference < delta_difference_epsilon) {
                // island is still progressing
                island.add_delta(static_cast<int>(curr_delta));
            }

            if (first_delta_switch) {
                if (delta_difference > delta_difference_epsilon) {
                    // bpm change is into slider, this is easy acc window
                    if (curr_obj->base_is_slider) {
                        effective_difficulty *= 0.5;
                    }

                    // repeated island polarity (2 -> 4, 3 -> 5)
                    if (island.is_similar_polarity(previous_island, delta_difference_epsilon)) {
                        effective_difficulty *= 0.5;
                    }

                    // previous increase happened a note ago, 1/1->1/2-1/4, dont want to buff this.
                    if (std::max(prev_prev_obj->delta_time, delta_min_value) >
                            prev_delta + delta_difference_epsilon &&
                        prev_delta > curr_delta + delta_difference_epsilon) {
                        effective_difficulty *= 0.125;
                    }

                    // repeated island size (ex: triplet -> triplet)
                    // TODO: remove this nerf since its staying here only for balancing purposes because
                    // of the flawed ratio calculation
                    if (previous_island.delta_count == island.delta_count) {
                        effective_difficulty *= 0.5;
                    }

                    bool is_speeding_up = prev_delta > curr_delta + delta_difference_epsilon;

                    if (is_speeding_up) {
                        effective_difficulty *= 0.65;
                    }

                    bool found = false;

                    for (size_t j = 0; j < islands.size(); j++) {
                        if (islands[j].almost_equals(island, delta_difference_epsilon)) {
                            // only increase island occurrences if they're going one after another
                            if (previous_island.almost_equals(island, delta_difference_epsilon)) {
                                islands[j].occurrences++;
                            }

                            // repeated island (ex: triplet -> triplet)
                            double power =
                                utils::logistic(static_cast<double>(island.delta), 58.33, 0.24, 2.75);
                            effective_difficulty *= std::min(3.0 / islands[j].occurrences,
                                                             utils::pow(1.0 / islands[j].occurrences, power));

                            found = true;
                            break;
                        }
                    }

                    if (!found && island.delta_count > 0) {
                        islands.push_back(island);
                    }

                    // scale down the difficulty if the object is double-tappable
                    effective_difficulty *=
                        1.0 - calculate_double_tap_feasibility(*prev_obj, *curr_obj) * 0.75;

                    if (island.delta_count > 1) {
                        rhythm_complexity_sum +=
                            std::sqrt(effective_difficulty * start_difficulty) * curr_historical_decay;
                    } else {
                        // constant difficulty for single-note islands
                        rhythm_complexity_sum += 0.7 * curr_historical_decay;
                    }

                    start_difficulty = effective_difficulty;

                    // we're slowing down, stop counting
                    if (prev_delta + delta_difference_epsilon < curr_delta) {
                        first_delta_switch = false; // if we're speeding up, this stays true and we keep
                                                    // counting island size.
                    }

                    previous_island = island;
                    island = Island(static_cast<int>(curr_delta));
                }
            } else if (prev_delta > curr_delta + delta_difference_epsilon) { // we're speeding up
                // Begin counting island until we change speed again.
                first_delta_switch = true;

                // bpm change is into slider, this is easy acc window
                if (curr_obj->base_is_slider) {
                    effective_difficulty *= 0.6;
                }

                // bpm change was from a slider, this is easier typically than circle -> circle
                // unintentional side effect is that bursts with kicksliders at the ends might have lower
                // difficulty than bursts without sliders
                if (prev_obj->base_is_slider) {
                    effective_difficulty *= 0.6;
                }

                start_difficulty = effective_difficulty;

                island = Island(static_cast<int>(curr_delta));
            }

            prev_prev_obj = prev_obj;
            prev_obj = curr_obj;
        }

        // If the current island is long we don't want the sum to have as big of an effect
        rhythm_complexity_sum *= utils::reverse_lerp(static_cast<double>(island.delta_count), 22, 3);

        return std::sqrt(4.0 + rhythm_complexity_sum * rhythm_overall_multiplier) /
               2.0; // produces multiplier that can be applied to strain. range [1, infinity) (not really
                    // though)
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::speed
