#ifndef PPPP_DIFFICULTY_H
#define PPPP_DIFFICULTY_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/fruits/difficulty/catch_difficulty_attributes.h"
#include "pppp/fruits/difficulty/catch_strains.h"
#include "pppp/mania/difficulty/mania_difficulty_attributes.h"
#include "pppp/mania/difficulty/mania_strains.h"
#include "pppp/mods/mod.h"
#include "pppp/mods/mods.h"
#include "pppp/osu/difficulty/osu_difficulty_attributes.h"
#include "pppp/osu/difficulty/osu_strains.h"
#include "pppp/status.h"
#include "pppp/taiko/difficulty/taiko_difficulty_attributes.h"
#include "pppp/taiko/difficulty/taiko_strains.h"
#include <vector>

namespace pppp {
    typedef pppp::mods::Mod Mod;
    typedef pppp::mods::Mods Mods;
    typedef pppp::osu::difficulty::OsuDifficultyAttributes OsuDifficultyAttributes;
    typedef pppp::taiko::difficulty::TaikoDifficultyAttributes TaikoDifficultyAttributes;
    typedef pppp::fruits::difficulty::CatchDifficultyAttributes CatchDifficultyAttributes;
    typedef pppp::mania::difficulty::ManiaDifficultyAttributes ManiaDifficultyAttributes;
    typedef pppp::osu::difficulty::OsuStrains OsuStrains;
    typedef pppp::taiko::difficulty::TaikoStrains TaikoStrains;
    typedef pppp::fruits::difficulty::CatchStrains CatchStrains;
    typedef pppp::mania::difficulty::ManiaStrains ManiaStrains;

    /// Which ruleset a tagged attribute set belongs to. The values are the beatmap's `Mode`.
    struct Ruleset {
        enum Value { OSU = 0, TAIKO, CATCH, MANIA };
    };

    /// The four rulesets' difficulty attributes in one value: a tag plus the four mode structs.
    /// C++98 has no variant, so all four are held and the tag says which one is live.
    class DifficultyAttributes {
    public:
        DifficultyAttributes();
        DifficultyAttributes(const OsuDifficultyAttributes& attributes);
        DifficultyAttributes(const TaikoDifficultyAttributes& attributes);
        DifficultyAttributes(const CatchDifficultyAttributes& attributes);
        DifficultyAttributes(const ManiaDifficultyAttributes& attributes);

        Ruleset::Value ruleset() const;

        const OsuDifficultyAttributes* osu() const;
        const TaikoDifficultyAttributes* taiko() const;
        const CatchDifficultyAttributes* fruits() const;
        const ManiaDifficultyAttributes* mania() const;

        /// The live mode's combined star rating.
        double star_rating() const;

        /// The live mode's maximum achievable combo.
        int max_combo() const;

    private:
        Ruleset::Value tag;
        OsuDifficultyAttributes osu_value;
        TaikoDifficultyAttributes taiko_value;
        CatchDifficultyAttributes fruits_value;
        ManiaDifficultyAttributes mania_value;
    };

    /// Wraps a DifficultyAttributes object and adds a time value for which the attribute is valid.
    /// Output by `Difficulty::calculate_timed`.
    struct TimedDifficultyAttributes {
        /// The non-clock-adjusted time value at which the attributes take effect.
        double time;

        /// The attributes.
        DifficultyAttributes attributes;
    };

    /// The result of calculating the strains on a map.
    ///
    /// Suitable to plot the difficulty of a map over time.
    class Strains {
    public:
        Strains();
        Strains(const OsuStrains& strains);
        Strains(const TaikoStrains& strains);
        Strains(const CatchStrains& strains);
        Strains(const ManiaStrains& strains);

        Ruleset::Value ruleset() const;

        const OsuStrains* osu() const;
        const TaikoStrains* taiko() const;
        const CatchStrains* fruits() const;
        const ManiaStrains* mania() const;

        double start_time() const;

        /// Time inbetween two strains in ms.
        double section_length() const;

    private:
        Ruleset::Value tag;
        OsuStrains osu_value;
        TaikoStrains taiko_value;
        CatchStrains fruits_value;
        ManiaStrains mania_value;
    };

    /// Difficulty calculator on maps of any mode.
    class Difficulty {
    public:
        Difficulty();

        /// Specify mods.
        Difficulty& mods(const Mods& mods);

        /// Calculate for this ruleset instead of the beatmap's own mode, as the ruleset's own
        /// calculator does in upstream. This is what a converted beatmap needs.
        Difficulty& ruleset(Ruleset::Value ruleset);

        /// Adjust the clock rate used in the calculation.
        Difficulty& clock_rate(double clock_rate);

        /// Perform the difficulty calculation for the beatmap's mode.
        Status calculate(const pppp::beatmaps::Beatmap& map, DifficultyAttributes& out) const;

        /// Calculates the difficulty of the beatmap using a specific mod combination and returns a set of
        /// TimedDifficultyAttributes representing the difficulty at every relevant time value in the
        /// beatmap.
        Status calculate_timed(const pppp::beatmaps::Beatmap& map,
                               std::vector<TimedDifficultyAttributes>& out) const;

        /// Perform the difficulty calculation but instead of evaluating the skill
        /// strains, return them as is.
        ///
        /// Suitable to plot the difficulty of a map over time.
        Status strains(const pppp::beatmaps::Beatmap& map, Strains& out) const;

