// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_TAIKO_BEATMAP_H
#define PPPP_TAIKO_TAIKO_BEATMAP_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/beatmap_converter.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/mods/mod.h"
#include "pppp/status.h"
#include "pppp/taiko/object/taiko_hit_object.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace taiko {
    const int PREDEFINED_DIVISORS[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 16};

    /// Multiplier factor added to the scrolling speed.
    const double EASY_SLIDER_MULTIPLIER = 0.8;

    /// Multiplier factor added to the scrolling speed.
    /// @remarks This factor is made up of two parts: the base part (1.4) and the aspect ratio adjustment
    /// (4/3). Stable applies the latter by dividing the width of the user's display by the width of a
    /// display with the same height, but 4:3 aspect ratio.
    /// TODO: Revisit if taiko playfield ever changes away from a hard-coded 16:9 (see
    /// https://github.com/ppy/osu/issues/5685).
    const double HARD_ROCK_SLIDER_MULTIPLIER = 1.4 * 4 / 3;

    bool hit_object_less(const TaikoHitObject& a, const TaikoHitObject& b);

    struct TaikoBeatmap {
        double clock_rate;
        double overall_difficulty;
        double drain_rate;
        double slider_multiplier;
        bool is_convert;

        std::vector<TaikoHitObject> objects;
        pppp::beatmaps::control_points::ControlPointInfo info;

        TaikoBeatmap();
    };

    struct ModdedDifficulty {
        double overall_difficulty;
        double drain_rate;
        double slider_multiplier;
    };

    ModdedDifficulty modded_difficulty(const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                       size_t mod_count);

    Status build(TaikoBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                 size_t mod_count,
                 const pppp::beatmaps::ObjectConverted<TaikoHitObject>& object_converted =
                     pppp::beatmaps::ObjectConverted<TaikoHitObject>());

    double beat_length_at(const TaikoBeatmap& pb, double time);

    double scroll_speed_at(const TaikoBeatmap& pb, double time);

    int max_combo(const TaikoBeatmap& pb);
}} // namespace pppp::taiko

#endif
