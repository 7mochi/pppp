#ifndef FOSU_ENGINE_PARSING_LINES_H
#define FOSU_ENGINE_PARSING_LINES_H

#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/nonstd_config.h>
#include <fosu/parse_options.h>

namespace fosu { namespace internal {
    inline bool ignored_line(const char* p, const char* end) {
        p = skip_numeric_space(p, end);
        return p == end || (end - p >= 2 && p[0] == '/' && p[1] == '/');
    }

    inline nonstd::string_view trim(const char* p, const char* end) {
        while (p < end && (*p == ' ' || *p == '\t')) {
            ++p;
        }
        while (end > p && (end[-1] == ' ' || end[-1] == '\t')) {
            --end;
        }
        return nonstd::string_view(p, static_cast<size_t>(end - p));
    }

    struct Line {
        nonstd::string_view text;
        const char* next;
    };

    inline const char* after_line_ending(const char* line_end, const char* end) {
        if (line_end == end) {
            return end;
        }
        const char* next = line_end + 1;
        return *line_end == '\r' && next < end && *next == '\n' ? next + 1 : next;
    }

    inline Line read_line(const char* p, const char* end) {
        const char* line_end = find_line_end(p, end);
        Line line;
        line.text = nonstd::string_view(p, static_cast<size_t>(line_end - p));
        line.next = after_line_ending(line_end, end);
        return line;
    }

    // osu!stable reads any line starting with '[' as a section header, even one
    // that names no section. Lazer also requires the closing ']', so a stray '['
    // at the start of a malformed record does not end its section.
    inline bool section_header_line(const char* p, const char* end, Client::Value client) {
        if (p == end || *p != '[') {
            return false;
        }
        if (client == Client::Stable) {
            return true;
        }
        while (end > p && (end[-1] == ' ' || end[-1] == '\t')) {
            --end;
        }
        return end - p >= 2 && end[-1] == ']';
    }

    // Consume a section body, leaving its next header for the document parser. `parse_line` is a
    // functor taking a nonstd::string_view (the C++98 stand-in for the upstream lambdas).
    template <typename ParseLine>
    inline const char* for_each_section_line(const char* p, const char* end, ParseLine& parse_line,
                                             Client::Value client) {
        while (p < end) {
            if (*p == '\r' || *p == '\n') {
                ++p;
                continue;
            }
            const Line line = read_line(p, end);
            if (section_header_line(p, p + line.text.size(), client)) {
                break;
            }
            if (!ignored_line(p, p + line.text.size())) {
                parse_line(line.text);
            }
            p = line.next;
        }
        return p;
    }

    inline const char* skip_section(const char* p, const char* end, Client::Value client) {
        const char* header = find_byte('[', p, end);
        // A bracket inside a value is not a section, nor for lazer is an incomplete header.
        while (header < end) {
            if (header == p || header[-1] == '\n' || header[-1] == '\r') {
                const Line line = read_line(header, end);
                if (section_header_line(header, header + line.text.size(), client)) {
                    return header;
                }
            }
            header = find_byte('[', header + 1, end);
        }
        return end;
    }
}} // namespace fosu::internal

#endif
