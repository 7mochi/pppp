#include "pppp/performance.h"
#include "pppp/fruits/difficulty/catch_performance_calculator.h"
#include "pppp/mania/difficulty/mania_performance_calculator.h"
#include "pppp/osu/difficulty/osu_performance_calculator.h"
#include "pppp/taiko/difficulty/taiko_performance_calculator.h"

namespace pppp {
    Status Performance::calculate(PerformanceAttributes& out) const {
        out = PerformanceAttributes();

        ScoreInfo score = score_state;
        if (has_mods) {
            score.mods = mod_list;
        }
        if (has_combo) {
            score.max_combo = combo_value;
        }
        if (has_accuracy) {
            score.accuracy = accuracy_value;
        }
        if (has_misses) {
            score.statistics[HIT_RESULT_MISS] = misses_value;
        }

        DifficultyAttributes attributes = difficulty_attributes;
        if (!has_attributes) {
            const Status status = Difficulty().mods(score.mods).calculate(*beatmap, attributes);
            if (!status.ok()) {
                return status;
            }
        }

        switch (attributes.ruleset()) {
        case Ruleset::OSU: {
            OsuPerformanceAttributes value;
            const Status status =
                pppp::osu::difficulty::calculate_performance(value, score, *attributes.osu(), *beatmap);
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::TAIKO: {
            TaikoPerformanceAttributes value;
            const Status status =
                pppp::taiko::difficulty::calculate_performance(value, score, *attributes.taiko(), *beatmap);
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::CATCH: {
            CatchPerformanceAttributes value;
            const Status status =
                pppp::fruits::difficulty::calculate_performance(value, score, *attributes.fruits(), *beatmap);
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::MANIA: {
            ManiaPerformanceAttributes value;
            const Status status =
                pppp::mania::difficulty::calculate_performance(value, score, *attributes.mania(), *beatmap);
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        default: return StatusCode::INVALID_ARGUMENT;
        }
    }
} // namespace pppp
