// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_MANIA_BEATMAP_H
#define PPPP_MANIA_MANIA_BEATMAP_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/config.h"
#include "pppp/mania/object/mania_hit_object.h"
#include "pppp/mods/mod.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania {
    /// The maximum number of supported keys in a single stage.
    const int MAX_STAGE_KEYS = 10;

    struct ManiaBeatmap {
        std::vector<ManiaHitObject> objects;
        int target_columns;
        bool dual;
        double clock_rate;
        bool is_for_current_ruleset;

        /// Total number of columns represented by all stages in this beatmap.
        int total_columns() const { return target_columns * (dual ? 2 : 1); }

        ManiaBeatmap();
    };

    Result::Value build(ManiaBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                        size_t mod_count);

    int max_combo(const ManiaBeatmap& pb);
}} // namespace pppp::mania

#endif
