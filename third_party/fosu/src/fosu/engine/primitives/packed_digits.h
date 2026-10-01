#ifndef FOSU_ENGINE_PRIMITIVES_PACKED_DIGITS_H
#define FOSU_ENGINE_PRIMITIVES_PACKED_DIGITS_H

// Digit-run detection and conversion. Upstream does this branchlessly on 8-byte words (SWAR);
// the port keeps the same functions and contract (the buffer is followed by kBufferPadding
// readable zero bytes, so looking at 8 bytes past `p` is always allowed) with plain loops.

#include <fosu/compiler.h>

namespace fosu { namespace internal {

    inline bool is_ascii_digit(char c) { return static_cast<unsigned char>(c - '0') <= 9; }

    // Number of leading ASCII digits in the next 8 bytes (0..8).
    inline fosu_uint32 digit_run8(const char* p) {
        fosu_uint32 run = 0;
        while (run < 8 && is_ascii_digit(p[run])) {
            run++;
        }
        return run;
    }

    // Convert `len` (1..4) leading digits at `p`.
    inline fosu_uint32 swar_parse_u32(const char* p, fosu_uint32 len) {
        fosu_uint32 value = 0;
        for (fosu_uint32 i = 0; i < len; i++) {
            value = value * 10 + static_cast<fosu_uint32>(p[i] - '0');
        }
        return value;
    }

    // Convert `len` (1..8) leading digits at `p`.
    inline fosu_uint64 swar_parse_u64(const char* p, fosu_uint32 len) {
        fosu_uint64 value = 0;
        for (fosu_uint32 i = 0; i < len; i++) {
            value = value * 10 + static_cast<fosu_uint64>(p[i] - '0');
        }
        return value;
    }

    // swar_parse_u64 that tolerates any `len` (the result is meaningless outside 1..8).
    inline fosu_uint64 swar_parse_u64_safe(const char* p, fosu_uint32 len) {
        return swar_parse_u64(p, len > 8 ? 8 : len);
    }

}} // namespace fosu::internal

#endif
