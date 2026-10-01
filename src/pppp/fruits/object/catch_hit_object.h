// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_FRUITS_OBJECT_CATCH_HIT_OBJECT_H
#define PPPP_FRUITS_OBJECT_CATCH_HIT_OBJECT_H

#include <vector>

namespace pppp { namespace fruits { namespace object {
    enum ObjectKind {
        OBJECT_FRUIT = 0,
        OBJECT_DROPLET = 1,
        OBJECT_TINY_DROPLET = 2,
        OBJECT_BANANA = 3,
        OBJECT_JUICE_STREAM = 4,
        OBJECT_BANANA_SHOWER = 5
    };

    struct CatchHitObject {
        ObjectKind kind;
        double time;

        /// The horizontal position of the hit object between 0 and PLAYFIELD_WIDTH.
        /// This value is the original X value specified in the beatmap, not affected by the beatmap
        /// processing. Use effective_x() for gameplay.
        double original_x;

        /// A random offset applied to the horizontal position, set by the beatmap processing.
        double x_offset;

        /// Difference between the distance to the next object and the distance that would have
        /// triggered a hyper dash. A value close to 0 indicates a difficult jump (for difficulty
        /// calculation).
        double distance_to_hyper_dash;

        /// Whether this fruit can initiate a hyperdash.
        bool hyper_dash;

        /// The hit objects this one contains, for the kinds that are containers of others: a juice
        /// stream holds its droplets and fruits, a banana shower holds its bananas. Empty for the
        /// objects that are not containers.
        std::vector<CatchHitObject> nested;

        /// The index of the slider of the beatmap this hit object was converted from, or -1. Only
        /// meaningful for a juice stream, whose geometry the beatmap processing needs.
        int slider_index;

        /// The effective horizontal position of the hit object between 0 and PLAYFIELD_WIDTH.
        /// This value is the original X value plus the offset applied by the beatmap processing. Use
        /// original_x if a value not affected by the offset is desired.
        double effective_x() const;

        /// Calculates the width of the area used for attempting catches in gameplay.
        /// @param circle_size The circle size of the beatmap.
        static float calculate_catch_width(double circle_size);
    };

    inline bool hit_object_earlier(const CatchHitObject* a, const CatchHitObject* b) {
        return a->time < b->time;
    }

    const double PLAYFIELD_WIDTH = 512.0;

    const float CATCHER_BASE_SIZE = 106.75f;

    const float CATCHER_ALLOWED_CATCH_RANGE = 0.8f;
    const double CATCHER_BASE_DASH_SPEED = 1.0;

}}} // namespace pppp::fruits::object

#endif
