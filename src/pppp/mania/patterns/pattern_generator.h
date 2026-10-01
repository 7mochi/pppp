// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_PATTERN_GENERATOR_H
#define PPPP_MANIA_PATTERNS_PATTERN_GENERATOR_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/beatmaps/control_points/control_point_info.h"
#include "pppp/mania/patterns/pattern.h"
#include "pppp/utils/vector2.h"
#include <vector>

namespace pppp { namespace mania { namespace patterns {
    enum PatternType {
        PATTERN_NONE = 0,

        /// Keep the same as last row.
        PATTERN_FORCE_STACK = 1 << 0,

        /// Keep different from last row.
        PATTERN_FORCE_NOT_STACK = 1 << 1,

        /// Keep as single note at its original position.
        PATTERN_KEEP_SINGLE = 1 << 2,

        /// Use a lower random value.
        PATTERN_LOW_PROBABILITY = 1 << 3,

        /// Reserved.
        PATTERN_ALTERNATE = 1 << 4,

        /// Ignore the repeat count.
        PATTERN_FORCE_SIG_SLIDER = 1 << 5,

        /// Convert slider to circle.
        PATTERN_FORCE_NOT_SLIDER = 1 << 6,

        /// Notes gathered together.
        PATTERN_GATHERED = 1 << 7,
        PATTERN_MIRROR = 1 << 8,

        /// Change 0 -> 6.
        PATTERN_REVERSE = 1 << 9,

        /// 1 -> 5 -> 1 -> 5 like reverse.
        PATTERN_CYCLE = 1 << 10,

        /// Next note will be at column + 1.
        PATTERN_STAIR = 1 << 11,

        /// Next note will be at column - 1.
        PATTERN_REVERSE_STAIR = 1 << 12
    };

    /// The fields of the source hit object the generators read, flattened out of the decoded beatmap
    /// model so a generator does not depend on it.
    struct SourceObject {
        double start_time;
        double end_time;
        pppp::utils::Vector2 position;
        unsigned hitsound;
        const std::vector<unsigned>* node_sounds;
        int slides;
        double expected_distance;
        bool has_path;
        bool has_duration;

        SourceObject()
            : start_time(0.0),
              end_time(0.0),
              hitsound(0),
              node_sounds(0),
              slides(1),
              expected_distance(0.0),
              has_path(false),
              has_duration(false) {
            position.x = 0.0;
            position.y = 0.0;
        }
    };

    /// Generator to create a pattern from a hit object.
    class PatternGenerator {
    public:
        /// The hit object to create the pattern for.
        const SourceObject& hit_object;

        /// The beatmap which the hit object is a part of.
        const pppp::beatmaps::Beatmap& beatmap;

        const pppp::beatmaps::control_points::ControlPointInfo& info;

        /// The last pattern.
        const Pattern& previous_pattern;

        int total_columns;

        PatternGenerator(const pppp::beatmaps::control_points::ControlPointInfo& info_in,
                         const pppp::beatmaps::Beatmap& beatmap_in, const SourceObject& hit_object_in,
                         const Pattern& previous_pattern_in, int total_columns_in)
            : hit_object(hit_object_in),
              beatmap(beatmap_in),
              info(info_in),
              previous_pattern(previous_pattern_in),
              total_columns(total_columns_in) {}

        virtual ~PatternGenerator() {}

        /// Generates the patterns for the hit object, each filled with hit objects.
        /// @param out The patterns containing the hit objects.
        virtual void generate(std::vector<Pattern>& out) = 0;
    };
}}} // namespace pppp::mania::patterns

#endif
