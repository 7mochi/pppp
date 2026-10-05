#ifndef PPPP_FRUITS_DIFFICULTY_CATCH_STRAINS_H
#define PPPP_FRUITS_DIFFICULTY_CATCH_STRAINS_H

#include <vector>

namespace pppp { namespace fruits { namespace difficulty {
    /// The result of calculating the strains on a osu!catch map.
    ///
    /// Suitable to plot the difficulty of a map over time.
    struct CatchStrains {
        double start_time;
        /// Time between two strains in ms.
        double section_length;
        /// Strain peaks of the movement skill.
        std::vector<double> movement;

        CatchStrains()
            : start_time(0.0),
              section_length(0.0) {}
    };
}}} // namespace pppp::fruits::difficulty

#endif
