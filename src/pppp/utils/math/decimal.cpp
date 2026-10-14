#include "pppp/utils/math/decimal.h"
#include "pppp/utils/math/csharp.h"
#include <cstring>

namespace pppp { namespace utils { namespace math {
    namespace {
        const unsigned UINT32_POWERS10[] = {1u,      10u,      100u,      1000u,      10000u,
                                            100000u, 1000000u, 10000000u, 100000000u, 1000000000u};

        pppp_uint64 uint64_power10(int power) {
            pppp_uint64 result = 1;
            for (int i = 0; i < power; i++) {
                result *= 10;
            }
            return result;
        }

        const double DOUBLE_POWERS10[] = {1,    1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,
                                          1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
                                          1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28};

        unsigned get_exponent(float f) {
            // Based on pulling out the exp from this single struct layout
            // typedef struct {
            //    ULONG mant:23;
            //    ULONG exp:8;
            //    ULONG sign:1;
            // } SNGSTRUCT;

            unsigned bits = 0;
            std::memcpy(&bits, &f, sizeof(f));
            return (bits >> 23) & 0xFFu;
        }

        unsigned get_exponent(double d) {
            // Based on pulling out the exp from this double struct layout
            // typedef struct {
            //   DWORDLONG mant:52;
            //   DWORDLONG signexp:12;
            // } DBLSTRUCT;

            pppp_uint64 bits;
            std::memcpy(&bits, &d, sizeof(bits));
            return static_cast<unsigned>(bits >> 52) & 0x7FFu;
        }

        int shift_right_16(int value) { return value >= 0 ? value >> 16 : -((-value + 65535) >> 16); }

        Decimal zero() {
            Decimal result;
            result.mantissa = u128_from(0);
            result.scale = 0;
            result.negative = false;
            return result;
        }
    } // namespace

    bool var_dec_from_r4(float input, Decimal* result) {
        *result = zero();

        // The most we can scale by is 10^28, which is just slightly more
        // than 2^93.  So a float with an exponent of -94 could just
        // barely reach 0.5, but smaller exponents will always round to zero.
        //
        const unsigned sngbias = 126;
        int exp = static_cast<int>(get_exponent(input)) - static_cast<int>(sngbias);
        if (exp < -94) {
            return true; // result should be zeroed out
        }

        if (exp > 96) {
            return false;
        }

        bool negative = false;
        if (input < 0) {
            input = -input;
            negative = true;
        }

        // Round the input to a 7-digit integer.  The R4 format has
        // only 7 digits of precision, and we want to keep garbage digits
        // out of the Decimal were making.
        //
        // Calculate max power of 10 input value could have by multiplying
        // the exponent by log10(2).  Using scaled integer multiplcation,
        // log10(2) * 2 ^ 16 = .30103 * 65536 = 19728.3.
        //
        double dbl = input;
        int power = 6 - shift_right_16(exp * 19728);
        // power is between -22 and 35

        if (power >= 0) {
            // We have less than 7 digits, scale input up.
            //
            if (power > DEC_SCALE_MAX) {
                power = DEC_SCALE_MAX;
            }

            dbl *= DOUBLE_POWERS10[power];
        } else {
            if (power != -1 || dbl >= 1E7) {
                dbl /= DOUBLE_POWERS10[-power];
            } else {
                power = 0; // didn't scale it
            }
        }

        if (dbl < 1E6 && power < DEC_SCALE_MAX) {
            dbl *= 10;
            power++;
        }

        // Round to integer
        //
        unsigned mant = static_cast<unsigned>(round_half_even(dbl));

        if (mant == 0) {
            return true; // result should be zeroed out
        }

        if (power < 0) {
            // Add -power factors of 10, -power <= (29 - 7) = 22.
            //
            power = -power;
            if (power < 10) {
                result->mantissa = u128_mul64(mant, UINT32_POWERS10[power]);
            } else {
                // Have a big power of 10.
                //
                if (power > 18) {
                    pppp_uint64 low64 = static_cast<pppp_uint64>(mant) * UINT32_POWERS10[power - 18];
                    result->mantissa = u128_mul64(low64, uint64_power10(18));
                } else {
                    pppp_uint64 low64 = static_cast<pppp_uint64>(mant) * UINT32_POWERS10[power - 9];
                    result->mantissa = u128_mul64(low64, TEN_TO_POWER_NINE);
                }
            }
        } else {
            // Factor out powers of 10 to reduce the scale, if possible.
            // The maximum number we could factor out would be 6.  This
            // comes from the fact we have a 7-digit number, and the
            // MSD must be non-zero -- but the lower 6 digits could be
            // zero.  Note also the scale factor is never negative, so
            // we can't scale by any more than the power we used to
            // get the integer.
            //
            int lmax = power;
            if (lmax > 6) {
                lmax = 6;
            }

            if ((mant & 0xF) == 0 && lmax >= 4) {
                const unsigned den = 10000;
                unsigned div = mant / den;
                if (mant == div * den) {
                    mant = div;
                    power -= 4;
                    lmax -= 4;
                }
            }

            if ((mant & 3) == 0 && lmax >= 2) {
                const unsigned den = 100;
                unsigned div = mant / den;
                if (mant == div * den) {
                    mant = div;
                    power -= 2;
                    lmax -= 2;
                }
            }

            if ((mant & 1) == 0 && lmax >= 1) {
                const unsigned den = 10;
                unsigned div = mant / den;
                if (mant == div * den) {
                    mant = div;
                    power--;
                }
            }

            result->scale = power;
            result->mantissa = u128_from(mant);
        }

        result->negative = negative;
        return true;
    }

