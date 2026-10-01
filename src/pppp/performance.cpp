#include "pppp/performance.h"
#include "pppp/fruits/difficulty/catch_performance_calculator.h"
#include "pppp/mania/difficulty/mania_performance_calculator.h"
#include "pppp/osu/difficulty/osu_performance_calculator.h"
#include "pppp/taiko/difficulty/taiko_performance_calculator.h"

namespace pppp {
    PerformanceAttributes Performance::calculate() const {
        DifficultyAttributes attributes = difficulty_attributes;
        if (!has_attributes) {
            Difficulty difficulty;
            if (has_mods) {
                difficulty.mods(mod_list.empty() ? 0 : &mod_list[0], mod_list.size());
            }
            attributes = difficulty.calculate(*beatmap);
        }

        PerformanceAttributes out;
        out.ruleset = attributes.ruleset;

        pppp::common::ScoreInfo score = score_state;
        if (has_mods) {
            score.mods = mod_list.empty() ? 0 : &mod_list[0];
            score.mod_count = mod_list.size();
        }
        if (has_combo) {
            score.max_combo = combo_value;
        }
        if (has_accuracy) {
            score.accuracy = accuracy_value;
        }
        if (has_misses) {
            score.statistics[pppp::common::HIT_RESULT_MISS] = misses_value;
        }

        switch (attributes.ruleset) {
        case Ruleset::RULESET_OSU:
            pppp::osu::difficulty::calculate_performance(out.osu, score, attributes.osu, *beatmap);
            break;
        case Ruleset::RULESET_TAIKO:
            pppp::taiko::difficulty::calculate_performance(out.taiko, score, attributes.taiko, *beatmap);
            break;
        case Ruleset::RULESET_CATCH:
            pppp::fruits::difficulty::calculate_performance(out.fruits, score, attributes.fruits, *beatmap);
            break;
        case Ruleset::RULESET_MANIA:
            pppp::mania::difficulty::calculate_performance(out.mania, score, attributes.mania, *beatmap);
            break;
        default: break;
        }
        return out;
    }
} // namespace pppp
