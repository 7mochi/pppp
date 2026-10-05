#ifndef PPPP_OSU_DIFFICULTY_OSU_STRAINS_H
#define PPPP_OSU_DIFFICULTY_OSU_STRAINS_H

#include <vector>

namespace pppp { namespace osu { namespace difficulty {
    /// The result of calculating the strains on a osu! map.
    ///
    /// Suitable to plot the difficulty of a map over time.
    struct OsuStrains {
        double start_time;
        /// Time between two strains in ms.
        double section_length;
        /// Strain peaks of the aim skill.
        std::vector<double> aim;
        /// Strain peaks of the aim skill without sliders.
        std::vector<double> aim_no_sliders;
        /// Strain peaks of the speed skill.
        std::vector<double> speed;
        std::vector<double> reading;
        /// Strain peaks of the flashlight skill.
        std::vector<double> flashlight;

        OsuStrains()
            : start_time(0.0),
              section_length(0.0) {}
    };
}}} // namespace pppp::osu::difficulty

#endif
