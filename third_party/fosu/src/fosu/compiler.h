#ifndef FOSU_COMPILER_H
#define FOSU_COMPILER_H

// Strict ISO C++98 port: the only compiler-specific things are the 64-bit integer types and a
// few optional attributes.

#if defined(_MSC_VER) && _MSC_VER < 1310
typedef __int64 fosu_int64;
typedef unsigned __int64 fosu_uint64;
#elif defined(__GNUC__)
__extension__ typedef long long fosu_int64;
__extension__ typedef unsigned long long fosu_uint64;
#else
typedef long long fosu_int64;
typedef unsigned long long fosu_uint64;
#endif

typedef int fosu_int32;
typedef unsigned int fosu_uint32;
typedef unsigned char fosu_uint8;
typedef unsigned short fosu_uint16;

#define FOSU_STATIC_ASSERT(name, cond) typedef char fosu_static_assert_##name[(cond) ? 1 : -1]

FOSU_STATIC_ASSERT(int64_is_64_bits, sizeof(fosu_int64) == 8);
FOSU_STATIC_ASSERT(int32_is_32_bits, sizeof(fosu_int32) == 4);
FOSU_STATIC_ASSERT(uint16_is_16_bits, sizeof(fosu_uint16) == 2);

#if defined(_MSC_VER)
#define FOSU_ALWAYS_INLINE __forceinline
#define FOSU_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define FOSU_ALWAYS_INLINE __attribute__((always_inline)) inline
#define FOSU_NOINLINE __attribute__((noinline))
#else
#define FOSU_ALWAYS_INLINE inline
#define FOSU_NOINLINE
#endif

namespace fosu {

    const fosu_int32 kInt32Max = 2147483647;
    const fosu_int32 kInt32Min = -2147483647 - 1;
    const fosu_uint32 kUint32Max = 4294967295u;

    inline fosu_uint64 uint64_max() { return ~static_cast<fosu_uint64>(0); }
    inline fosu_int64 int64_max() { return static_cast<fosu_int64>(uint64_max() >> 1); }
    inline fosu_int64 int64_min() { return -int64_max() - 1; }

} // namespace fosu

#endif
