#include "pppp/utils/math/csharp.h"
#include "pppp/config.h"
#include "pppp/utils/precision.h"
#include <cmath>
#include <cstring>
#include <limits>

namespace pppp { namespace utils { namespace math {
    namespace {
        /// The constants and polynomials for sine and cosine.
        struct SinCos {
            /// Sign of sine in quadrants 0..3.
            double sign[4];
            /// 2 / PI ( * 2^24 if !TOINT_INTRINSICS).
            double hpi_inv;
            /// PI / 2.
            double hpi;
            /// Cosine polynomial.
            double c0, c1, c2, c3, c4;
            /// Sine polynomial.
            double s1, s2, s3;
        };

        /// The constants and polynomials for sine and cosine. The 2nd entry computes -cos (x) rather than
        /// cos (x) to get negation for free.
        const SinCos SINCOSF_TABLE[2] = {{{1.0, -1.0, -1.0, 1.0},
                                          10680707.430881744,
                                          1.5707963267948966,
                                          1.0,
                                          -0.49999999725108224,
                                          0.041666623324344516,
                                          -0.0013886763794376041,
                                          2.4390450703564542e-05,
                                          -0.16666654943701084,
                                          0.0083321781461388536,
                                          -0.00019517298981385725},
                                         {{1.0, -1.0, -1.0, 1.0},
                                          10680707.430881744,
                                          1.5707963267948966,
                                          -1.0,
                                          0.49999999725108224,
                                          -0.041666623324344516,
                                          0.0013886763794376041,
                                          -2.4390450703564542e-05,
                                          -0.16666654943701084,
                                          0.0083321781461388536,
                                          -0.00019517298981385725}};

        /// Table with 4/PI to 192 bit precision. To avoid unaligned accesses only 8 new bits are added per
        /// entry, making the table 4 times larger.
        const unsigned INV_PIO4[24] = {0xa2u,       0xa2f9u,     0xa2f983u,   0xa2f9836eu, 0xf9836e4eu,
                                       0x836e4e44u, 0x6e4e4415u, 0x4e441529u, 0x441529fcu, 0x1529fc27u,
                                       0x29fc2757u, 0xfc2757d1u, 0x2757d1f5u, 0x57d1f534u, 0xd1f534ddu,
                                       0xf534ddc0u, 0x34ddc0dbu, 0xddc0db62u, 0xc0db6295u, 0xdb629599u,
                                       0x6295993cu, 0x95993c43u, 0x993c4390u, 0x3c439041u};

        unsigned asuint(float f) {
            unsigned bits = 0;
            std::memcpy(&bits, &f, sizeof(f));
            return bits;
        }

        pppp_uint64 asuint64(double d) {
            pppp_uint64 bits = 0;
            std::memcpy(&bits, &d, sizeof(d));
            return bits;
        }

        double asdouble(pppp_uint64 bits) {
            double d = 0;
            std::memcpy(&d, &bits, sizeof(d));
            return d;
        }

        float math_uflowf(unsigned sign) {
            const float x = 2.5243548967072378e-29f;
            return (sign ? -x : x) * x;
        }

        float math_invalidf(float x) { return (x - x) / (x - x); }

        /// Top 12 bits of the float representation with the sign bit cleared.
        unsigned abstop12(float x) { return (asuint(x) >> 20) & 0x7ffu; }

        /// Fast range reduction using single multiply-subtract. Return the modulo of X as a value between
        /// -PI/4 and PI/4 and store the quadrant in NP. The values for PI/2 and 2/PI are accessed via P.
        /// Since PI/2 as a double is accurate to 55 bits and the worst-case cancellation happens at
        /// 6 * PI/4, the result is accurate for |X| <= 120.0.
        double reduce_fast(double x, const SinCos* p, int* np) {
            double r;
            // Use scaled float to int conversion with explicit rounding.
            // hpi_inv is prescaled by 2^24 so the quadrant ends up in bits 24..31.
            // This avoids inaccuracies introduced by truncating negative values.
            r = x * p->hpi_inv;
            const int scaled = static_cast<int>(r) + 0x800000;
            int n = scaled >= 0 ? scaled >> 24 : -((-scaled + 0xffffff) >> 24);
            *np = n;
            return (x - n * SINCOSF_HPI_HI) - n * SINCOSF_HPI_LO;
        }

