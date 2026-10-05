#ifndef PPPP_TAIKO_DIFFICULTY_TAIKO_STRAINS_H
#define PPPP_TAIKO_DIFFICULTY_TAIKO_STRAINS_H

#include <vector>

namespace pppp { namespace taiko { namespace difficulty {
    /// The result of calculating the strains on a osu!taiko map.
    ///
    /// Suitable to plot the difficulty of a map over time.
    struct TaikoStrains {
        double start_time;
        /// Time between two strains in ms.
        double section_length;
        /// Strain peaks of the color skill.
        std::vector<double> colour;
        /// Strain peaks of the reading skill.
        std::vector<double> reading;
        /// Strain peaks of the rhythm skill.
        std::vector<double> rhythm;
        /// Strain peaks of the stamina skill.
        std::vector<double> stamina;
        /// Strain peaks of the single color stamina skill.
        std::vector<double> single_colour_stamina;

        TaikoStrains()
            : start_time(0.0),
              section_length(0.0) {}
    };
}}} // namespace pppp::taiko::difficulty

#endif
