#ifndef PPPP_DIFFICULTY_H
#define PPPP_DIFFICULTY_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/fruits/difficulty/catch_difficulty_attributes.h"
#include "pppp/mania/difficulty/mania_difficulty_attributes.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/taiko/difficulty/taiko_difficulty_attributes.h"
#include <cstddef>
#include <vector>

namespace pppp {
    /// Which ruleset a tagged attribute set belongs to. The values are the beatmap's `Mode`.
    struct Ruleset {
        enum Value { RULESET_OSU = 0, RULESET_TAIKO, RULESET_CATCH, RULESET_MANIA };
    };

    /// The four rulesets' difficulty attributes in one value: a tag plus the four mode structs.
    /// C++98 has no variant, so all four are held and the tag says which one is live.
    struct DifficultyAttributes {
        Ruleset::Value ruleset;
        pppp::osu::difficulty::OsuDifficultyAttributes osu;
        pppp::taiko::difficulty::TaikoDifficultyAttributes taiko;
        pppp::fruits::difficulty::CatchDifficultyAttributes fruits;
        pppp::mania::difficulty::ManiaDifficultyAttributes mania;

        DifficultyAttributes();

        /// The live mode's combined star rating.
        double star_rating() const;

        /// The live mode's maximum achievable combo.
        int max_combo() const;
    };

    /// Difficulty calculator on maps of any mode.
    class Difficulty {
    public:
        Difficulty();

        /// Specify mods.
        Difficulty& mods(const pppp::mods::Mod* mods, size_t mod_count);

        /// Calculate for this ruleset instead of the beatmap's own mode, as the ruleset's own
        /// calculator does in upstream. This is what a converted beatmap needs.
        Difficulty& ruleset(Ruleset::Value ruleset);

        /// Adjust the clock rate used in the calculation.
        Difficulty& clock_rate(double clock_rate);

        /// Perform the difficulty calculation for the beatmap's mode.
        DifficultyAttributes calculate(const pppp::beatmaps::Beatmap& map) const;

    private:
        bool is_rate_mod(pppp::mods::ModId id) const;
        std::vector<pppp::mods::Mod> effective_mods() const;

        std::vector<pppp::mods::Mod> mod_list;
        bool has_ruleset;
        Ruleset::Value ruleset_value;
        bool has_clock_rate;
        double clock_rate_value;
    };

    inline DifficultyAttributes::DifficultyAttributes()
        : ruleset(Ruleset::RULESET_OSU),
          osu(),
          taiko(),
          fruits(),
          mania() {}

    inline double DifficultyAttributes::star_rating() const {
        switch (ruleset) {
        case Ruleset::RULESET_OSU: return osu.star_rating;
        case Ruleset::RULESET_TAIKO: return taiko.star_rating;
        case Ruleset::RULESET_CATCH: return fruits.star_rating;
        case Ruleset::RULESET_MANIA: return mania.star_rating;
        default: return 0.0;
        }
    }

    inline int DifficultyAttributes::max_combo() const {
        switch (ruleset) {
        case Ruleset::RULESET_OSU: return osu.max_combo;
        case Ruleset::RULESET_TAIKO: return taiko.max_combo;
        case Ruleset::RULESET_CATCH: return fruits.max_combo;
        case Ruleset::RULESET_MANIA: return mania.max_combo;
        default: return 0;
        }
    }

    inline Difficulty::Difficulty()
        : mod_list(),
          has_ruleset(false),
          ruleset_value(Ruleset::RULESET_OSU),
          has_clock_rate(false),
          clock_rate_value(1.0) {}

    inline Difficulty& Difficulty::mods(const pppp::mods::Mod* mods, size_t mod_count) {
        if (mod_count == 0) {
            mod_list.clear();
        } else {
            mod_list.assign(mods, mods + mod_count);
        }
        return *this;
    }

    inline Difficulty& Difficulty::ruleset(Ruleset::Value ruleset) {
        has_ruleset = true;
        ruleset_value = ruleset;
        return *this;
    }

    inline Difficulty& Difficulty::clock_rate(double clock_rate) {
        has_clock_rate = true;
        clock_rate_value = clock_rate;
        return *this;
    }

    inline bool Difficulty::is_rate_mod(pppp::mods::ModId id) const {
        return id == pppp::mods::MOD_DT || id == pppp::mods::MOD_NC || id == pppp::mods::MOD_HT ||
               id == pppp::mods::MOD_DC || id == pppp::mods::MOD_WU || id == pppp::mods::MOD_WD ||
               id == pppp::mods::MOD_AS;
    }

    inline std::vector<pppp::mods::Mod> Difficulty::effective_mods() const {
        if (!has_clock_rate) {
            return mod_list;
        }

        std::vector<pppp::mods::Mod> mods;
        mods.reserve(mod_list.size() + 1);
        for (size_t i = 0; i < mod_list.size(); i++) {
            if (!is_rate_mod(mod_list[i].id)) {
                mods.push_back(mod_list[i]);
            }
        }

        pppp::mods::Mod rate;
        pppp::mods::mod_make(&rate, pppp::mods::MOD_AS);
        rate.variable_rate.initial_rate = clock_rate_value;
        mods.push_back(rate);
        return mods;
    }
} // namespace pppp

#endif
