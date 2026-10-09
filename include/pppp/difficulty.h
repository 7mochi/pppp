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
        nonstd::variant<OsuDifficultyAttributes, TaikoDifficultyAttributes, CatchDifficultyAttributes,
                        ManiaDifficultyAttributes>
            value;
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
        nonstd::variant<OsuStrains, TaikoStrains, CatchStrains, ManiaStrains> value;
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
        : value() {}

    inline DifficultyAttributes::DifficultyAttributes(const OsuDifficultyAttributes& attributes)
        : value(attributes) {}

    inline DifficultyAttributes::DifficultyAttributes(const TaikoDifficultyAttributes& attributes)
        : value(attributes) {}

    inline DifficultyAttributes::DifficultyAttributes(const CatchDifficultyAttributes& attributes)
        : value(attributes) {}

    inline DifficultyAttributes::DifficultyAttributes(const ManiaDifficultyAttributes& attributes)
        : value(attributes) {}

    inline Ruleset::Value DifficultyAttributes::ruleset() const {
        return static_cast<Ruleset::Value>(value.index());
    }

    inline const OsuDifficultyAttributes* DifficultyAttributes::osu() const {
        return nonstd::get_if<OsuDifficultyAttributes>(&value);
    }

    inline const TaikoDifficultyAttributes* DifficultyAttributes::taiko() const {
        return nonstd::get_if<TaikoDifficultyAttributes>(&value);
    }

    inline const CatchDifficultyAttributes* DifficultyAttributes::fruits() const {
        return nonstd::get_if<CatchDifficultyAttributes>(&value);
    }

    inline const ManiaDifficultyAttributes* DifficultyAttributes::mania() const {
        return nonstd::get_if<ManiaDifficultyAttributes>(&value);
    }

    inline double DifficultyAttributes::star_rating() const {
        switch (value.index()) {
        case Ruleset::OSU: return nonstd::get<OsuDifficultyAttributes>(value).star_rating;
        case Ruleset::TAIKO: return nonstd::get<TaikoDifficultyAttributes>(value).star_rating;
        case Ruleset::CATCH: return nonstd::get<CatchDifficultyAttributes>(value).star_rating;
        case Ruleset::MANIA: return nonstd::get<ManiaDifficultyAttributes>(value).star_rating;
        default: return 0.0;
        }
    }

    inline int DifficultyAttributes::max_combo() const {
        switch (value.index()) {
        case Ruleset::OSU: return nonstd::get<OsuDifficultyAttributes>(value).max_combo;
        case Ruleset::TAIKO: return nonstd::get<TaikoDifficultyAttributes>(value).max_combo;
        case Ruleset::CATCH: return nonstd::get<CatchDifficultyAttributes>(value).max_combo;
        case Ruleset::MANIA: return nonstd::get<ManiaDifficultyAttributes>(value).max_combo;
        default: return 0;
        }
    }

    inline Strains::Strains()
        : value() {}

    inline Strains::Strains(const OsuStrains& strains)
        : value(strains) {}

    inline Strains::Strains(const TaikoStrains& strains)
        : value(strains) {}

    inline Strains::Strains(const CatchStrains& strains)
        : value(strains) {}

    inline Strains::Strains(const ManiaStrains& strains)
        : value(strains) {}

    inline Ruleset::Value Strains::ruleset() const { return static_cast<Ruleset::Value>(value.index()); }

    inline const OsuStrains* Strains::osu() const { return nonstd::get_if<OsuStrains>(&value); }

    inline const TaikoStrains* Strains::taiko() const { return nonstd::get_if<TaikoStrains>(&value); }

    inline const CatchStrains* Strains::fruits() const { return nonstd::get_if<CatchStrains>(&value); }

    inline const ManiaStrains* Strains::mania() const { return nonstd::get_if<ManiaStrains>(&value); }

    inline double Strains::start_time() const {
        switch (value.index()) {
        case Ruleset::OSU: return nonstd::get<OsuStrains>(value).start_time;
        case Ruleset::TAIKO: return nonstd::get<TaikoStrains>(value).start_time;
        case Ruleset::CATCH: return nonstd::get<CatchStrains>(value).start_time;
        case Ruleset::MANIA: return nonstd::get<ManiaStrains>(value).start_time;
        default: return 0.0;
        }
    }

    inline double Strains::section_length() const {
        switch (value.index()) {
        case Ruleset::OSU: return nonstd::get<OsuStrains>(value).section_length;
        case Ruleset::TAIKO: return nonstd::get<TaikoStrains>(value).section_length;
        case Ruleset::CATCH: return nonstd::get<CatchStrains>(value).section_length;
        case Ruleset::MANIA: return nonstd::get<ManiaStrains>(value).section_length;
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
