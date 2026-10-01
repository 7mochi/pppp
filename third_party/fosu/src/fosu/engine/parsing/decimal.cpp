#include <fosu/engine/parsing/decimal.h>

#include <cstring>

// Multiprecision decimal conversion after Go's strconv (decimal.go / atof.go floatBits): the
// digits are kept as a decimal string, scaled by powers of two (which are exact in decimal)
// until the value lies in [0.5, 1) * 2^exp, then the leading 1+mantbits bits are rounded to
// nearest-even with a sticky "truncated" flag. Exact for every input; slow only for inputs
// with more digits than the fast path below handles.

namespace fosu { namespace internal {

    namespace {

        const int kMaxDecimalDigits = 800;
        const fosu_uint32 kMaxShift = 60; // 2^60 * 9 fits an unsigned 64-bit accumulator

        struct Decimal {
            unsigned char d[kMaxDecimalDigits]; // digits '0'..'9'
            int nd; // digit count
            int dp; // decimal point: value = 0.d[0]d[1]... * 10^dp
            bool neg;
            bool trunc; // nonzero digits were dropped beyond kMaxDecimalDigits
        };

        struct FloatInfo {
            fosu_uint32 mantbits;
            fosu_uint32 expbits;
            int bias;
        };

        void trim(Decimal& a) {
            while (a.nd > 0 && a.d[a.nd - 1] == '0') {
                a.nd--;
            }
            if (a.nd == 0) {
                a.dp = 0;
            }
        }

        void decimal_set(Decimal& b, const DecimalNumber& number) {
            b.nd = 0;
            b.dp = 0;
            b.neg = number.negative;
            b.trunc = false;
            for (int part = 0; part < 2; part++) {
                const char* p = part == 0 ? number.integer_begin : number.fraction_begin;
                const size_t length = part == 0 ? number.integer_length : number.fraction_length;
                if (part == 1) {
                    b.dp = b.nd; // the decimal point sits after the integer digits
                }
                for (size_t i = 0; i < length; i++) {
                    const char c = p[i];
                    if (c == '0' && b.nd == 0) { // ignore leading zeros
                        b.dp--;
                        continue;
                    }
                    if (b.nd < kMaxDecimalDigits) {
                        b.d[b.nd] = static_cast<unsigned char>(c);
                        b.nd++;
                    } else if (c != '0') {
                        b.trunc = true;
                    }
                }
            }
            if (number.fraction_begin == 0) {
                b.dp = b.nd;
            }
            // The explicit exponent moves the decimal point; its magnitude is already capped by the
            // scanner, and the point cannot overflow an int.
            b.dp += static_cast<int>(number.exponent);
            trim(b);
        }

        // Cutoffs for a left shift by k bits: multiplying by 2^k adds `delta` (= the digit count of
        // 2^k) digits when the leading digits are >= the decimal representation of 5^k, else delta - 1.
        // Since 2^k * 5^k = 10^k, digits(2^k) = k + 1 - digits(5^k).
        struct LeftCheat {
            int delta;
            char cutoff[64];
        };

        const LeftCheat& left_cheat(fosu_uint32 k) {
            static LeftCheat table[kMaxShift + 1];
            static bool built = false;
            if (!built) {
                // 5^k as decimal digits, built by repeated multiplication of the digit string
                unsigned char digits[64];
                int count = 1;
                digits[0] = 1;
                table[0].delta = 0;
                table[0].cutoff[0] = 0;
                for (fosu_uint32 k2 = 1; k2 <= kMaxShift; k2++) {
                    int carry = 0;
                    for (int i = 0; i < count; i++) {
                        const int value = digits[i] * 5 + carry;
                        digits[i] = static_cast<unsigned char>(value % 10);
                        carry = value / 10;
                    }
                    while (carry) {
                        digits[count++] = static_cast<unsigned char>(carry % 10);
                        carry /= 10;
                    }
                    table[k2].delta = static_cast<int>(k2) + 1 - count;
                    for (int i = 0; i < count; i++) {
                        table[k2].cutoff[i] = static_cast<char>('0' + digits[count - 1 - i]);
                    }
                    table[k2].cutoff[count] = 0;
                }
                built = true;
            }
            return table[k];
        }

