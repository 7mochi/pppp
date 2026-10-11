#ifndef FOSU_ENGINE_HIT_OBJECTS_COMMON_FIELDS_H
#define FOSU_ENGINE_HIT_OBJECTS_COMMON_FIELDS_H

// Parsers for the hitobject line prefix "x,y,time,type,hitSound".
//
// The AVX2 path classifies delimiters with one vector compare, derives a
// (len_x, len_y, len_time, len_type) signature, and uses it to index
// precomputed permute+shuffle masks that normalize every digit into a
// fixed position. A single maddubs/madd chain converts the whole prefix at
// once. It also uses:
//   - parallel delimiter extraction (blsr chain + independent tzcnts)
//     instead of a serialized tzcnt/shift chain
//   - the table index computed directly from delimiter positions:
//     ((x0*3+y0)*10+t0)*3+ty0  ==  60*p0 + 27*p1 + 2*p2 + p3 - 158
//   - permute and shuffle masks fused into one 64-byte (one cache line)
//     table entry, generated at compile time
//   - structural validation with scalar fallback instead of assuming
//     well-formed input
//   - 1-2 digit hitSound support (osu! hitsound bitflags go up to 15)
//
// Callers must guarantee kBufferPadding readable bytes past the end of
// the buffer (see io.h); all paths rely on it to load past short lines
// and speculate past field boundaries safely.

#include <fosu/compiler.h>
#include <fosu/engine/primitives/digit_groups.h>
#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/types.h>

#if FOSU_SIMD_X86
#include <cstdlib>
#endif

namespace fosu { namespace internal {
#if !FOSU_SIMD
    struct HitObjectParseConstants {}; // the scalar path has no vector constants
#endif

    const u32 kNPrefixVariants = 3 * 3 * 10 * 3;

#if FOSU_SIMD_X86

    // One entry per (len_x, len_y, len_time, len_type, len_hit_sound) combination.
    // `perm` feeds vpermd to move each field's dwords into the lane that needs
    // them; `shuf` then places digits at fixed offsets (0x80 lanes produce zero):
    //   bytes  0-3   x   right-aligned  -> dword 0 after madd
    //   bytes  4-7   y                  -> dword 1
    //   bytes  8-11  type               -> dword 2
    //   bytes 12-15  hitSound digits available in the low lane
    //   bytes 16-19  remaining hitSound digits, summed with dword 3 after madd
    //   bytes 20-31  time right-aligned -> dwords 5,6,7 = top2/mid4/low4 digits
    struct FOSU_SIMD_ALIGN(64) LaneMasks {
        i32 perm[8];
        i8 shuf[32];
    };
    FOSU_STATIC_ASSERT(lane_masks_size, sizeof(LaneMasks) == 64);

    // The upstream `find_or_add_word` lambda, lifted to a free function over the
    // entry it mutates and its per-lane occupancy counts.
    inline i32 lane_mask_find_or_add_word(LaneMasks& lm, i32* used, i32 word, i32 lane) {
        for (i32 i = 0; i < used[lane]; ++i) {
            if (lm.perm[lane * 4 + i] == word) {
                return i;
            }
        }
        if (used[lane] == 4) {
            return -1;
        }
        const i32 slot = used[lane]++;
        lm.perm[lane * 4 + slot] = word;
        return slot;
    }

