#include "pppp/utils/math/uint128.h"

namespace pppp { namespace utils { namespace math {
    namespace {
        Uint128 make(pppp_uint64 hi, pppp_uint64 lo) {
            Uint128 r;
            r.hi = hi;
            r.lo = lo;
            return r;
        }
    } // namespace

    Uint128 u128_from(pppp_uint64 value) { return make(0, value); }

    bool u128_is_odd(Uint128 a) { return (a.lo & 1u) != 0; }

    int u128_cmp(Uint128 a, Uint128 b) {
        if (a.hi != b.hi) {
            return a.hi < b.hi ? -1 : 1;
        }
        if (a.lo != b.lo) {
            return a.lo < b.lo ? -1 : 1;
        }
        return 0;
    }

    Uint128 u128_add(Uint128 a, Uint128 b) {
        Uint128 r;
        r.lo = a.lo + b.lo;
        r.hi = a.hi + b.hi + (r.lo < a.lo ? 1u : 0u);
        return r;
    }

    Uint128 u128_sub(Uint128 a, Uint128 b) {
        Uint128 r;
        r.lo = a.lo - b.lo;
        r.hi = a.hi - b.hi - (a.lo < b.lo ? 1u : 0u);
        return r;
    }

    Uint128 u128_mul64(pppp_uint64 a, pppp_uint64 b) {
        pppp_uint64 a0 = a & LOW32, a1 = a >> 32;
        pppp_uint64 b0 = b & LOW32, b1 = b >> 32;
        pppp_uint64 p00 = a0 * b0;
        pppp_uint64 p01 = a0 * b1;
        pppp_uint64 p10 = a1 * b0;
        pppp_uint64 p11 = a1 * b1;
        pppp_uint64 mid = (p00 >> 32) + (p01 & LOW32) + (p10 & LOW32);
        Uint128 r;
        r.lo = (mid << 32) | (p00 & LOW32);
        r.hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
        return r;
    }

    Uint128 u128_mul(Uint128 a, pppp_uint64 b) {
        Uint128 low = u128_mul64(a.lo, b);
        low.hi += a.hi * b;
        return low;
    }

    Uint128 u128_shl1(Uint128 a) {
        Uint128 r;
        r.hi = (a.hi << 1) | (a.lo >> 63);
        r.lo = a.lo << 1;
        return r;
    }

    void u128_divmod(Uint128 n, Uint128 d, Uint128* quotient, Uint128* remainder) {
        Uint128 q = make(0, 0);
        Uint128 r = make(0, 0);
        for (int i = 127; i >= 0; i--) {
            r = u128_shl1(r);
            pppp_uint64 bit = i >= 64 ? (n.hi >> (i - 64)) & 1u : (n.lo >> i) & 1u;
            r.lo |= bit;
            if (u128_cmp(r, d) >= 0) {
                r = u128_sub(r, d);
                if (i >= 64) {
                    q.hi |= static_cast<pppp_uint64>(1) << (i - 64);
                } else {
                    q.lo |= static_cast<pppp_uint64>(1) << i;
                }
            }
        }
        *quotient = q;
        *remainder = r;
    }
}}} // namespace pppp::utils::math
