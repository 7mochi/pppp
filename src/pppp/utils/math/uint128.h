#ifndef PPPP_UTILS_MATH_UINT128_H
#define PPPP_UTILS_MATH_UINT128_H

#include "pppp/config.h"

namespace pppp { namespace utils { namespace math {
    const pppp_uint64 LOW32 = 0xFFFFFFFFu;

    /// A 128-bit unsigned integer held as two 64-bit halves, with the narrow set of operations the
    /// legacy score simulation needs. It exists because that simulation reproduces the scorebase's
    /// decimal arithmetic exactly, and the intermediate products overflow what C++98 guarantees for
    /// the widest built-in integer.
    struct Uint128 {
        /// The upper 64 bits.
        pppp_uint64 hi;
        /// The lower 64 bits.
        pppp_uint64 lo;
    };

    /// Widens a 64-bit value to a 128-bit one with a zero upper half.
    /// @param value The value to widen.
    /// @returns `value` as a `Uint128`.
    Uint128 u128_from(pppp_uint64 value);

    /// Tests whether a 128-bit value is odd.
    /// @param a The value to test.
    /// @returns Whether the lowest bit is set.
    bool u128_is_odd(Uint128 a);

    /// Three-way comparison, by the upper half first.
    /// @param a The left value.
    /// @param b The right value.
    /// @returns A negative number, zero or a positive number.
    int u128_cmp(Uint128 a, Uint128 b);

    /// Adds two 128-bit values, wrapping modulo 2^128.
    /// @param a The left value.
    /// @param b The right value.
    /// @returns The sum.
    Uint128 u128_add(Uint128 a, Uint128 b);

    /// Subtracts two 128-bit values, wrapping modulo 2^128.
    /// @param a The left value.
    /// @param b The right value.
    /// @returns The difference.
    Uint128 u128_sub(Uint128 a, Uint128 b);

    /// Multiplies two 64-bit values to their full 128-bit product.
    /// @param a The left factor.
    /// @param b The right factor.
    /// @returns The product.
    Uint128 u128_mul64(pppp_uint64 a, pppp_uint64 b);

    /// Multiplies a 128-bit value by a 64-bit one, keeping the low 128 bits.
    /// @param a The 128-bit factor.
    /// @param b The 64-bit factor.
    /// @returns The product.
    Uint128 u128_mul(Uint128 a, pppp_uint64 b);

    /// Shifts a 128-bit value left by one bit.
    /// @param a The value to shift.
    /// @returns The shifted value.
    Uint128 u128_shl1(Uint128 a);

    /// Divides a 128-bit value by another by restoring long division, one bit at a time.
    /// @param n The dividend.
    /// @param d The divisor.
    /// @param quotient Set to the quotient.
    /// @param remainder Set to the remainder.
    void u128_divmod(Uint128 n, Uint128 d, Uint128* quotient, Uint128* remainder);
}}} // namespace pppp::utils::math

// The score simulation computes with this type throughout, so it is re-exported under its short name.
namespace pppp { namespace utils {
    typedef math::Uint128 Uint128;
}} // namespace pppp::utils

#endif