        bool prefix_is_less_than(const unsigned char* b, int nb, const char* s) {
            for (int i = 0; s[i]; i++) {
                if (i >= nb) {
                    return true;
                }
                if (b[i] != static_cast<unsigned char>(s[i])) {
                    return b[i] < static_cast<unsigned char>(s[i]);
                }
            }
            return false;
        }

        // Multiply by 2^k (0 < k <= kMaxShift).
        void left_shift(Decimal& a, fosu_uint32 k) {
            const LeftCheat& cheat = left_cheat(k);
            int delta = cheat.delta;
            if (prefix_is_less_than(a.d, a.nd, cheat.cutoff)) {
                delta--;
            }

            int r = a.nd; // read index
            int w = a.nd + delta; // write index
            fosu_uint64 n = 0;
            for (r--; r >= 0; r--) {
                n += static_cast<fosu_uint64>(a.d[r] - '0') << k;
                const fosu_uint64 quo = n / 10;
                const fosu_uint64 rem = n - 10 * quo;
                w--;
                if (w < kMaxDecimalDigits) {
                    a.d[w] = static_cast<unsigned char>(rem + '0');
                } else if (rem != 0) {
                    a.trunc = true;
                }
                n = quo;
            }
            while (n > 0) {
                const fosu_uint64 quo = n / 10;
                const fosu_uint64 rem = n - 10 * quo;
                w--;
                if (w < kMaxDecimalDigits) {
                    a.d[w] = static_cast<unsigned char>(rem + '0');
                } else if (rem != 0) {
                    a.trunc = true;
                }
                n = quo;
            }
            a.nd += delta;
            if (a.nd >= kMaxDecimalDigits) {
                a.nd = kMaxDecimalDigits;
            }
            a.dp += delta;
            trim(a);
        }

        // Divide by 2^k (0 < k <= kMaxShift).
        void right_shift(Decimal& a, fosu_uint32 k) {
            int r = 0; // read pointer
            int w = 0; // write pointer
            fosu_uint64 n = 0;
            for (; (n >> k) == 0; r++) {
                if (r >= a.nd) {
                    if (n == 0) { // a == 0
                        a.nd = 0;
                        return;
                    }
                    while ((n >> k) == 0) {
                        n = n * 10;
                        r++;
                    }
                    break;
                }
                n = n * 10 + static_cast<fosu_uint64>(a.d[r] - '0');
            }
            a.dp -= r - 1;
            const fosu_uint64 mask = (static_cast<fosu_uint64>(1) << k) - 1;
            for (; r < a.nd; r++) {
                const fosu_uint64 c = static_cast<fosu_uint64>(a.d[r] - '0');
                const fosu_uint64 dig = n >> k;
                n &= mask;
                a.d[w] = static_cast<unsigned char>(dig + '0');
                w++;
                n = n * 10 + c;
            }
            while (n > 0) {
                const fosu_uint64 dig = n >> k;
                n &= mask;
                if (w < kMaxDecimalDigits) {
                    a.d[w] = static_cast<unsigned char>(dig + '0');
                    w++;
                } else if (dig > 0) {
                    a.trunc = true;
                }
                n = n * 10;
            }
            a.nd = w;
            trim(a);
        }

        void shift(Decimal& a, int k) {
            if (a.nd == 0) {
                return;
            }
            if (k > 0) {
                while (k > static_cast<int>(kMaxShift)) {
                    left_shift(a, kMaxShift);
                    k -= static_cast<int>(kMaxShift);
                }
                left_shift(a, static_cast<fosu_uint32>(k));
            } else if (k < 0) {
                while (k < -static_cast<int>(kMaxShift)) {
                    right_shift(a, kMaxShift);
                    k += static_cast<int>(kMaxShift);
                }
                right_shift(a, static_cast<fosu_uint32>(-k));
            }
        }

        // Round the number to an integer at digit position nd?
        bool should_round_up(const Decimal& a, int nd) {
            if (nd < 0 || nd >= a.nd) {
                return false;
            }
            if (a.d[nd] == '5' && nd + 1 == a.nd) { // exactly halfway - round to even
                // if we truncated, a little higher than what's recorded - always round up
                if (a.trunc) {
                    return true;
                }
                return nd > 0 && ((a.d[nd - 1] - '0') % 2 == 1);
            }
            return a.d[nd] >= '5'; // not halfway - digit tells all
        }

