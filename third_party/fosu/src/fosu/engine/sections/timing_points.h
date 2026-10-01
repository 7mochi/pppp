#ifndef FOSU_ENGINE_SECTIONS_TIMING_POINTS_H
#define FOSU_ENGINE_SECTIONS_TIMING_POINTS_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/timing_points/point.h>

namespace fosu { namespace internal {

    struct TimingPointsSectionLine {
        Beatmap* beatmap;
        size_t* point_count;
        int time_offset;
        void operator()(StringView line) {
            TimingPoint point;
            if (parse_timing_point(line.data(), line.data() + line.size(), time_offset, point)) {
                beatmap->timing_points[(*point_count)++] = point;
            } else {
                ++beatmap->stats.malformed_lines;
            }
        }
    };

    inline const char* parse_timing_points_section(Beatmap& beatmap, size_t& point_count, const char* p,
                                                   const char* file_end, int time_offset = 0) {
        TimingPointsSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.point_count = &point_count;
        parse_line.time_offset = time_offset;
        return for_each_section_line(p, file_end, parse_line);
    }

}} // namespace fosu::internal

#endif