    private:
        Mods effective_mods() const;

        Mods mod_list;
        bool has_ruleset;
        Ruleset::Value ruleset_value;
        bool has_clock_rate;
        double clock_rate_value;
    };

    inline DifficultyAttributes::DifficultyAttributes()
        : tag(Ruleset::OSU),
          osu_value(),
          taiko_value(),
          fruits_value(),
          mania_value() {}

    inline DifficultyAttributes::DifficultyAttributes(const OsuDifficultyAttributes& attributes)
        : tag(Ruleset::OSU),
          osu_value(attributes),
          taiko_value(),
          fruits_value(),
          mania_value() {}

    inline DifficultyAttributes::DifficultyAttributes(const TaikoDifficultyAttributes& attributes)
        : tag(Ruleset::TAIKO),
          osu_value(),
          taiko_value(attributes),
          fruits_value(),
          mania_value() {}

    inline DifficultyAttributes::DifficultyAttributes(const CatchDifficultyAttributes& attributes)
        : tag(Ruleset::CATCH),
          osu_value(),
          taiko_value(),
          fruits_value(attributes),
          mania_value() {}

    inline DifficultyAttributes::DifficultyAttributes(const ManiaDifficultyAttributes& attributes)
        : tag(Ruleset::MANIA),
          osu_value(),
          taiko_value(),
          fruits_value(),
          mania_value(attributes) {}

    inline Ruleset::Value DifficultyAttributes::ruleset() const { return tag; }

    inline const OsuDifficultyAttributes* DifficultyAttributes::osu() const {
        return tag == Ruleset::OSU ? &osu_value : 0;
    }

    inline const TaikoDifficultyAttributes* DifficultyAttributes::taiko() const {
        return tag == Ruleset::TAIKO ? &taiko_value : 0;
    }

    inline const CatchDifficultyAttributes* DifficultyAttributes::fruits() const {
        return tag == Ruleset::CATCH ? &fruits_value : 0;
    }

    inline const ManiaDifficultyAttributes* DifficultyAttributes::mania() const {
        return tag == Ruleset::MANIA ? &mania_value : 0;
    }

    inline double DifficultyAttributes::star_rating() const {
        switch (tag) {
        case Ruleset::OSU: return osu_value.star_rating;
        case Ruleset::TAIKO: return taiko_value.star_rating;
        case Ruleset::CATCH: return fruits_value.star_rating;
        case Ruleset::MANIA: return mania_value.star_rating;
        default: return 0.0;
        }
    }

    inline int DifficultyAttributes::max_combo() const {
        switch (tag) {
        case Ruleset::OSU: return osu_value.max_combo;
        case Ruleset::TAIKO: return taiko_value.max_combo;
        case Ruleset::CATCH: return fruits_value.max_combo;
        case Ruleset::MANIA: return mania_value.max_combo;
        default: return 0;
        }
    }

    inline Strains::Strains()
        : tag(Ruleset::OSU),
          osu_value(),
          taiko_value(),
          fruits_value(),
          mania_value() {}

    inline Strains::Strains(const OsuStrains& strains)
        : tag(Ruleset::OSU),
          osu_value(strains),
          taiko_value(),
          fruits_value(),
          mania_value() {}

    inline Strains::Strains(const TaikoStrains& strains)
        : tag(Ruleset::TAIKO),
          osu_value(),
          taiko_value(strains),
          fruits_value(),
          mania_value() {}

    inline Strains::Strains(const CatchStrains& strains)
        : tag(Ruleset::CATCH),
          osu_value(),
          taiko_value(),
          fruits_value(strains),
          mania_value() {}

    inline Strains::Strains(const ManiaStrains& strains)
        : tag(Ruleset::MANIA),
          osu_value(),
          taiko_value(),
          fruits_value(),
          mania_value(strains) {}

    inline Ruleset::Value Strains::ruleset() const { return tag; }

    inline const OsuStrains* Strains::osu() const { return tag == Ruleset::OSU ? &osu_value : 0; }

    inline const TaikoStrains* Strains::taiko() const { return tag == Ruleset::TAIKO ? &taiko_value : 0; }

    inline const CatchStrains* Strains::fruits() const { return tag == Ruleset::CATCH ? &fruits_value : 0; }

    inline const ManiaStrains* Strains::mania() const { return tag == Ruleset::MANIA ? &mania_value : 0; }

    inline double Strains::start_time() const {
        switch (tag) {
        case Ruleset::OSU: return osu_value.start_time;
        case Ruleset::TAIKO: return taiko_value.start_time;
        case Ruleset::CATCH: return fruits_value.start_time;
        case Ruleset::MANIA: return mania_value.start_time;
        default: return 0.0;
        }
    }

    inline double Strains::section_length() const {
        switch (tag) {
        case Ruleset::OSU: return osu_value.section_length;
        case Ruleset::TAIKO: return taiko_value.section_length;
        case Ruleset::CATCH: return fruits_value.section_length;
        case Ruleset::MANIA: return mania_value.section_length;
        default: return 0.0;
        }
    }

    inline Difficulty::Difficulty()
        : mod_list(),
          has_ruleset(false),
          ruleset_value(Ruleset::OSU),
          has_clock_rate(false),
          clock_rate_value(1.0) {}

    inline Difficulty& Difficulty::mods(const Mods& mods) {
        mod_list = mods;
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
} // namespace pppp

#endif
