#ifndef PPPP_PERFORMANCE_H
#define PPPP_PERFORMANCE_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/common/score_info.h"
#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_performance_attributes.h"
#include "pppp/mania/difficulty/mania_performance_attributes.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/osu_performance_attributes.h"
#include "pppp/taiko/difficulty/taiko_performance_attributes.h"
#include <cstddef>
#include <vector>

namespace pppp {
    /// The four rulesets' performance attributes in one value: a tag plus the four mode structs.
    struct PerformanceAttributes {
        Ruleset::Value ruleset;
        pppp::osu::difficulty::OsuPerformanceAttributes osu;
        pppp::taiko::difficulty::TaikoPerformanceAttributes taiko;
        pppp::fruits::difficulty::CatchPerformanceAttributes fruits;
        pppp::mania::difficulty::ManiaPerformanceAttributes mania;

        PerformanceAttributes();

        /// The live mode's total performance points.
        double total() const;
    };

    /// Performance calculator on maps of any mode.
    class Performance {
    public:
        /// Create a performance calculator that will calculate the difficulty itself.
        explicit Performance(const pppp::beatmaps::Beatmap& map);

        /// Create a performance calculator from already-calculated attributes, skipping the
        /// difficulty calculation.
        Performance(const pppp::beatmaps::Beatmap& map, const DifficultyAttributes& attributes);

        /// Use the given already-calculated attributes, skipping the difficulty calculation.
        Performance& attributes(const DifficultyAttributes& attributes);

        /// Specify mods.
        Performance& mods(const pppp::mods::Mod* mods, size_t mod_count);

        /// Specify the max combo of the play.
        Performance& combo(int combo);

        /// Set the accuracy between `0.0` and `1.0`.
        Performance& accuracy(double accuracy);

        /// Specify the amount of misses of the play.
        Performance& misses(int misses);

        /// Provide the score state through a `ScoreInfo`.
        Performance& state(const pppp::common::ScoreInfo& state);

        /// Perform the performance calculation for the map's or the attributes' mode.
        PerformanceAttributes calculate() const;

    private:
        const pppp::beatmaps::Beatmap* beatmap;
        DifficultyAttributes difficulty_attributes;
        bool has_attributes;
        std::vector<pppp::mods::Mod> mod_list;
        bool has_mods;
        pppp::common::ScoreInfo score_state;
        bool has_combo;
        bool has_accuracy;
        bool has_misses;
        int combo_value;
        double accuracy_value;
        int misses_value;
    };

    inline PerformanceAttributes::PerformanceAttributes()
        : ruleset(Ruleset::RULESET_OSU),
          osu(),
          taiko(),
          fruits(),
          mania() {}

    inline double PerformanceAttributes::total() const {
        switch (ruleset) {
        case Ruleset::RULESET_OSU: return osu.total;
        case Ruleset::RULESET_TAIKO: return taiko.total;
        case Ruleset::RULESET_CATCH: return fruits.total;
        case Ruleset::RULESET_MANIA: return mania.total;
        default: return 0.0;
        }
    }

    inline Performance::Performance(const pppp::beatmaps::Beatmap& map)
        : beatmap(&map),
          difficulty_attributes(),
          has_attributes(false),
          mod_list(),
          has_mods(false),
          score_state(),
          has_combo(false),
          has_accuracy(false),
          has_misses(false),
          combo_value(0),
          accuracy_value(0.0),
          misses_value(0) {}

    inline Performance::Performance(const pppp::beatmaps::Beatmap& map,
                                    const DifficultyAttributes& attributes)
        : beatmap(&map),
          difficulty_attributes(attributes),
          has_attributes(true),
          mod_list(),
          has_mods(false),
          score_state(),
          has_combo(false),
          has_accuracy(false),
          has_misses(false),
          combo_value(0),
          accuracy_value(0.0),
          misses_value(0) {}

    inline Performance& Performance::attributes(const DifficultyAttributes& attributes) {
        difficulty_attributes = attributes;
        has_attributes = true;
        return *this;
    }

    inline Performance& Performance::mods(const pppp::mods::Mod* mods, size_t mod_count) {
        if (mod_count == 0) {
            mod_list.clear();
        } else {
            mod_list.assign(mods, mods + mod_count);
        }
        has_mods = true;
        return *this;
    }

    inline Performance& Performance::combo(int combo) {
        has_combo = true;
        combo_value = combo;
        return *this;
    }

    inline Performance& Performance::accuracy(double accuracy) {
        has_accuracy = true;
        accuracy_value = accuracy;
        return *this;
    }

    inline Performance& Performance::misses(int misses) {
        has_misses = true;
        misses_value = misses;
        return *this;
    }

    inline Performance& Performance::state(const pppp::common::ScoreInfo& state) {
        score_state = state;
        return *this;
    }
} // namespace pppp

#endif
