#ifndef FOSU_ENGINE_PRIMITIVES_BYTE_SCAN_H
#define FOSU_ENGINE_PRIMITIVES_BYTE_SCAN_H

#include <cstring>

namespace fosu { namespace internal {

    // First occurrence of `delimiter` in [p, end), or end when absent.
    inline const char* find_byte(char delimiter, const char* p, const char* end) {
        const char* match = static_cast<const char*>(std::memchr(p, delimiter, static_cast<size_t>(end - p)));
        return match ? match : end;
    }

    // First CR or LF in [p, end), or end when absent: a lone CR ends a line as it does in osu!.
    inline const char* find_line_end(const char* p, const char* end) {
        if (p >= end) {
            return end;
        }
        // Bound each search so CR-only files do not rescan the whole suffix per line.
        while (p < end) {
            const size_t remaining = static_cast<size_t>(end - p);
            const size_t count = remaining < 128 ? remaining : 128;
            const char* lf = static_cast<const char*>(std::memchr(p, '\n', count));
            const char* cr =
                static_cast<const char*>(std::memchr(p, '\r', lf ? static_cast<size_t>(lf - p) : count));
            if (cr) {
                return cr;
            }
            if (lf) {
                return lf;
            }
            p += count;
        }
        return p;
    }

}} // namespace fosu::internal

#endif
