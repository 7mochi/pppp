#ifndef FOSU_ENGINE_PRIMITIVES_VECTOR_OPS_H
#define FOSU_ENGINE_PRIMITIVES_VECTOR_OPS_H

#include <fosu/types.h>

#if (defined(FOSU_COMPILE_AVX2) || (!defined(_MSC_VER) && defined(__AVX2__) && defined(__BMI__))) &&         \
    !defined(FOSU_DISABLE_SIMD)
#define FOSU_SIMD_X86 1
#include <immintrin.h>
#else
#define FOSU_SIMD_X86 0
#endif
#if defined(__aarch64__) && defined(__ARM_NEON) && !defined(FOSU_DISABLE_SIMD)
#define FOSU_SIMD_NEON 1
#include <arm_neon.h>
#else
#define FOSU_SIMD_NEON 0
#endif
#define FOSU_SIMD (FOSU_SIMD_X86 || FOSU_SIMD_NEON)

// Shared home for the SIMD alignment macro (upstream spells `alignas`); used by
// the hit-object and slider mask tables.
#if FOSU_SIMD
#if defined(_MSC_VER)
#define FOSU_SIMD_ALIGN(n) __declspec(align(n))
#else
#define FOSU_SIMD_ALIGN(n) __attribute__((aligned(n)))
#endif
#endif

namespace fosu { namespace internal {
#if FOSU_SIMD_X86
    // `if constexpr (sizeof(T) == 4)` becomes a bool partial specialisation.
    template <bool Is32>
    struct trailing_zeros_impl;

    template <>
    struct trailing_zeros_impl<true> {
        template <typename T>
        static T apply(T v) {
            return static_cast<T>(_tzcnt_u32(static_cast<unsigned int>(v)));
        }
    };

    template <>
    struct trailing_zeros_impl<false> {
        template <typename T>
        static T apply(T v) {
            return static_cast<T>(_tzcnt_u64(static_cast<u64>(v)));
        }
    };
#endif
#if FOSU_SIMD
    // Keep the native BMI operation on x86: GCC can add zero-input branches
    // around std::countr_zero even when TZCNT already defines that result.
    template <typename T>
    inline T trailing_zeros(T v) {
#if FOSU_SIMD_X86
        return trailing_zeros_impl<sizeof(T) == 4>::apply(v);
#else
        // std::countr_zero is C++20; __builtin_ctzll preserves it, including countr_zero(0).
        return v == 0 ? static_cast<T>(sizeof(T) * 8) : static_cast<T>(__builtin_ctzll(static_cast<u64>(v)));
#endif
    }
#endif
#if FOSU_SIMD_X86
    typedef __m256i ByteVector;
    typedef __m256i Bytes32;
    inline Bytes32 load32(const char* p) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p)); }
    template <u8 Value>
    inline ByteVector broadcast_byte() {
#if defined(_MSC_VER)
        return _mm256_set1_epi8(static_cast<char>(Value));
#else
        // A memory broadcast prevents GCC rebuilding constants through a general
        // register inside loops containing calls. Keep that detail out of parsers.
        static const u8 value = Value;
        ByteVector result;
        __asm__("vpbroadcastb %1, %0" : "=x"(result) : "m"(value));
        return result;
#endif
    }
    // Port-only: the scalar find_byte takes its delimiter as a runtime argument
    // (upstream templates it), so the vector path broadcasts that value. Same
    // instruction as the template; the compiler cannot fold it into the mask.
    inline ByteVector broadcast_byte(u8 value) { return _mm256_set1_epi8(static_cast<char>(value)); }
    inline u32 equal_mask32(Bytes32 v, ByteVector c) {
        return static_cast<u32>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(v, c)));
    }
    inline u32 line_end_mask32(Bytes32 v) {
        const __m256i cr = _mm256_cmpeq_epi8(v, broadcast_byte<'\r'>());
        const __m256i lf = _mm256_cmpeq_epi8(v, broadcast_byte<'\n'>());
        return static_cast<u32>(_mm256_movemask_epi8(_mm256_or_si256(cr, lf)));
    }
    // AVX2 has no unsigned byte comparison: adding 80 maps ASCII digits to
    // [-128, -119], below every other byte under a signed comparison.
    inline u32 nondigit_mask32(Bytes32 v) {
        return static_cast<u32>(_mm256_movemask_epi8(
            _mm256_cmpgt_epi8(_mm256_add_epi8(v, broadcast_byte<80>()), broadcast_byte<137>())));
    }
#elif FOSU_SIMD_NEON
    typedef uint8x16_t ByteVector;
    typedef uint8x16x2_t Bytes32;
    inline Bytes32 load32(const char* p) {
        Bytes32 result;
        result.val[0] = vld1q_u8(reinterpret_cast<const u8*>(p));
        result.val[1] = vld1q_u8(reinterpret_cast<const u8*>(p + 16));
        return result;
    }
    template <u8 Value>
    inline ByteVector broadcast_byte() {
        return vdupq_n_u8(Value);
    }
    // Port-only runtime counterpart to the template above (see the x86 branch).
    inline ByteVector broadcast_byte(u8 value) { return vdupq_n_u8(value); }
    // Comparisons have all-zero or all-one lanes. Weight each true lane by its bit
    // and pairwise-add until each byte holds eight lanes' bits. Both halves share
    // one reduction, so a 32-byte mask costs one extraction, as with AVX2.
    inline u32 byte_mask32(uint8x16_t low, uint8x16_t high) {
        const u8 weights[16] = {1, 2, 4, 8, 16, 32, 64, 128, 1, 2, 4, 8, 16, 32, 64, 128};
        const uint8x16_t w = vld1q_u8(weights);
        const uint8x16_t pairs = vpaddq_u8(vandq_u8(low, w), vandq_u8(high, w));
        const uint8x16_t fours = vpaddq_u8(pairs, pairs);
        const uint8x16_t eights = vpaddq_u8(fours, fours);
        return vgetq_lane_u32(vreinterpretq_u32_u8(eights), 0);
    }
    inline u32 equal_mask32(Bytes32 v, ByteVector c) {
        return byte_mask32(vceqq_u8(v.val[0], c), vceqq_u8(v.val[1], c));
    }
    inline u32 line_end_mask32(Bytes32 v) {
        const ByteVector cr = broadcast_byte<'\r'>();
        const ByteVector lf = broadcast_byte<'\n'>();
        return byte_mask32(vorrq_u8(vceqq_u8(v.val[0], cr), vceqq_u8(v.val[0], lf)),
                           vorrq_u8(vceqq_u8(v.val[1], cr), vceqq_u8(v.val[1], lf)));
    }
    // Four bits per byte, all set where `matches` is. One narrowing shift is
    // cheaper than byte_mask32's pairwise sums, for code that needs only
    // positions: divide a mask's bit index by 4.
    inline u64 nibble_mask16(uint8x16_t matches) {
        return vget_lane_u64(vreinterpret_u64_u8(vshrn_n_u16(vreinterpretq_u16_u8(matches), 4)), 0);
    }
    inline uint8x16_t nondigit_bytes16(uint8x16_t v) {
        return vcgtq_u8(vsubq_u8(v, vdupq_n_u8('0')), vdupq_n_u8(9));
    }
    inline u32 nondigit_mask32(Bytes32 v) {
        return byte_mask32(nondigit_bytes16(v.val[0]), nondigit_bytes16(v.val[1]));
    }
#endif
}} // namespace fosu::internal

#endif
