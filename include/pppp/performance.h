#ifndef PPPP_PERFORMANCE_H
#define PPPP_PERFORMANCE_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/common/hit_result.h"
#include "pppp/common/score_info.h"
#include "pppp/difficulty.h"
#include "pppp/fruits/difficulty/catch_performance_attributes.h"
#include "pppp/mania/difficulty/mania_performance_attributes.h"
#include "pppp/osu/difficulty/osu_performance_attributes.h"
#include "pppp/status.h"
#include "pppp/taiko/difficulty/taiko_performance_attributes.h"

namespace pppp {
    typedef pppp::common::ScoreInfo ScoreInfo;
    typedef pppp::common::HitResult HitResult;
    using pppp::common::HIT_RESULT_COMBO_BREAK;
    using pppp::common::HIT_RESULT_COUNT;
    using pppp::common::HIT_RESULT_GOOD;
    using pppp::common::HIT_RESULT_GREAT;
    using pppp::common::HIT_RESULT_IGNORE_HIT;
    using pppp::common::HIT_RESULT_IGNORE_MISS;
    using pppp::common::HIT_RESULT_LARGE_BONUS;
    using pppp::common::HIT_RESULT_LARGE_TICK_HIT;
    using pppp::common::HIT_RESULT_LARGE_TICK_MISS;
    using pppp::common::HIT_RESULT_LEGACY_COMBO_INCREASE;
    using pppp::common::HIT_RESULT_MEH;
    using pppp::common::HIT_RESULT_MISS;
    using pppp::common::HIT_RESULT_NONE;
    using pppp::common::HIT_RESULT_OK;
    using pppp::common::HIT_RESULT_PERFECT;
    using pppp::common::HIT_RESULT_SLIDER_TAIL_HIT;
    using pppp::common::HIT_RESULT_SMALL_BONUS;
    using pppp::common::HIT_RESULT_SMALL_TICK_HIT;
    using pppp::common::HIT_RESULT_SMALL_TICK_MISS;

    typedef pppp::osu::difficulty::OsuPerformanceAttributes OsuPerformanceAttributes;
    typedef pppp::taiko::difficulty::TaikoPerformanceAttributes TaikoPerformanceAttributes;
    typedef pppp::fruits::difficulty::CatchPerformanceAttributes CatchPerformanceAttributes;
    typedef pppp::mania::difficulty::ManiaPerformanceAttributes ManiaPerformanceAttributes;

    /// The four rulesets' performance attributes in one value: a tag plus the four mode structs.
    class PerformanceAttributes {
    public:
        PerformanceAttributes();
        PerformanceAttributes(const OsuPerformanceAttributes& attributes);
        PerformanceAttributes(const TaikoPerformanceAttributes& attributes);
        PerformanceAttributes(const CatchPerformanceAttributes& attributes);
        PerformanceAttributes(const ManiaPerformanceAttributes& attributes);

        Ruleset::Value ruleset() const;

        const OsuPerformanceAttributes* osu() const;
        const TaikoPerformanceAttributes* taiko() const;
        const CatchPerformanceAttributes* fruits() const;
        const ManiaPerformanceAttributes* mania() const;

        /// The live mode's total performance points.
        double total() const;

    private:
        Ruleset::Value tag;
        OsuPerformanceAttributes osu_value;
        TaikoPerformanceAttributes taiko_value;
        CatchPerformanceAttributes fruits_value;
        ManiaPerformanceAttributes mania_value;
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
        Performance& mods(const Mods& mods);

        /// Specify the max combo of the play.
        Performance& combo(int combo);

        /// Set the accuracy between `0.0` and `1.0`.
        Performance& accuracy(double accuracy);

        /// Specify the amount of misses of the play.
        Performance& misses(int misses);

        /// Provide the score state through a `ScoreInfo`.
        Performance& score(const ScoreInfo& score);

        /// Perform the performance calculation for the map's or the attributes' mode.
        Status calculate(PerformanceAttributes& out) const;

    private:
        const pppp::beatmaps::Beatmap* beatmap;
        DifficultyAttributes difficulty_attributes;
        bool has_attributes;
        Mods mod_list;
        bool has_mods;
        ScoreInfo score_state;
        bool has_combo;
        bool has_accuracy;
        bool has_misses;
        int combo_value;
        double accuracy_value;
        int misses_value;
    };

    inline PerformanceAttributes::PerformanceAttributes()
        : tag(Ruleset::OSU),
          osu_value(),
          taiko_value(),
          fruits_value(),
          mania_value() {}

    inline PerformanceAttributes::PerformanceAttributes(const OsuPerformanceAttributes& attributes)
        : tag(Ruleset::OSU),
          osu_value(attributes),
          taiko_value(),
          fruits_value(),
          mania_value() {}

    inline PerformanceAttributes::PerformanceAttributes(const TaikoPerformanceAttributes& attributes)
        : tag(Ruleset::TAIKO),
          osu_value(),
          taiko_value(attributes),
          fruits_value(),
          mania_value() {}

    inline PerformanceAttributes::PerformanceAttributes(const CatchPerformanceAttributes& attributes)
        : tag(Ruleset::CATCH),
          osu_value(),
          taiko_value(),
          fruits_value(attributes),
          mania_value() {}

    inline PerformanceAttributes::PerformanceAttributes(const ManiaPerformanceAttributes& attributes)
        : tag(Ruleset::MANIA),
          osu_value(),
          taiko_value(),
          fruits_value(),
          mania_value(attributes) {}

    inline Ruleset::Value PerformanceAttributes::ruleset() const { return tag; }

    inline const OsuPerformanceAttributes* PerformanceAttributes::osu() const {
        return tag == Ruleset::OSU ? &osu_value : 0;
    }

    inline const TaikoPerformanceAttributes* PerformanceAttributes::taiko() const {
        return tag == Ruleset::TAIKO ? &taiko_value : 0;
    }

    inline const CatchPerformanceAttributes* PerformanceAttributes::fruits() const {
        return tag == Ruleset::CATCH ? &fruits_value : 0;
    }

    inline const ManiaPerformanceAttributes* PerformanceAttributes::mania() const {
        return tag == Ruleset::MANIA ? &mania_value : 0;
    }

    inline double PerformanceAttributes::total() const {
        switch (tag) {
        case Ruleset::OSU: return osu_value.total;
        case Ruleset::TAIKO: return taiko_value.total;
        case Ruleset::CATCH: return fruits_value.total;
        case Ruleset::MANIA: return mania_value.total;
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

    inline Performance& Performance::mods(const Mods& mods) {
        mod_list = mods;
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

    inline Performance& Performance::score(const ScoreInfo& score) {
        score_state = score;
        return *this;
    }
} // namespace pppp

#endif
