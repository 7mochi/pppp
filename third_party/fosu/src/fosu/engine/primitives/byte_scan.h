#ifndef FOSU_ENGINE_PRIMITIVES_BYTE_SCAN_H
#define FOSU_ENGINE_PRIMITIVES_BYTE_SCAN_H

#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/types.h>

#include <cstring>

namespace fosu { namespace internal {
#if FOSU_SIMD_NEON
    inline u32 first_line_end16(uint8x16_t bytes) {
        const u8 positions[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const uint8x16_t cr = vceqq_u8(bytes, broadcast_byte<'\r'>());
        const uint8x16_t lf = vceqq_u8(bytes, broadcast_byte<'\n'>());
        const uint8x16_t positions_or_end = vbslq_u8(vorrq_u8(cr, lf), vld1q_u8(positions), vdupq_n_u8(16));
        return vminvq_u8(positions_or_end);
    }

    // A line scan only needs the first ending, not the complete byte mask.
    inline u32 first_line_end32(Bytes32 bytes) {
        const u32 first = first_line_end16(bytes.val[0]);
        if (first < 16) {
            return first;
        }
        return 16 + first_line_end16(bytes.val[1]);
    }
#elif FOSU_SIMD
    // Offset of the first line ending in 32 bytes, or 32 without one.
    inline u32 first_line_end32(Bytes32 bytes) {
        const u32 endings = line_end_mask32(bytes);
        return endings ? static_cast<u32>(trailing_zeros(endings)) : 32;
    }
#endif

    // Input belongs to the parser's padded buffer. Matches are bounded by end, even when the
    // vector load includes bytes from the following field or line. Returns end when the
    // delimiter is absent. The port keeps a runtime delimiter (upstream templates it); the SIMD
    // branch broadcasts that value instead.
    inline const char* find_byte(char delimiter, const char* p, const char* end) {
#if FOSU_SIMD
        const ByteVector delimiter_vector = broadcast_byte(static_cast<u8>(delimiter));
        while (p < end) {
            const u32 mask = equal_mask32(load32(p), delimiter_vector);
            const size_t remaining = static_cast<size_t>(end - p);
            if (mask) {
                const u32 offset = trailing_zeros(mask);
                return offset < remaining ? p + offset : end;
            }
            if (remaining <= 32) {
                return end;
            }
            p += 32;
        }
        return end;
#else
        const char* match = static_cast<const char*>(std::memchr(p, delimiter, static_cast<size_t>(end - p)));
        return match ? match : end;
#endif
    }

    // First CR or LF in [p, end), or end when absent: a lone CR ends a line as it does in osu!.
    inline const char* find_line_end(const char* p, const char* end) {
        if (p >= end) {
            return end;
        }
#if FOSU_SIMD_NEON
        while (p < end) {
            const u32 offset = first_line_end32(load32(p));
            const size_t remaining = static_cast<size_t>(end - p);
            if (offset < 32) {
                return offset < remaining ? p + offset : end;
            }
            if (remaining <= 32) {
                return end;
            }
            p += 32;
        }
#elif FOSU_SIMD
        while (p < end) {
            const Bytes32 chars = load32(p);
            const u32 mask = line_end_mask32(chars);
            const size_t remaining = static_cast<size_t>(end - p);
            if (mask) {
                const u32 offset = trailing_zeros(mask);
                return offset < remaining ? p + offset : end;
            }
            if (remaining <= 32) {
                return end;
            }
            p += 32;
        }
#else
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
#endif
        return p;
    }
}} // namespace fosu::internal

#endif