        /// Reduce the range of XI to a multiple of PI/2 using fast integer arithmetic. XI is a
        /// reinterpreted float and must be >= 2.0f (the sign bit is ignored). Return the modulo between
        /// -PI/4 and PI/4 and store the quadrant in NP. Reduction uses a table of 4/PI with 192 bits of
        /// precision. A 32x96->128 bit multiply computes the exact 2.62-bit fixed-point modulo. Since the
        /// result can have at most 29 leading zeros after the binary point, the double precision result
        /// is accurate to 33 bits.
        double reduce_large(unsigned xi, int* np) {
            const unsigned* arr = &INV_PIO4[(xi >> 26) & 15];
            int shift = static_cast<int>((xi >> 23) & 7);
            pppp_uint64 n, res0, res1, res2;

            xi = (xi & 0xffffffu) | 0x800000u;
            xi <<= shift;

            res0 = (static_cast<pppp_uint64>(xi) * arr[0]) & 0xffffffffu;
            res1 = static_cast<pppp_uint64>(xi) * arr[4];
            res2 = static_cast<pppp_uint64>(xi) * arr[8];
            res0 = (res2 >> 32) | (res0 << 32);
            res0 += res1;

            n = (res0 + (static_cast<pppp_uint64>(1) << 61)) >> 62;
            res0 -= n << 62;
            double x = static_cast<double>(static_cast<pppp_int64>(res0));
            *np = static_cast<int>(n);
            return x * SINCOSF_PI63;
        }

        /// Return the sine of inputs X and X2 (X squared) using the polynomial P. N is the quadrant, and if
        /// odd the cosine polynomial is used.
        float sinf_poly(double x, double x2, const SinCos* p, int n) {
            double x3, x4, x6, x7, s, c, c1, c2, s1;

            if ((n & 1) == 0) {
                x3 = x * x2;
                s1 = p->s2 + x2 * p->s3;

                x7 = x3 * x2;
                s = x + x3 * p->s1;

                return static_cast<float>(s + x7 * s1);
            }

            x4 = x2 * x2;
            c2 = p->c3 + x2 * p->c4;
            c1 = p->c0 + x2 * p->c1;

            x6 = x4 * x2;
            c = c1 + x4 * p->c2;

            return static_cast<float>(c + x6 * c2);
        }

        double product_error(double a, double b, double p) {
            const double split = 134217729.0;
            const double ca = split * a;
            const double a_hi = ca - (ca - a);
            const double a_lo = a - a_hi;
            const double cb = split * b;
            const double b_hi = cb - (cb - b);
            const double b_lo = b - b_hi;
            return ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
        }

        double division_remainder(double q, double d, double n) {
            const double p = q * d;
            return (n - p) - product_error(q, d, p);
        }

        // Copyright (c) 2022-2024 Alexei Sibidanov and Paul Zimmermann.
        //
        // The original version of this file was copied from the CORE-MATH
        // project (file src/binary32/atan2/atan2f.c, revision 9b28a4a).
        //
        // Permission is hereby granted, free of charge, to any person obtaining a copy
        // of this software and associated documentation files (the "Software"), to deal
        // in the Software without restriction, including without limitation the rights
        // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        // copies of the Software, and to permit persons to whom the Software is
        // furnished to do so, subject to the following conditions:
        //
        // The above copyright notice and this permission notice shall be included in all
        // copies or substantial portions of the Software.
        //
        // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
        // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        // SOFTWARE.

        double muldd(double xh, double xl, double ch, double cl, double* l) {
            double ahlh = ch * xl;
            double alhh = cl * xh;
            double ahhh = ch * xh;
            double ahhl = product_error(ch, xh, ahhh);
            ahhl += alhh + ahlh;
            ch = ahhh + ahhl;
            *l = (ahhh - ch) + ahhl;
            return ch;
        }

        double polydd(double xh, double xl, int n, const double c[][2], double* l) {
            int i = n - 1;
            double ch = c[i][0];
            double cl = c[i][1];
            while (--i >= 0) {
                ch = muldd(xh, xl, ch, cl, &cl);
                double th = ch + c[i][0];
                double tl = (c[i][0] - th) + ch;
                ch = th;
                cl += tl + c[i][1];
            }
            *l = cl;
            return ch;
        }

