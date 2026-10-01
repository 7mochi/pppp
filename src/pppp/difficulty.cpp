#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/osu/difficulty/osu_difficulty_calculator.h"
#include "pppp/taiko/difficulty/taiko_difficulty_calculator.h"

namespace pppp {
    DifficultyAttributes Difficulty::calculate(const pppp::beatmaps::Beatmap& map) const {
        std::vector<pppp::mods::Mod> mods = effective_mods();
        const pppp::mods::Mod* mod_ptr = mods.empty() ? 0 : &mods[0];
        size_t mod_count = mods.size();
        const Ruleset::Value mode = has_ruleset ? ruleset_value : static_cast<Ruleset::Value>(map.mode);

        DifficultyAttributes out;
        switch (mode) {
        case Ruleset::RULESET_OSU:
            out.ruleset = Ruleset::RULESET_OSU;
            pppp::osu::difficulty::calculate_difficulty(out.osu, map, mod_ptr, mod_count);
            break;
        case Ruleset::RULESET_TAIKO:
            out.ruleset = Ruleset::RULESET_TAIKO;
            pppp::taiko::difficulty::calculate_difficulty(out.taiko, map, mod_ptr, mod_count);
            break;
        case Ruleset::RULESET_CATCH:
            out.ruleset = Ruleset::RULESET_CATCH;
            pppp::fruits::difficulty::calculate_difficulty(out.fruits, map, mod_ptr, mod_count);
            break;
        case Ruleset::RULESET_MANIA:
            out.ruleset = Ruleset::RULESET_MANIA;
            pppp::mania::difficulty::calculate_difficulty(out.mania, map, mod_ptr, mod_count);
            break;
        default: break;
        }
        return out;
    }
} // namespace pppp
