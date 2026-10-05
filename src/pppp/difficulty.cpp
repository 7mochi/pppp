#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_difficulty_calculator.h"
#include "pppp/mania/difficulty/mania_difficulty_calculator.h"
#include "pppp/osu/difficulty/osu_difficulty_calculator.h"
#include "pppp/taiko/difficulty/taiko_difficulty_calculator.h"

namespace pppp {
    namespace {
        template <class Attributes>
        void tag(std::vector<TimedDifficultyAttributes>& out, const std::vector<double>& times,
                 const std::vector<Attributes>& attributes) {
            out.resize(times.size());
            for (size_t i = 0; i < times.size(); i++) {
                out[i].time = times[i];
                out[i].attributes = attributes[i];
            }
        }

        bool is_rate_mod(pppp::mods::ModId id) {
            return id == pppp::mods::MOD_DT || id == pppp::mods::MOD_NC || id == pppp::mods::MOD_HT ||
                   id == pppp::mods::MOD_DC || id == pppp::mods::MOD_WU || id == pppp::mods::MOD_WD ||
                   id == pppp::mods::MOD_AS;
        }
    } // namespace

    Mods Difficulty::effective_mods() const {
        if (!has_clock_rate) {
            return mod_list;
        }

        Mods mods;
        for (Mods::const_iterator it = mod_list.begin(); it != mod_list.end(); ++it) {
            if (!is_rate_mod(it->id)) {
                mods.push_back(*it);
            }
        }

        Mod rate;
        pppp::mods::mod_make(&rate, pppp::mods::MOD_AS);
        rate.variable_rate.initial_rate = clock_rate_value;
        mods.push_back(rate);
        return mods;
    }

    Status Difficulty::calculate(const pppp::beatmaps::Beatmap& map, DifficultyAttributes& out) const {
        out = DifficultyAttributes();
        const Mods mods = effective_mods();
        const Ruleset::Value mode = has_ruleset ? ruleset_value : static_cast<Ruleset::Value>(map.mode);

        switch (mode) {
        case Ruleset::OSU: {
            OsuDifficultyAttributes value;
            const Status status =
                pppp::osu::difficulty::calculate_difficulty(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::TAIKO: {
            TaikoDifficultyAttributes value;
            const Status status =
                pppp::taiko::difficulty::calculate_difficulty(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::CATCH: {
            CatchDifficultyAttributes value;
            const Status status =
                pppp::fruits::difficulty::calculate_difficulty(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::MANIA: {
            ManiaDifficultyAttributes value;
            const Status status =
                pppp::mania::difficulty::calculate_difficulty(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        default: return StatusCode::INVALID_ARGUMENT;
        }
    }

    Status Difficulty::calculate_timed(const pppp::beatmaps::Beatmap& map,
                                       std::vector<TimedDifficultyAttributes>& out) const {
        out.clear();
        const Mods mods = effective_mods();
        const Ruleset::Value mode = has_ruleset ? ruleset_value : static_cast<Ruleset::Value>(map.mode);

        std::vector<double> times;
        switch (mode) {
        case Ruleset::OSU: {
            std::vector<OsuDifficultyAttributes> attributes;
            const Status status = pppp::osu::difficulty::calculate_timed_difficulty(times, attributes, map,
                                                                                    mods.data(), mods.size());
            if (status.ok()) {
                tag(out, times, attributes);
            }
            return status;
        }
        case Ruleset::TAIKO: {
            std::vector<TaikoDifficultyAttributes> attributes;
            const Status status = pppp::taiko::difficulty::calculate_timed_difficulty(
                times, attributes, map, mods.data(), mods.size());
            if (status.ok()) {
                tag(out, times, attributes);
            }
            return status;
        }
        case Ruleset::CATCH: {
            std::vector<CatchDifficultyAttributes> attributes;
            const Status status = pppp::fruits::difficulty::calculate_timed_difficulty(
                times, attributes, map, mods.data(), mods.size());
            if (status.ok()) {
                tag(out, times, attributes);
            }
            return status;
        }
        case Ruleset::MANIA: {
            std::vector<ManiaDifficultyAttributes> attributes;
            const Status status = pppp::mania::difficulty::calculate_timed_difficulty(
                times, attributes, map, mods.data(), mods.size());
            if (status.ok()) {
                tag(out, times, attributes);
            }
            return status;
        }
        default: return StatusCode::INVALID_ARGUMENT;
        }
    }

    Status Difficulty::strains(const pppp::beatmaps::Beatmap& map, Strains& out) const {
        out = Strains();
        const Mods mods = effective_mods();
        const Ruleset::Value mode = has_ruleset ? ruleset_value : static_cast<Ruleset::Value>(map.mode);

        switch (mode) {
        case Ruleset::OSU: {
            OsuStrains value;
            const Status status =
                pppp::osu::difficulty::calculate_strains(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::TAIKO: {
            TaikoStrains value;
            const Status status =
                pppp::taiko::difficulty::calculate_strains(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::CATCH: {
            CatchStrains value;
            const Status status =
                pppp::fruits::difficulty::calculate_strains(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        case Ruleset::MANIA: {
            ManiaStrains value;
            const Status status =
                pppp::mania::difficulty::calculate_strains(value, map, mods.data(), mods.size());
            if (status.ok()) {
                out = value;
            }
            return status;
        }
        default: return StatusCode::INVALID_ARGUMENT;
        }
    }
} // namespace pppp