        /// for y/x tiny, use Taylor approximation z - z^3/3 where z=y/x
        float cr_atan2f_tiny(float y, float x) {
            double dy = y;
            double dx = x;
            double z = dy / dx;
            double e = division_remainder(z, dx, dy);
            // z * x + e = y thus y/x = z + e/x
            const double c = -0.33333333333333331; // -1/3 rounded to nearest
            double zz = z * z;
            double cz = c * z;
            e = e / dx + cz * zz;
            pppp_uint64 t = asuint64(z);
            if ((t & 0xfffffffu) == 0) { // boundary case
                // If z and e are of same sign (resp. of different signs), we increase
                // (resp. decrease) the significant of t by 1 to avoid a double-rounding
                // issue when rounding t to binary32.
                if (z * e > 0) {
                    t += 1;
                } else {
                    t -= 1;
                }
            }
            double r = asdouble(t);
            float rf = static_cast<float>(r);
            if (rf == 0.0f) {
                return math_uflowf(static_cast<unsigned>(t >> 63));
            }
            return rf;
        }
    } // namespace

    double clamp(double x, double lo, double hi) {
        if (x < lo) {
            return lo;
        }
        if (x > hi) {
            return hi;
        }
        return x;
    }

    double lerp(double a, double b, double t) { return a + (b - a) * t; }

    double lerp_dotnet(double value1, double value2, double amount) {
        return (value1 * (1.0 - amount)) + (value2 * amount);
    }

    double degrees_to_radians(double degrees) { return (degrees * PI_D) / 180.0; }

    double cbrt(double x) {
        // Reduce X.  XM now is an range 1.0 to 0.5.
        int xe = 0;
        const double xm = std::frexp(std::fabs(x), &xe);

        // If X is not finite or is null return it (with raising exceptions
        // if necessary.
        if (x == 0.0 || !(x - x == 0.0)) {
            return x + x;
        }

        const double u =
            (0.354895765043919860 +
             ((1.50819193781584896 +
               ((-2.11499494167371287 +
                 ((2.44693122563534430 +
                   ((-1.83469277483613086 + (0.784932344976639262 - 0.145263899385486377 * xm) * xm) * xm)) *
                  xm)) *
                xm)) *
              xm));

        const double t2 = u * u * u;

        const int xe_div3 = xe >= 0 ? xe / 3 : -(-xe / 3);
        const int xe_mod3 = xe >= 0 ? xe % 3 : -(-xe % 3);

        const double ym = u * (t2 + 2.0 * xm) / (2.0 * t2 + xm) * CBRT_FACTOR[2 + xe_mod3];

        return std::ldexp(x > 0.0 ? ym : -ym, xe_div3);
    }

    float sinf(float y) {
        double x = y;
        double s;
        int n;
        const SinCos* p = &SINCOSF_TABLE[0];

        if (abstop12(y) < abstop12(SINCOSF_PIO4)) {
            s = x * x;

            if (abstop12(y) < abstop12(0.000244140625f)) {
                return y;
            }

            return sinf_poly(x, s, p, 0);
        }
        if (abstop12(y) < abstop12(120.0f)) {
            x = reduce_fast(x, p, &n);

            // Setup the signs for sin and cos.
            s = p->sign[n & 3];

            if (n & 2) {
                p = &SINCOSF_TABLE[1];
            }

            return sinf_poly(x * s, x * x, p, n);
        }
        if (abstop12(y) < abstop12(std::numeric_limits<float>::infinity())) {
            unsigned xi = asuint(y);
            int sign = static_cast<int>(xi >> 31);

            x = reduce_large(xi, &n);

            // Setup signs for sin and cos - include original sign.
            s = p->sign[(n + sign) & 3];

            if ((n + sign) & 2) {
                p = &SINCOSF_TABLE[1];
            }

            return sinf_poly(x * s, x * x, p, n);
        }
        return math_invalidf(y);
    }

    float cosf(float y) {
        double x = y;
        double s;
        int n;
        const SinCos* p = &SINCOSF_TABLE[0];

        if (abstop12(y) < abstop12(SINCOSF_PIO4)) {
            double x2 = x * x;

            if (abstop12(y) < abstop12(0.000244140625f)) {
                return 1.0f;
            }

            return sinf_poly(x, x2, p, 1);
        }
        if (abstop12(y) < abstop12(120.0f)) {
            x = reduce_fast(x, p, &n);

            // Setup the signs for sin and cos.
            s = p->sign[n & 3];

            if (n & 2) {
                p = &SINCOSF_TABLE[1];
            }

            return sinf_poly(x * s, x * x, p, n ^ 1);
        }
        if (abstop12(y) < abstop12(std::numeric_limits<float>::infinity())) {
            unsigned xi = asuint(y);
            int sign = static_cast<int>(xi >> 31);

            x = reduce_large(xi, &n);

            // Setup signs for sin and cos - include original sign.
            s = p->sign[(n + sign) & 3];

            if ((n + sign) & 2) {
                p = &SINCOSF_TABLE[1];
            }

            return sinf_poly(x * s, x * x, p, n ^ 1);
        }
        return math_invalidf(y);
    }

