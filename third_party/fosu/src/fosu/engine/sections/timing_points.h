#ifndef FOSU_ENGINE_SECTIONS_TIMING_POINTS_H
#define FOSU_ENGINE_SECTIONS_TIMING_POINTS_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/engine/timing_points/point.h>
#include <fosu/format.h>

namespace fosu { namespace internal {
    struct TimingPointsSectionLine {
        Beatmap* beatmap;
        size_t* point_count;
        int time_offset;
        void operator()(nonstd::string_view line) {
            TimingPoint point;
            if (parse_timing_point(line.data(), line.data() + line.size(), time_offset, point)) {
                beatmap->timing_points[(*point_count)++] = point;
            } else {
                ++beatmap->stats.malformed_lines;
            }
        }
    };

#if FOSU_SIMD
    inline const char* parse_timing_points_section_simd(Beatmap& beatmap, size_t& point_count, const char* p,
                                                        const char* file_end, int time_offset,
                                                        Client::Value client) {
        while (p < file_end) {
            TimingPoint point;
            // The next row's address comes from one vector load rather than from this row's
            // fields; a row of at most 32 bytes has its ending in p[0, 32).
            const u32 ending = first_line_end32(load32(p));
            if (ending < 32) {
                const char* line_end = p + ending;
                if (parse_timing_point(p, line_end, time_offset, point)) {
                    beatmap.timing_points[point_count++] = point;
                    p = after_line_ending(line_end, file_end);
                    continue;
                }
            }
            // Blank lines, comments, a section header, or another spelling.
            if (*p == '\r' || *p == '\n') {
                ++p;
                continue;
            }
            const Line line = read_line(p, file_end);
            const char* line_end = p + line.text.size();
            if (section_header_line(p, line_end, client)) {
                break;
            }
            if (!ignored_line(p, line_end)) {
                if (parse_timing_point(p, line_end, time_offset, point)) {
                    beatmap.timing_points[point_count++] = point;
                } else {
                    ++beatmap.stats.malformed_lines;
                }
            }
            p = line.next;
        }
        return p;
    }
#endif

    inline const char* parse_timing_points_section_scalar(Beatmap& beatmap, size_t& point_count,
                                                          const char* p, const char* file_end,
                                                          const ParseContext& context) {
        TimingPointsSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.point_count = &point_count;
        parse_line.time_offset = context.format.time_offset;
        return for_each_section_line(p, file_end, parse_line, context.client);
    }

    inline const char* parse_timing_points_section(Beatmap& beatmap, size_t& point_count, const char* p,
                                                   const char* file_end, const ParseContext& context) {
#if FOSU_SIMD
        return parse_timing_points_section_simd(beatmap, point_count, p, file_end, context.format.time_offset,
                                                context.client);
#else
        return parse_timing_points_section_scalar(beatmap, point_count, p, file_end, context);
#endif
    }
}} // namespace fosu::internal

#endif
