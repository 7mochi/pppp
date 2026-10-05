// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/difficulty/mania_performance_calculator.h"
#include "pppp/mods/mod.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>

namespace pppp { namespace mania { namespace difficulty {
    namespace {
        struct ManiaPerformanceCalculator {
            const pppp::common::ScoreInfo& score;
            const ManiaDifficultyAttributes& attributes;

            int count_perfect;
            int count_great;
            int count_good;
            int count_ok;
            int count_meh;
            int count_miss;
            double score_accuracy;

            ManiaPerformanceCalculator(const pppp::common::ScoreInfo& s, const ManiaDifficultyAttributes& a)
                : score(s),
                  attributes(a),
                  count_perfect(0),
                  count_great(0),
                  count_good(0),
                  count_ok(0),
                  count_meh(0),
                  count_miss(0),
                  score_accuracy(0.0) {}

            double total_hits() const {
                return count_perfect + count_ok + count_great + count_good + count_meh + count_miss;
            }

            /// Accuracy used to weight judgements independently from the score's actual accuracy.
            double calculate_custom_accuracy() const {
                if (total_hits() == 0) {
                    return 0;
                }
                return (count_perfect * 320 + count_great * 300 + count_good * 200 + count_ok * 100 +
                        count_meh * 50) /
                       (total_hits() * 320);
            }

            double compute_difficulty_value() const {
                return 8.0 *
                       utils::pow(std::max(attributes.star_rating - 0.15, 0.05),
                                  2.2) // Star rating to pp curve
                       * std::max(0.0,
                                  5 * score_accuracy - 4) // From 80% accuracy, 1/20th of total pp is
                                                          // awarded per additional 1% accuracy
                       * (1 + 0.1 * std::min(1.0, total_hits() / 1500)); // Length bonus, capped at 1500 notes
            }
        };
    } // namespace

    Status calculate_performance(ManiaPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                 const ManiaDifficultyAttributes& attributes,
                                 const pppp::beatmaps::Beatmap& beatmap) {
        (void)beatmap;

        out = ManiaPerformanceAttributes();

        ManiaPerformanceCalculator c(score, attributes);

        c.count_perfect = score.statistics[pppp::common::HIT_RESULT_PERFECT];
        c.count_great = score.statistics[pppp::common::HIT_RESULT_GREAT];
        c.count_good = score.statistics[pppp::common::HIT_RESULT_GOOD];
        c.count_ok = score.statistics[pppp::common::HIT_RESULT_OK];
        c.count_meh = score.statistics[pppp::common::HIT_RESULT_MEH];
        c.count_miss = std::max(0, score.statistics[pppp::common::HIT_RESULT_MISS]);
        c.score_accuracy = utils::math::clamp(c.calculate_custom_accuracy(), 0.0, 1.0);

        double multiplier = 1.0;

        for (size_t i = 0; i < score.mods.size(); i++) {
            if (score.mods[i].id == pppp::mods::MOD_NF) {
                multiplier *= 0.75;
            }
            if (score.mods[i].id == pppp::mods::MOD_EZ) {
                multiplier *= 0.5;
            }
        }

        double difficulty_value = c.compute_difficulty_value();
        double total_value = difficulty_value * multiplier;

        out.difficulty = difficulty_value;
        out.total = total_value;

        return StatusCode::OK;
    }
}}} // namespace pppp::mania::difficulty