    // Upstream builds this at compile time with `consteval`; the port fills the table once at
    // start-up in the LaneMaskTable constructor below.
    inline void fill_lane_masks(LaneMasks* out) {
        for (i32 lx = 1; lx <= 3; ++lx) {
            for (i32 ly = 1; ly <= 3; ++ly) {
                for (i32 lt = 1; lt <= 10; ++lt) {
                    for (i32 lty = 1; lty <= 3; ++lty) {
                        for (i32 lhs = 1; lhs <= 2; ++lhs) {
                            const i32 p0 = lx;
                            const i32 p1 = p0 + 1 + ly;
                            const i32 p2 = p1 + 1 + lt;
                            const i32 p3 = p2 + 1 + lty;
                            const i32 index =
                                ((((lx - 1) * 3 + (ly - 1)) * 10 + (lt - 1)) * 3 + (lty - 1)) * 2 + lhs - 1;

                            i32 src[32];
                            for (i32 i = 0; i < 32; ++i) {
                                src[i] = -1;
                            }
                            for (i32 i = 0; i < lx; ++i) {
                                src[4 - lx + i] = i;
                            }
                            for (i32 i = 0; i < ly; ++i) {
                                src[8 - ly + i] = p0 + 1 + i;
                            }
                            for (i32 i = 0; i < lty; ++i) {
                                src[12 - lty + i] = p2 + 1 + i;
                            }
                            for (i32 i = 0; i < lt; ++i) {
                                src[32 - lt + i] = p1 + 1 + i;
                            }

                            LaneMasks& lm = out[index];
                            for (i32 i = 0; i < 8; ++i) {
                                lm.perm[i] = -1;
                            }
                            i32 used[2] = {0, 0};
                            // Each 128-bit lane can gather four source dwords before the byte
                            // shuffle.
                            for (i32 b = 0; b < 32; ++b) {
                                if (src[b] >= 0 &&
                                    lane_mask_find_or_add_word(lm, used, src[b] / 4, b / 16) < 0) {
                                    std::abort();
                                }
                            }

                            // Put hitSound in the low lane where possible; use spare high-lane
                            // space otherwise. Preserve each digit's decimal weight in either
                            // lane.
                            for (i32 i = 0; i < lhs; ++i) {
                                const i32 source = p3 + 1 + i;
                                if (lane_mask_find_or_add_word(lm, used, source / 4, 0) >= 0) {
                                    src[16 - lhs + i] = source;
                                } else {
                                    if (lane_mask_find_or_add_word(lm, used, source / 4, 1) < 0) {
                                        std::abort();
                                    }
                                    src[20 - lhs + i] = source;
                                }
                            }

                            for (i32 b = 0; b < 32; ++b) {
                                if (src[b] < 0) {
                                    lm.shuf[b] = static_cast<i8>(0x80);
                                    continue;
                                }
                                const i32 dw = src[b] / 4;
                                const i32 off = src[b] % 4;
                                const i32 slot = lane_mask_find_or_add_word(lm, used, dw, b / 16);
                                lm.shuf[b] = static_cast<i8>(slot * 4 + off);
                            }
                            for (i32 i = 0; i < 8; ++i) {
                                if (lm.perm[i] < 0) {
                                    lm.perm[i] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    struct LaneMaskTable {
        LaneMasks entries[kNPrefixVariants * 2];
        LaneMaskTable() { fill_lane_masks(entries); }
    };
    static const LaneMaskTable kLaneMaskTable;
    static const LaneMasks (&kLaneMasks)[kNPrefixVariants * 2] = kLaneMaskTable.entries;

    // Vector constants for one [HitObjects] section, built once and passed by
    // reference so callees use them as memory operands instead of rebuilding
    // them per call.
    struct HitObjectParseConstants {
        __m256i comma, zero;
        __m128i pair_weights, word_weights; // slider point digit weights
        HitObjectParseConstants()
            : comma(broadcast_byte<','>()),
              zero(broadcast_byte<'0'>()),
              pair_weights(_mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0)),
              word_weights(_mm_setr_epi16(100, 1, 100, 1, 0, 0, 0, 0)) {}
    };

#elif FOSU_SIMD_NEON
    struct HitObjectParseConstants {
        ByteVector comma;
        ByteVector zero;
        HitObjectParseConstants()
            : comma(broadcast_byte<','>()),
              zero(broadcast_byte<'0'>()) {}
    };

    // TBL directly addresses both 16-byte input registers. No lane permutation
    // is needed, so each prefix shape occupies 32 bytes instead of AVX2's 64.
    // The first shuffle decodes x, y, type and hitSound; the second decodes time.
    struct FOSU_SIMD_ALIGN(32) PrefixShuffle {
        u8 bytes[32];
    };

    // Upstream builds this at compile time with `consteval`; the port fills the table once at
    // start-up in the PrefixShuffleTable constructor below.
    inline void fill_prefix_shuffles(PrefixShuffle* out) {
        for (i32 x = 1; x <= 3; ++x) {
            for (i32 y = 1; y <= 3; ++y) {
                for (i32 t = 1; t <= 10; ++t) {
                    for (i32 type = 1; type <= 3; ++type) {
                        for (i32 sound = 1; sound <= 2; ++sound) {
                            PrefixShuffle& m =
                                out[((((x - 1) * 3 + y - 1) * 10 + t - 1) * 3 + type - 1) * 2 + sound - 1];
                            for (i32 i = 0; i < 32; ++i) {
                                m.bytes[i] = 255;
                            }
                            for (i32 i = 0; i < x; ++i) {
                                m.bytes[4 - x + i] = static_cast<u8>(i);
                            }
                            for (i32 i = 0; i < y; ++i) {
                                m.bytes[8 - y + i] = static_cast<u8>(x + 1 + i);
                            }
                            for (i32 i = 0; i < type; ++i) {
                                m.bytes[12 - type + i] = static_cast<u8>(x + y + t + 3 + i);
                            }
                            for (i32 i = 0; i < t; ++i) {
                                m.bytes[32 - t + i] = static_cast<u8>(x + y + 2 + i);
                            }
                            for (i32 i = 0; i < sound; ++i) {
                                m.bytes[16 - sound + i] = static_cast<u8>(x + y + t + type + 4 + i);
                            }
                        }
                    }
                }
            }
        }
    }

    struct PrefixShuffleTable {
        PrefixShuffle entries[kNPrefixVariants * 2];
        PrefixShuffleTable() { fill_prefix_shuffles(entries); }
    };
    static const PrefixShuffleTable kPrefixShuffleTable;
    static const PrefixShuffle (&kPrefixShuffles)[kNPrefixVariants * 2] = kPrefixShuffleTable.entries;
#endif
}} // namespace fosu::internal

#endif
