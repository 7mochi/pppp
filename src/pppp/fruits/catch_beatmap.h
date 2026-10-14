// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_CATCH_BEATMAP_H
#define PPPP_FRUITS_CATCH_BEATMAP_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/fruits/object/catch_hit_object.h"
#include "pppp/mods/mod.h"
#include "pppp/status.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace fruits {
    struct CatchBeatmap {
        double clock_rate;
        float circle_size;
        float approach_rate;
        bool is_convert;

        std::vector<object::CatchHitObject> objects;
        std::vector<object::CatchHitObject*> all_objects;

        CatchBeatmap();
    };

    Status build(CatchBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                 size_t mod_count,
                 const pppp::beatmaps::ObjectConverted<object::CatchHitObject>& object_converted =
                     pppp::beatmaps::ObjectConverted<object::CatchHitObject>());

    struct ModdedDifficulty {
        float circle_size;
        float approach_rate;
        float overall_difficulty;
        float drain_rate;
    };

    ModdedDifficulty modded_difficulty(const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                       size_t mod_count);

    /// Enumerate all palpable catch hit objects, sorted by their start times.
    /// If multiple objects have the same start time, the ordering is preserved (it is a stable
    /// sorting).
    void palpable_objects(std::vector<object::CatchHitObject*>& out, CatchBeatmap& pb);

    int max_combo(const CatchBeatmap& pb);
}} // namespace pppp::fruits

#endif