        // Integer part rounded to nearest-even (the decimal is scaled so it fits 64 bits).
        fosu_uint64 rounded_integer(const Decimal& a) {
            if (a.dp > 20) {
                return uint64_max();
            }
            int i;
            fosu_uint64 n = 0;
            for (i = 0; i < a.dp && i < a.nd; i++) {
                n = n * 10 + static_cast<fosu_uint64>(a.d[i] - '0');
            }
            for (; i < a.dp; i++) {
                n *= 10;
            }
            if (should_round_up(a, a.dp)) {
                n++;
            }
            return n;
        }

        const int kPowTab[] = {1, 3, 6, 9, 13, 16, 19, 23, 26};
        const int kPowTabSize = 9;

        // Bits of the nearest floating-point value of `d`; returns false on overflow.
        bool float_bits(Decimal& d, const FloatInfo& flt, fosu_uint64& bits) {
            int exp = 0;
            fosu_uint64 mant = 0;
            bool overflow = false;

            if (d.nd == 0) {
                mant = 0;
                exp = flt.bias;
            } else if (d.dp > 310) {
                overflow = true;
            } else if (d.dp < -330) {
                mant = 0;
                exp = flt.bias;
            } else {
                // Scale by powers of two until in range [0.5, 1.0)
                while (d.dp > 0) {
                    const int n = d.dp >= kPowTabSize ? 27 : kPowTab[d.dp];
                    shift(d, -n);
                    exp += n;
                }
                while (d.dp < 0 || (d.dp == 0 && d.d[0] < '5')) {
                    const int n = -d.dp >= kPowTabSize ? 27 : kPowTab[-d.dp];
                    shift(d, n);
                    exp -= n;
                }
                // Our range is [0.5,1) but floating point range is [1,2).
                exp--;
                // Minimum representable exponent is flt.bias+1. If the exponent is smaller, move it
                // up and adjust d accordingly.
                if (exp < flt.bias + 1) {
                    const int n = flt.bias + 1 - exp;
                    shift(d, -n);
                    exp += n;
                }
                if (exp - flt.bias >= (1 << flt.expbits) - 1) {
                    overflow = true;
                } else {
                    // Extract 1+mantbits bits.
                    shift(d, static_cast<int>(1 + flt.mantbits));
                    mant = rounded_integer(d);
                    // Rounding might have added a bit; shift down.
                    if (mant == (static_cast<fosu_uint64>(2) << flt.mantbits)) {
                        mant >>= 1;
                        exp++;
                        if (exp - flt.bias >= (1 << flt.expbits) - 1) {
                            overflow = true;
                        }
                    }
                    // Denormalized?
                    if ((mant & (static_cast<fosu_uint64>(1) << flt.mantbits)) == 0) {
                        exp = flt.bias;
                    }
                }
            }
            if (overflow) {
                return false;
            }

            bits = mant & ((static_cast<fosu_uint64>(1) << flt.mantbits) - 1);
            bits |= static_cast<fosu_uint64>((exp - flt.bias) & ((1 << flt.expbits) - 1)) << flt.mantbits;
            if (d.neg) {
                bits |= static_cast<fosu_uint64>(1) << flt.mantbits << flt.expbits;
            }
            return true;
        }

        // Clinger's fast path: an integer mantissa that fits the significand exactly, times or divided
        // by an exactly representable power of ten, is a single correctly rounded operation.
        const double kPow10Double[23] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
                                         1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
                                         1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
        const float kPow10Float[11] = {1e0f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f, 1e10f};

        // Significant digits as an integer (at most 19 digits), with the decimal exponent that makes
        // mantissa * 10^exponent the value. Returns false when there are more than 19 significant
        // digits (the slow path handles those).
        bool significand(const DecimalNumber& number, fosu_uint64& mantissa, fosu_int64& exponent) {
            mantissa = 0;
            int digits = 0;
            int fraction_digits = 0;
            for (int part = 0; part < 2; part++) {
                const char* p = part == 0 ? number.integer_begin : number.fraction_begin;
                const size_t length = part == 0 ? number.integer_length : number.fraction_length;
                for (size_t i = 0; i < length; i++) {
                    const char c = p[i];
                    if (part == 1) {
                        fraction_digits++;
                    }
                    if (c == '0' && digits == 0) {
                        continue; // leading zeros
                    }
                    if (digits == 19) {
                        return false;
                    }
                    mantissa = mantissa * 10 + static_cast<fosu_uint64>(c - '0');
                    digits++;
                }
            }
            exponent = number.exponent - fraction_digits;
            return true;
        }

    } // namespace