    bool var_dec_from_r8(double input, Decimal* result) {
        *result = zero();

        // The most we can scale by is 10^28, which is just slightly more
        // than 2^93.  So a float with an exponent of -94 could just
        // barely reach 0.5, but smaller exponents will always round to zero.
        //
        const unsigned dblbias = 1022;
        int exp = static_cast<int>(get_exponent(input)) - static_cast<int>(dblbias);
        if (exp < -94) {
            return true; // result should be zeroed out
        }

        if (exp > 96) {
            return false;
        }

        bool negative = false;
        if (input < 0) {
            input = -input;
            negative = true;
        }

        // Round the input to a 15-digit integer.  The R8 format has
        // only 15 digits of precision, and we want to keep garbage digits
        // out of the Decimal were making.
        //
        // Calculate max power of 10 input value could have by multiplying
        // the exponent by log10(2).  Using scaled integer multiplcation,
        // log10(2) * 2 ^ 16 = .30103 * 65536 = 19728.3.
        //
        double dbl = input;
        int power = 14 - shift_right_16(exp * 19728);
        // power is between -14 and 43

        if (power >= 0) {
            // We have less than 15 digits, scale input up.
            //
            if (power > DEC_SCALE_MAX) {
                power = DEC_SCALE_MAX;
            }

            dbl *= DOUBLE_POWERS10[power];
        } else {
            if (power != -1 || dbl >= 1E15) {
                dbl /= DOUBLE_POWERS10[-power];
            } else {
                power = 0; // didn't scale it
            }
        }

        if (dbl < 1E14 && power < DEC_SCALE_MAX) {
            dbl *= 10;
            power++;
        }

        // Round to int64
        //
        pppp_uint64 mant = static_cast<pppp_uint64>(round_half_even(dbl));

        if (mant == 0) {
            return true; // result should be zeroed out
        }

        if (power < 0) {
            // Add -power factors of 10, -power <= (29 - 15) = 14.
            //
            power = -power;
            if (power < 10) {
                result->mantissa = u128_mul64(mant, UINT32_POWERS10[power]);
            } else {
                // Have a big power of 10.
                //
                result->mantissa = u128_mul64(mant, uint64_power10(power));
            }
        } else {
            // Factor out powers of 10 to reduce the scale, if possible.
            // The maximum number we could factor out would be 14.  This
            // comes from the fact we have a 15-digit number, and the
            // MSD must be non-zero -- but the lower 14 digits could be
            // zero.  Note also the scale factor is never negative, so
            // we can't scale by any more than the power we used to
            // get the integer.
            //
            int lmax = power;
            if (lmax > 14) {
                lmax = 14;
            }

            if ((mant & 0xFF) == 0 && lmax >= 8) {
                const pppp_uint64 den = 100000000u;
                pppp_uint64 div = mant / den;
                if ((mant & 0xFFFFFFFFu) == ((div * den) & 0xFFFFFFFFu)) {
                    mant = div;
                    power -= 8;
                    lmax -= 8;
                }
            }

            if ((mant & 0xF) == 0 && lmax >= 4) {
                const pppp_uint64 den = 10000;
                pppp_uint64 div = mant / den;
                if ((mant & 0xFFFFFFFFu) == ((div * den) & 0xFFFFFFFFu)) {
                    mant = div;
                    power -= 4;
                    lmax -= 4;
                }
            }

            if ((mant & 3) == 0 && lmax >= 2) {
                const pppp_uint64 den = 100;
                pppp_uint64 div = mant / den;
                if ((mant & 0xFFFFFFFFu) == ((div * den) & 0xFFFFFFFFu)) {
                    mant = div;
                    power -= 2;
                    lmax -= 2;
                }
            }

            if ((mant & 1) == 0 && lmax >= 1) {
                const pppp_uint64 den = 10;
                pppp_uint64 div = mant / den;
                if ((mant & 0xFFFFFFFFu) == ((div * den) & 0xFFFFFFFFu)) {
                    mant = div;
                    power--;
                }
            }

            result->scale = power;
            result->mantissa = u128_from(mant);
        }

        result->negative = negative;
        return true;
    }

    float var_r4_from_dec(const Decimal& value) { return static_cast<float>(var_r8_from_dec(value)); }

    double var_r8_from_dec(const Decimal& value) {
        // Value taken via reverse engineering the double that corresponds to 2^64. (oleaut32 has ds2to64 =
        // DEFDS(0, 0, DBLBIAS + 65, 0))
        const double ds2to64 = 1.8446744073709552e+019;

        double dbl =
            (static_cast<double>(value.mantissa.lo) + static_cast<double>(value.mantissa.hi) * ds2to64) /
            DOUBLE_POWERS10[value.scale];

        if (value.negative) {
            dbl = -dbl;
        }

        return dbl;
    }
}}} // namespace pppp::utils::math
