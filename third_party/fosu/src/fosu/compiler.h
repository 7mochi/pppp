#ifndef FOSU_COMPILER_H
#define FOSU_COMPILER_H

#include <fosu/types.h>

#if defined(_MSC_VER)
#define FOSU_ALWAYS_INLINE __forceinline
#define FOSU_NOINLINE __declspec(noinline)
#define FOSU_UNREACHABLE() __assume(0)
#else
#define FOSU_ALWAYS_INLINE __attribute__((always_inline)) inline
#define FOSU_NOINLINE __attribute__((noinline))
#define FOSU_UNREACHABLE() __builtin_unreachable()
#endif

// osu! rounds every floating-point operation separately. Between these
// markers, compilers may not contract a multiply and an add into one fused
// multiply-add, which rounds once. GCC's pragma is a function attribute, and
// GCC does not inline across differing attributes: keep the post-processing
// code that calls these functions inside the markers too.
#if defined(__clang__)
#define FOSU_FP_CONTRACT_OFF_BEGIN _Pragma("float_control(push)") _Pragma("clang fp contract(off)")
#define FOSU_FP_CONTRACT_OFF_END _Pragma("float_control(pop)")
#elif defined(__GNUC__)
#define FOSU_FP_CONTRACT_OFF_BEGIN _Pragma("GCC push_options") _Pragma("GCC optimize(\"fp-contract=off\")")
#define FOSU_FP_CONTRACT_OFF_END _Pragma("GCC pop_options")
#else
#define FOSU_FP_CONTRACT_OFF_BEGIN
#define FOSU_FP_CONTRACT_OFF_END
#endif

#if defined(_WIN32)
#define FOSU_EXPORT __declspec(dllexport)
#define FOSU_IMPORT __declspec(dllimport)
#else
#define FOSU_EXPORT __attribute__((visibility("default")))
#define FOSU_IMPORT
#endif

// port-only block: the master tree keeps its integer helpers in types.h; the port keeps the
// static-assert macro and the int32 min/max constants here because C++98 has no static_assert and
// the scalar paths need the bounds.
#define FOSU_STATIC_ASSERT(name, cond) typedef char fosu_static_assert_##name[(cond) ? 1 : -1]

namespace fosu {
    const i32 kInt32Max = 2147483647;
    const i32 kInt32Min = -2147483647 - 1;
    const u32 kUint32Max = 4294967295u;

    inline u64 uint64_max() { return ~static_cast<u64>(0); }
    inline i64 int64_max() { return static_cast<i64>(uint64_max() >> 1); }
    inline i64 int64_min() { return -int64_max() - 1; }
} // namespace fosu

#endif