    const char* scan_decimal(const char* p, const char* end, DecimalNumber& out) {
        const char* start = p;
        out.negative = false;
        out.exponent = 0;
        out.fraction_begin = 0;
        out.fraction_length = 0;
        if (p < end && *p == '-') {
            out.negative = true;
            ++p;
        }
        out.integer_begin = p;
        while (p < end && static_cast<unsigned char>(*p - '0') <= 9) {
            ++p;
        }
        out.integer_length = static_cast<size_t>(p - out.integer_begin);
        size_t digit_count = out.integer_length;
        if (p < end && *p == '.') {
            ++p;
            out.fraction_begin = p;
            while (p < end && static_cast<unsigned char>(*p - '0') <= 9) {
                ++p;
            }
            out.fraction_length = static_cast<size_t>(p - out.fraction_begin);
            digit_count += out.fraction_length;
        }
        if (digit_count == 0) {
            return start;
        }
        if (p < end && (*p == 'e' || *p == 'E')) {
            const char* location_of_e = p;
            ++p;
            bool negative_exponent = false;
            if (p < end && *p == '-') {
                negative_exponent = true;
                ++p;
            } else if (p < end && *p == '+') {
                ++p;
            }
            if (p >= end || static_cast<unsigned char>(*p - '0') > 9) {
                p = location_of_e; // not an exponent after all
            } else {
                fosu_int64 exp_number = 0;
                while (p < end && static_cast<unsigned char>(*p - '0') <= 9) {
                    if (exp_number < 0x10000) {
                        exp_number = 10 * exp_number + (*p - '0');
                    }
                    ++p;
                }
                out.exponent = negative_exponent ? -exp_number : exp_number;
            }
        }
        return p;
    }

    bool decimal_to_double(const DecimalNumber& number, double& out) {
        fosu_uint64 mantissa;
        fosu_int64 exponent;
        if (significand(number, mantissa, exponent)) {
            if (mantissa == 0) {
                out = number.negative ? -0.0 : 0.0;
                return true;
            }
            if (mantissa <= (static_cast<fosu_uint64>(1) << 53) && exponent >= -22 && exponent <= 22) {
                double value = static_cast<double>(mantissa);
                if (exponent < 0) {
                    value /= kPow10Double[-exponent];
                } else {
                    value *= kPow10Double[exponent];
                }
                out = number.negative ? -value : value;
                return true;
            }
        }
        Decimal d;
        decimal_set(d, number);
        FloatInfo info;
        info.mantbits = 52;
        info.expbits = 11;
        info.bias = -1023;
        fosu_uint64 bits;
        if (!float_bits(d, info, bits)) {
            return false;
        }
        std::memcpy(&out, &bits, sizeof(out));
        return true;
    }

    bool decimal_to_float(const DecimalNumber& number, float& out) {
        fosu_uint64 mantissa;
        fosu_int64 exponent;
        if (significand(number, mantissa, exponent)) {
            if (mantissa == 0) {
                out = number.negative ? -0.0f : 0.0f;
                return true;
            }
            if (mantissa <= (static_cast<fosu_uint64>(1) << 24) && exponent >= -10 && exponent <= 10) {
                float value = static_cast<float>(mantissa);
                if (exponent < 0) {
                    value /= kPow10Float[-exponent];
                } else {
                    value *= kPow10Float[exponent];
                }
                out = number.negative ? -value : value;
                return true;
            }
        }
        Decimal d;
        decimal_set(d, number);
        FloatInfo info;
        info.mantbits = 23;
        info.expbits = 8;
        info.bias = -127;
        fosu_uint64 bits;
        if (!float_bits(d, info, bits)) {
            return false;
        }
        const fosu_uint32 bits32 = static_cast<fosu_uint32>(bits);
        std::memcpy(&out, &bits32, sizeof(out));
        return true;
    }

}} // namespace fosu::internal
