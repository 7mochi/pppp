#ifndef FOSU_ENGINE_SECTIONS_EVENTS_H
#define FOSU_ENGINE_SECTIONS_EVENTS_H

#include <algorithm>

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/parsing/string_lookup.h>
#include <fosu/engine/primitives/byte_scan.h>

namespace fosu { namespace internal {

    inline StringView strip_quotes(StringView v) {
        if (v.size() >= 2 && v.front() == '"' && v.back() == '"') {
            return v.substr(1, v.size() - 2);
        }
        return v;
    }

    inline Optional<StringView> parse_event_filename(const char* rest, const char* end) {
        const char* comma = find_byte(',', rest, end);
        if (comma == end) {
            return Optional<StringView>();
        }
        const char* filename = comma + 1;
        const char* next = find_byte(',', filename, end);
        return Optional<StringView>(strip_quotes(trim(filename, next)));
    }

    inline void parse_background_event(Beatmap& bm, size_t&, const char* rest, const char* end, int) {
        const Optional<StringView> filename = parse_event_filename(rest, end);
        if (filename.has_value()) {
            bm.background = *filename;
        }
    }

    inline void parse_video_event(Beatmap& bm, size_t&, const char* rest, const char* end, int) {
        const Optional<StringView> filename = parse_event_filename(rest, end);
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
        const char* r = parse_osu_double(q + 1, end, stop);
        if (r == q + 1 || r != end) {
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
        const StringView f0(p, static_cast<size_t>(c1 - p));
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
        void operator()(StringView line) {
            parse_event_line(*beatmap, *break_count, line.data(), line.size(), time_offset);
        }
    };

    inline const char* parse_events_section(Beatmap& bm, size_t& break_count, const char* p,
                                            const char* file_end, int time_offset = 0) {
        EventsSectionLine parse_line;
        parse_line.beatmap = &bm;
        parse_line.break_count = &break_count;
        parse_line.time_offset = time_offset;
        return for_each_section_line(p, file_end, parse_line);
    }

}} // namespace fosu::internal

#endif
