#ifndef FOSU_ENGINE_HIT_OBJECTS_SAMPLES_H
#define FOSU_ENGINE_HIT_OBJECTS_SAMPLES_H

#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/span.h>

namespace fosu { namespace internal {

    // The common editor spelling "d:d:d:d:" (upstream tests the 8 bytes at once; the padding
    // contract makes reading them always safe).
    inline bool short_sample(const char* p) {
        return p[1] == ':' && p[3] == ':' && p[5] == ':' && p[7] == ':' && is_digit(p[0]) && is_digit(p[2]) &&
               is_digit(p[4]) && is_digit(p[6]);
    }

    // Only the fields read by the official legacy decoder affect acceptance.
    // Unknown trailing sample/edge columns are retained as raw text by callers.
    inline bool valid_sample(StringView sample, bool banks_only = false) {
        if (sample.empty()) {
            return true;
        }
        if (sample.size() == 3 && sample[1] == ':' && is_digit(sample[0]) && is_digit(sample[2])) {
            return true;
        }
        // The common editor spelling avoids four separate integer conversions.
        if (sample.size() >= 8 && short_sample(sample.data())) {
            return true;
        }
        const char* p = sample.data();
        const char* end = p + sample.size();
        for (int i = 0; i < (banks_only ? 2 : 4); ++i) {
            fosu_int64 value;
            const char* q = parse_osu_int(p, end, value);
            if (q == p || (q < end && *q != ':')) {
                return false;
            }
            if (q == end) {
                return i >= 1;
            }
            p = q + 1;
        }
        return true; // The fifth field is an arbitrary filename.
    }

    inline bool valid_edge_sets(StringView sets, fosu_int32 slides) {
        if (sets.empty()) {
            return true;
        }
        if (sets.size() == 7) {
            // "d:d|d:d"
            const char* s = sets.data();
            if (s[1] == ':' && s[3] == '|' && s[5] == ':' && is_digit(s[0]) && is_digit(s[2]) &&
                is_digit(s[4]) && is_digit(s[6])) {
                return true;
            }
        }
        const char* p = sets.data();
        const char* end = p + sets.size();
        const int nodes = (slides > 0 ? slides : 1) + 1;
        for (int i = 0; i < nodes; ++i) {
            // Editor bank pairs are single digits. Validate those directly;
            // additional sample fields and unusual integers use the same fallback.
            if (end - p >= 3 && p[1] == ':' && is_digit(p[0]) && is_digit(p[2])) {
                if (end - p == 3) {
                    return true;
                }
                if (p[3] == '|') {
                    p += 4;
                    continue;
                }
            }
            const char* next = find_byte('|', p, end);
            if (!valid_sample(StringView(p, static_cast<size_t>(next - p)))) {
                return false;
            }
            if (next == end) {
                break;
            }
            p = next + 1;
        }
        return true;
    }

    inline bool parse_hit_sample(const char* p, const char* end, StringView& out, bool banks_only = false) {
        if (end - p == 8 && short_sample(p)) {
            out = StringView(p, 8);
            return true;
        }
        const char* sample_end = find_byte(',', p, end);
        const StringView sample(p, static_cast<size_t>(sample_end - p));
        if (!valid_sample(sample, banks_only)) {
            return false;
        }
        out = sample;
        return true;
    }

}} // namespace fosu::internal

#endif
