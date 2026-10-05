#ifndef PPPP_MANIA_DIFFICULTY_MANIA_STRAINS_H
#define PPPP_MANIA_DIFFICULTY_MANIA_STRAINS_H

#include <vector>

namespace pppp { namespace mania { namespace difficulty {
    /// The result of calculating the strains on a osu!mania map.
    ///
    /// Suitable to plot the difficulty of a map over time.
    struct ManiaStrains {
        double start_time;
        /// Time between two strains in ms.
        double section_length;
        /// Strain peaks of the strain skill.
        std::vector<double> strain;

        ManiaStrains()
            : start_time(0.0),
              section_length(0.0) {}
    };
}}} // namespace pppp::mania::difficulty

#endif
