// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/difficulty/catch_performance_calculator.h"
#include "pppp/fruits/catch_beatmap.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/difficulty_range.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace fruits { namespace difficulty {
    namespace {
        struct CatchPerformanceCalculator {
            const pppp::common::ScoreInfo& score;
            const CatchDifficultyAttributes& attributes;

            int num300;
            int num100;
            int num50;
            int num_katu;
            int num_miss;

            CatchPerformanceCalculator(const pppp::common::ScoreInfo& s, const CatchDifficultyAttributes& a)
                : score(s),
                  attributes(a),
                  num300(0),
                  num100(0),
                  num50(0),
                  num_katu(0),
                  num_miss(0) {}

            double accuracy() const {
                return total_hits() == 0
                           ? 0.0
                           : utils::math::clamp(static_cast<double>(total_successful_hits()) / total_hits(),
                                                0.0, 1.0);
            }

            int total_hits() const { return num50 + num100 + num300 + num_miss + num_katu; }
            int total_successful_hits() const { return num50 + num100 + num300; }
            int total_combo_hits() const { return num_miss + num100 + num300; }
        };
    } // namespace

    Status calculate_performance(CatchPerformanceAttributes& out, const pppp::common::ScoreInfo& score,
                                 const CatchDifficultyAttributes& attributes,
                                 const pppp::beatmaps::Beatmap& beatmap) {
        out = CatchPerformanceAttributes();

        CatchPerformanceCalculator c(score, attributes);

        c.num300 = score.statistics[pppp::common::HIT_RESULT_GREAT]; // HitResult.Great
        c.num100 = score.statistics[pppp::common::HIT_RESULT_LARGE_TICK_HIT]; // HitResult.LargeTickHit
        c.num50 = score.statistics[pppp::common::HIT_RESULT_SMALL_TICK_HIT]; // HitResult.SmallTickHit
        c.num_katu = score.statistics[pppp::common::HIT_RESULT_SMALL_TICK_MISS]; // HitResult.SmallTickMiss
        c.num_miss = std::max(
            0, score.statistics[pppp::common::HIT_RESULT_MISS] +
                   score.statistics[pppp::common::HIT_RESULT_LARGE_TICK_MISS]); // HitResult.Miss PLUS
                                                                                // HitResult.LargeTickMiss

        double score_max_combo = utils::math::clamp(score.max_combo, 0, attributes.max_combo);

        // We are heavily relying on aim in catch the beat
        double value = utils::pow(5.0 * std::max(1.0, attributes.star_rating / 0.0049) - 4.0, 2.0) / 100000.0;

        // Longer maps are worth more. "Longer" means how many hits there are which can contribute to
        // combo
        int num_total_hits = c.total_combo_hits();

        double length_bonus = 0.95 + 0.3 * std::min(1.0, num_total_hits / 2500.0) +
                              (num_total_hits > 2500 ? std::log10(num_total_hits / 2500.0) * 0.475 : 0.0);
        value *= length_bonus;

        value *= utils::pow(0.97, c.num_miss);

        // Combo scaling
        if (attributes.max_combo > 0) {
            value *=
                std::min(utils::pow(score_max_combo, 0.35) / utils::pow(attributes.max_combo, 0.35), 1.0);
        }

        double clock_rate = pppp::mods::mod_calculate_rate(score.mods.data(), score.mods.size());
        ModdedDifficulty difficulty = modded_difficulty(beatmap, score.mods.data(), score.mods.size());

        // this is the same as osu!, so there's potential to share the implementation... maybe
        double preempt =
            utils::difficulty_range(difficulty.approach_rate, 1800.0, 1200.0, 450.0) / clock_rate;
        double approach_rate =
            preempt > 1200.0 ? -(preempt - 1800.0) / 120.0 : -(preempt - 1200.0) / 150.0 + 5.0;

        double approach_rate_factor = 1.0;
        if (approach_rate > 9.0) {
            approach_rate_factor += 0.1 * (approach_rate - 9.0); // 10% for each AR above 9
        }
        if (approach_rate > 10.0) {
            approach_rate_factor += 0.1 * (approach_rate - 10.0); // Additional 10% at AR 11, 30% total
        } else if (approach_rate < 8.0) {
            approach_rate_factor += 0.025 * (8.0 - approach_rate); // 2.5% for each AR below 8
        }

        value *= approach_rate_factor;

        if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_HD)) {
            // Hiddens gives almost nothing on max approach rate, and more the lower it is
            if (approach_rate <= 10.0) {
                value *= 1.05 + 0.075 * (10.0 - approach_rate); // 7.5% for each AR below 10
            } else {
                value *= 1.01 + 0.04 * (11.0 - std::min(11.0, approach_rate)); // 5% at AR 10, 1% at AR 11
            }
        }

        if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_FL)) {
            value *= 1.35 * length_bonus;
        }

        value *= utils::pow(c.accuracy(), 5.5);

        if (pppp::mods::mod_has(score.mods.data(), score.mods.size(), pppp::mods::MOD_NF)) {
            value *= std::max(0.90, 1.0 - 0.02 * c.num_miss);
        }

        out.total = value;

        return StatusCode::OK;
    }
}}} // namespace pppp::fruits::difficulty
