#ifndef FOSU_ENGINE_SECTIONS_EVENTS_H
#define FOSU_ENGINE_SECTIONS_EVENTS_H

#include <algorithm>

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/format.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/parsing/string_lookup.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/types.h>

namespace fosu { namespace internal {
    inline nonstd::string_view strip_quotes(nonstd::string_view v) {
        if (v.size() >= 2 && v.front() == '"' && v.back() == '"') {
            return v.substr(1, v.size() - 2);
        }
        return v;
    }

    inline nonstd::optional<nonstd::string_view> parse_event_filename(const char* rest, const char* end) {
#if FOSU_SIMD
        // Timestamp and filename usually fit in one window. Reuse its comma mask
        // for both boundaries without exposing SIMD state to the event handlers.
        if (rest < end) {
            const u32 commas = equal_mask32(load32(rest), broadcast_byte<','>());
            const u32 first = trailing_zeros(commas);
            if (first < 32 && first < static_cast<size_t>(end - rest)) {
                const char* filename = rest + first + 1;
                const u32 second = trailing_zeros(commas & (commas - 1));
                const char* next = second < 32 && second < static_cast<size_t>(end - rest)
                                       ? rest + second
                                       : find_byte(',', filename, end);
                return nonstd::optional<nonstd::string_view>(strip_quotes(trim(filename, next)));
            }
        }
#endif
        const char* comma = find_byte(',', rest, end);
        if (comma == end) {
            return nonstd::optional<nonstd::string_view>();
        }
        const char* filename = comma + 1;
        const char* next = find_byte(',', filename, end);
        return nonstd::optional<nonstd::string_view>(strip_quotes(trim(filename, next)));
    }

    inline void parse_background_event(Beatmap& bm, size_t&, const char* rest, const char* end, int) {
        const nonstd::optional<nonstd::string_view> filename = parse_event_filename(rest, end);
        if (filename.has_value()) {
            bm.background = *filename;
        }
    }

    inline void parse_video_event(Beatmap& bm, size_t&, const char* rest, const char* end, int) {
        const nonstd::optional<nonstd::string_view> filename = parse_event_filename(rest, end);
        if (filename.has_value()) {
            bm.video = *filename;
        }
    }

    inline void parse_break_event(Beatmap& bm, size_t& break_count, const char* rest, const char* end,
                                  int time_offset) {
        double start, stop;
        const char* q = parse_osu_double(rest, end, start);
        if (q == rest || q >= end || *q != ',') {
            ++bm.stats.malformed_lines;
            return;
        }
        // osu! ignores any fields after the end time.
        const char* r = parse_osu_double(q + 1, end, stop);
        if (r == q + 1 || (r != end && *r != ',')) {
            ++bm.stats.malformed_lines;
            return;
        }
        start += time_offset;
        Break period;
        period.start = start;
        period.end = std::max(start, stop + time_offset);
        bm.breaks[break_count++] = period;
    }

    typedef void (*EventHandler)(Beatmap&, size_t&, const char*, const char*, int);
    extern const StringLookup<EventHandler> kEventHandlers;

    inline void parse_event_line(Beatmap& bm, size_t& break_count, const char* p, size_t len,
                                 int time_offset) {
        // Storyboard commands are indented; count and skip them.
        if (len == 0 || *p == ' ' || *p == '_') {
            ++bm.stats.storyboard_lines;
            return;
        }
        const char* end = p + len;
        const char* c1 = find_byte(',', p, end);
        if (c1 == end) {
            ++bm.stats.storyboard_lines;
            return;
        }
        const nonstd::string_view f0(p, static_cast<size_t>(c1 - p));
        const char* rest = c1 + 1;
        const EventHandler* handler = kEventHandlers.find(f0);
        if (handler) {
            (*handler)(bm, break_count, rest, end, time_offset);
        } else {
            ++bm.stats.storyboard_lines;
        }
    }

    struct EventsSectionLine {
        Beatmap* beatmap;
        size_t* break_count;
        int time_offset;
        void operator()(nonstd::string_view line) {
            parse_event_line(*beatmap, *break_count, line.data(), line.size(), time_offset);
        }
    };

#if FOSU_SIMD
    inline const char* parse_events_section_simd(Beatmap& bm, size_t& break_count, const char* p,
                                                 const char* file_end, int time_offset,
                                                 Client::Value client) {
        // Fused loop: skip indented storyboard commands on their first byte and find line endings
        // with two 32-byte vector loads.
        u32 storyboard_lines = 0;
        while (p < file_end) {
            const char c = *p;
            if (c == '\r' || c == '\n') {
                ++p;
                continue;
            }
            const Bytes32 a = load32(p);
            const Bytes32 b = load32(p + 32);
            const u64 endings = line_end_mask32(a) | (static_cast<u64>(line_end_mask32(b)) << 32);
            const char* line = p;
            const char* next_line;
            const char* line_end;
            if (endings) {
                line_end = p + trailing_zeros(endings);
            } else {
                line_end = find_line_end(p + 64, file_end);
            }
            next_line = after_line_ending(line_end, file_end);
            if (c == '[' && section_header_line(line, line_end, client)) {
                break;
            }
            p = next_line;

            if (ignored_line(line, line_end)) {
                continue;
            }
            if (c == ' ' || c == '_') { // indented storyboard command
                ++storyboard_lines;
                continue;
            }
            parse_event_line(bm, break_count, line, static_cast<size_t>(line_end - line), time_offset);
        }
        bm.stats.storyboard_lines += storyboard_lines;
        return p;
    }
#endif

    inline const char* parse_events_section_scalar(Beatmap& bm, size_t& break_count, const char* p,
                                                   const char* file_end, const ParseContext& context) {
        EventsSectionLine parse_line;
        parse_line.beatmap = &bm;
        parse_line.break_count = &break_count;
        parse_line.time_offset = context.format.time_offset;
        return for_each_section_line(p, file_end, parse_line, context.client);
    }

    inline const char* parse_events_section(Beatmap& bm, size_t& break_count, const char* p,
                                            const char* file_end, const ParseContext& context) {
#if FOSU_SIMD
        return parse_events_section_simd(bm, break_count, p, file_end, context.format.time_offset,
                                         context.client);
#else
        return parse_events_section_scalar(bm, break_count, p, file_end, context);
#endif
    }
}} // namespace fosu::internal

#endif
