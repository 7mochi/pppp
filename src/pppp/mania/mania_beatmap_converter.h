// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_MANIA_BEATMAP_CONVERTER_H
#define PPPP_MANIA_MANIA_BEATMAP_CONVERTER_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/mania/mania_beatmap.h"
#include "pppp/mania/patterns/pattern.h"
#include "pppp/mods/mod.h"
#include "pppp/utils/random/osu.h"
#include "pppp/utils/vector2.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania {
    /// Maximum number of previous notes to consider for density calculation.
    const size_t MAX_NOTES_FOR_DENSITY = 7;

    class ManiaBeatmapConverter {
    public:
        const pppp::beatmaps::Beatmap& beatmap;

        /// The number of columns per-stage.
        int target_columns;

        /// Whether to double the number of stages.
        bool dual;

        /// Whether the beatmap instantiated with is for the mania ruleset.
        bool is_for_current_ruleset;

        ManiaBeatmapConverter(const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                              size_t mod_count);

        /// The total number of columns.
        int total_columns() const { return target_columns * (dual ? 2 : 1); }

        void convert(ManiaBeatmap& pb);

        /// The random number generator to use.
        pppp::utils::LegacyRandom random;

        pppp::beatmaps::ObjectConverted<ManiaHitObject> object_converted;

    private:
        const pppp::mods::Mod* mods;
        size_t mod_count;

        /// The last pattern.
        patterns::Pattern last_pattern;

        /// The times of the previous notes, for the density calculation.
        std::vector<double> previous_note_times;

        /// The density of the notes.
        double density;

        double last_time;
        pppp::utils::Vector2 last_position;
        int last_stair;

        void compute_density(double new_note_time);

        void record_note(double time, pppp::utils::Vector2 position);
    };
}} // namespace pppp::mania

#endif