    float atan2f(float y, float x) {
        static const double cn[] = {1.0,
                                    2.5068485213355651,
                                    2.2855336234350774,
                                    0.92275406111120506,
                                    0.15965700667756133,
                                    0.0093982071883745005,
                                    8.1162663838090543e-05};
        static const double cd[] = {1.0,
                                    2.8401818546688959,
                                    3.0322609083249099,
                                    1.5083284691366383,
                                    0.35061013533424623,
                                    0.03311601651598859,
                                    0.00083070468185660116};
        static const double m[] = {0, 1};
        const double pi = 3.1415926535897931;
        const double pi2 = 1.5707963267948966;
        static const double off[] = {0.0,  1.5707963267948966,  3.1415926535897931,  1.5707963267948966,
                                     -0.0, -1.5707963267948966, -3.1415926535897931, -1.5707963267948966};
        static const double offl[] = {
            0.0,  6.123233995736766e-17,  2 * 6.123233995736766e-17,  6.123233995736766e-17,
            -0.0, -6.123233995736766e-17, -2 * 6.123233995736766e-17, -6.123233995736766e-17};
        static const double sgn[] = {1, -1};
        unsigned ux = asuint(x);
        unsigned uy = asuint(y);
        unsigned ax = ux & 0x7fffffffu;
        unsigned ay = uy & 0x7fffffffu;
        if (ay >= 0x7f800000u || ax >= 0x7f800000u) {
            // we use x+y below so that the invalid exception is set
            // for (x,y) = (qnan,snan) or (snan,qnan)
            if (ay > 0x7f800000u) {
                return x + y; // nan
            }
            if (ax > 0x7f800000u) {
                return x + y; // nan
            }
            bool yinf = ay == 0x7f800000u;
            bool xinf = ax == 0x7f800000u;
            if (yinf && xinf) {
                if (ux >> 31) {
                    return static_cast<float>(2.3561944901923448 * sgn[uy >> 31]); // +/-3pi/4
                }
                return static_cast<float>(0.78539816339744828 * sgn[uy >> 31]); // +/-pi/4
            }
            if (xinf) {
                if (ux >> 31) {
                    return static_cast<float>(pi * sgn[uy >> 31]);
                }
                return static_cast<float>(0.0 * sgn[uy >> 31]);
            }
            if (yinf) {
                return static_cast<float>(pi2 * sgn[uy >> 31]);
            }
        }
        if (ay == 0) {
            if (!ax) {
                unsigned i = (uy >> 31) * 4 + (ux >> 31) * 2;
                if (ux >> 31) {
                    return static_cast<float>(off[i] + offl[i]);
                }
                return static_cast<float>(off[i]);
            }
            if (!(ux >> 31)) {
                return static_cast<float>(0.0 * sgn[uy >> 31]);
            }
        }
        unsigned gt = ay > ax;
        unsigned i = (uy >> 31) * 4 + (ux >> 31) * 2 + gt;

        double zx = x;
        double zy = y;
        double z = (m[gt] * zx + m[1 - gt] * zy) / (m[gt] * zy + m[1 - gt] * zx);
        // z = x/y if |y| > |x|, and z = y/x otherwise
        double r;
        int d = static_cast<int>(ax) - static_cast<int>(ay);
        if (d < (27 << 23) && d > (-(27 << 23))) {
            double z2 = z * z, z4 = z2 * z2, z8 = z4 * z4;
            // z2 cannot underflow, since for |y|=0x1p-149 and |x|=0x1.fffffep+127
            // we get |z| > 2^-277 thus z2 > 2^-554, but z4 and z8 might underflow,
            // which might give spurious underflow exceptions.
            double cn0 = cn[0] + z2 * cn[1];
            double cn2 = cn[2] + z2 * cn[3];
            double cn4 = cn[4] + z2 * cn[5];
            double cn6 = cn[6];
            cn0 += z4 * cn2;
            cn4 += z4 * cn6;
            cn0 += z8 * cn4;
            double cd0 = cd[0] + z2 * cd[1];
            double cd2 = cd[2] + z2 * cd[3];
            double cd4 = cd[4] + z2 * cd[5];
            double cd6 = cd[6];
            cd0 += z4 * cd2;
            cd4 += z4 * cd6;
            cd0 += z8 * cd4;
            r = cn0 / cd0;
        } else {
            r = 1;
        }
        z *= sgn[gt];
        r = z * r + off[i];
        if (((asuint64(r) + 8) & 0xfffffffu) <= 16) {
            // check tiny y/x
            if (ay < ax && ((ax - ay) >> 23 >= 25)) {
                return cr_atan2f_tiny(y, x);
            }
            double zh;
            double zl;
            if (gt == 0) {
                zh = zy / zx;
                zl = division_remainder(zh, zx, zy) / zx;
            } else {
                zh = zx / zy;
                zl = division_remainder(zh, zy, zx) / zy;
            }
            double z2l;
            double z2h = muldd(zh, zl, zh, zl, &z2l);
            static const double c[32][2] = {{1, -9.9993713909802762e-27},
                                            {-0.33333333333333331, -1.8503696081891694e-17},
                                            {0.20000000000000001, -1.1109570448589454e-17},
                                            {-0.14285714285714285, -6.906377603268134e-18},
                                            {0.11111111111111104, -6.0498620754704096e-19},
                                            {-0.090909090909087401, -4.813186107306423e-18},
                                            {0.07692307692296789, 6.0635784048294865e-18},
                                            {-0.066666666664230045, 5.3573683924228454e-19},
                                            {0.058823529370947883, -3.0234229312063074e-18},
                                            {-0.052631578418319884, 7.1219063142300131e-19},
                                            {0.047619042181978474, 3.2606056478649954e-18},
                                            {-0.043478215705419529, 2.1474249882470664e-18},
                                            {0.039999692056834395, -1.4496021955960109e-18},
                                            {-0.037035291887394496, 8.3946147130053678e-19},
                                            {0.034474453317332822, 3.5732167404770099e-19},
                                            {-0.032224585930579239, -6.0591993063025197e-19},
                                            {0.030187892413408284, 8.5503597778482721e-19},
                                            {-0.028231467664296316, 8.6457562390298748e-19},
                                            {0.026160406443643782, -1.1013132206776929e-18},
                                            {-0.023723636947114012, 5.8205312635420101e-19},
                                            {0.0206884675894418, 8.7895316349397806e-19},
                                            {-0.016981732349686016, 9.5194871038492798e-19},
                                            {0.012817485672589527, -7.7422393466476828e-19},
                                            {-0.0086873139633617431, -5.6307156457055604e-19},
                                            {0.0051636184396118149, 3.2806307955849715e-19},
                                            {-0.0026269189247960299, -1.3737775104430079e-19},
                                            {0.0011135875725313154, -6.6187460764317516e-20},
                                            {-0.00038080882644929778, 1.5620808802045265e-20},
                                            {0.00010056717515235685, -5.1946273311009068e-21},
                                            {-1.919552508092125e-05, 8.2028906555295314e-22},
                                            {2.3516721065233102e-06, -1.3883775330103416e-22},
                                            {-1.3863591848022874e-07, -1.0947593470915086e-23}};
            double pl;
            double ph = polydd(z2h, z2l, 32, c, &pl);
            zh *= sgn[gt];
            zl *= sgn[gt];
            ph = muldd(zh, zl, ph, pl, &pl);
            double sh = ph + off[i];
            double sl = ((off[i] - sh) + ph) + pl + offl[i];
            float rf = static_cast<float>(sh);
            double th = rf;
            double dh = sh - th;
            double tm = dh + sl;
            pppp_uint64 tth = asuint64(th);
            if (th + th * 8.6736173798840355e-19 == th - th * 8.6736173798840355e-19) {
                tth &= static_cast<pppp_uint64>(0x7ff) << 52;
                tth -= static_cast<pppp_uint64>(24) << 52;
                if (std::fabs(tm) > asdouble(tth)) {
                    tm *= 1.25;
                } else {
                    tm *= 0.75;
                }
            }
            r = th + tm;
        }
        float rf = static_cast<float>(r);
        if (rf == 0.0f) {
            return math_uflowf(asuint(rf) >> 1);
        }
        return rf;
    }
}}} // namespace pppp::utils::math
