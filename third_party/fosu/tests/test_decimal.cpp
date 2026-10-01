// Exact decimal conversion: syntax of the scanner and a differential check of the values
// against the C library (glibc's strtod/strtof are correctly rounded) on random inputs.
#include "harness.h"
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fosu/engine/parsing/decimal.h>
#include <fosu/engine/parsing/numbers.h>

using namespace fosu;
using namespace fosu::internal;

static fosu_uint64 rng_state = 0x243F6A88u;
static fosu_uint64 rng() {
    // xorshift64*
    rng_state ^= rng_state >> 12;
    rng_state ^= rng_state << 25;
    rng_state ^= rng_state >> 27;
    return rng_state * (static_cast<fosu_uint64>(0x2545F491u) << 32 | 0x4F6CDD1Du);
}

static bool same_double(double a, double b) { return std::memcmp(&a, &b, sizeof(a)) == 0; }
static bool same_float(float a, float b) { return std::memcmp(&a, &b, sizeof(a)) == 0; }

PPPP_TEST(scanner_syntax) {
    struct Case {
        const char* text;
        int consumed; // -1 = rejected
        double value;
    };
    static const Case cases[] = {
        {"5", 1, 5.0},   {"5.", 2, 5.0},    {".5", 2, 0.5},
        {"-0", 2, -0.0}, {"1e", 1, 1.0},    {"1e+", 1, 1.0},
        {"1e5", 3, 1e5}, {"1E-2", 4, 0.01}, {"12.5e1x", 6, 125},
        {"007", 3, 7.0}, {".", -1, 0},      {"-", -1, 0},
        {"e5", -1, 0},   {"nan", -1, 0},    {"inf", -1, 0},
        {"+5", -1, 0},   {"1.5.5", 3, 1.5}, {"0.6999999999999999556", 21, 0.7 - 4.44e-17},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char buf[64];
        std::memset(buf, 0, sizeof(buf));
        std::strcpy(buf, cases[i].text);
        DecimalNumber number;
        const char* q = scan_decimal(buf, buf + std::strlen(buf), number);
        if (cases[i].consumed < 0) {
            CHECK_MSG(q == buf, cases[i].text);
            continue;
        }
        CHECK_MSG(q - buf == cases[i].consumed, cases[i].text);
        double value = 0;
        CHECK_MSG(decimal_to_double(number, value), cases[i].text);
        if (i < 17) {
            CHECK_MSG(same_double(value, cases[i].value), cases[i].text);
        } else {
            CHECK_MSG(same_double(value, std::strtod(cases[i].text, 0)), cases[i].text);
        }
    }
    // -0 keeps its sign
    DecimalNumber number;
    char zero[] = "-0.0";
    scan_decimal(zero, zero + 4, number);
    double value = 1;
    decimal_to_double(number, value);
    CHECK(same_double(value, -0.0));
}

PPPP_TEST(overflow_and_underflow) {
    const char* overflow[] = {"1e309", "1.8e308", "1e400",
                              "179769313486231580793728971405303415079934132710037826936173778980444968292764"
                              "750946649017977587207096330286416692887910946555547851940402630657488671505820"
                              "681908902000708383676273854845817711531764475730270069855571366959622842914819"
                              "860834936475292719074168444365510704342711559699508093042880177904174497792"};
    for (size_t i = 0; i < sizeof(overflow) / sizeof(overflow[0]); i++) {
        DecimalNumber number;
        scan_decimal(overflow[i], overflow[i] + std::strlen(overflow[i]), number);
        double value;
        CHECK_MSG(!decimal_to_double(number, value), overflow[i]);
    }
    const char* underflow[] = {"1e-400", "-1e-400", "4.9e-325"};
    for (size_t i = 0; i < sizeof(underflow) / sizeof(underflow[0]); i++) {
        DecimalNumber number;
        scan_decimal(underflow[i], underflow[i] + std::strlen(underflow[i]), number);
        double value = 5;
        CHECK_MSG(decimal_to_double(number, value) && value == 0.0, underflow[i]);
        CHECK_MSG(same_double(value, std::strtod(underflow[i], 0)), underflow[i]);
    }
    // the largest finite double and the smallest denormal parse exactly
    DecimalNumber number;
    const char* max = "1.7976931348623157e308";
    scan_decimal(max, max + std::strlen(max), number);
    double value;
    CHECK(decimal_to_double(number, value) && same_double(value, std::strtod(max, 0)));
    const char* denormal = "4.9406564584124654e-324";
    scan_decimal(denormal, denormal + std::strlen(denormal), number);
    CHECK(decimal_to_double(number, value) && same_double(value, std::strtod(denormal, 0)) && value > 0);
    // float
    const char* float_overflow = "3.5e38";
    scan_decimal(float_overflow, float_overflow + std::strlen(float_overflow), number);
    float f;
    CHECK(!decimal_to_float(number, f));
    const char* float_max = "3.4028235e38";
    scan_decimal(float_max, float_max + std::strlen(float_max), number);
    CHECK(decimal_to_float(number, f) && same_float(f, std::strtof(float_max, 0)));
    const char* float_denormal = "1.4e-45";
    scan_decimal(float_denormal, float_denormal + std::strlen(float_denormal), number);
    CHECK(decimal_to_float(number, f) && same_float(f, std::strtof(float_denormal, 0)) && f > 0);
    const char* float_underflow = "1e-46";
    scan_decimal(float_underflow, float_underflow + std::strlen(float_underflow), number);
    CHECK(decimal_to_float(number, f) && f == 0.0f);
}

// Random decimal strings of every shape .osu files (and worse) contain, compared bit for bit
// with the C library.
static void random_number(char* buf, size_t& length, bool with_exponent) {
    length = 0;
    if (rng() % 3 == 0) {
        buf[length++] = '-';
    }
    const int int_digits = static_cast<int>(rng() % 26); // may be 0
    const int frac_digits = static_cast<int>(rng() % 22);
    const bool with_dot = frac_digits > 0 || rng() % 4 == 0;
    for (int i = 0; i < int_digits; i++) {
        buf[length++] = static_cast<char>('0' + rng() % 10);
    }
    if (int_digits == 0 && (!with_dot || frac_digits == 0)) {
        buf[length++] = static_cast<char>('0' + rng() % 10);
    }
    if (with_dot) {
        buf[length++] = '.';
        for (int i = 0; i < frac_digits; i++) {
            buf[length++] = static_cast<char>('0' + rng() % 10);
        }
    }
    if (with_exponent && rng() % 2 == 0) {
        buf[length++] = 'e';
        const int shape = static_cast<int>(rng() % 3);
        if (shape == 1) {
            buf[length++] = '-';
        } else if (shape == 2) {
            buf[length++] = '+';
        }
        const int exponent = static_cast<int>(rng() % 360);
        std::sprintf(buf + length, "%d", exponent);
        length += std::strlen(buf + length);
    }
    buf[length] = 0;
}

PPPP_TEST(random_doubles_match_strtod) {
    char buf[96];
    int compared = 0;
    for (int iter = 0; iter < 400000; iter++) {
        size_t length;
        random_number(buf, length, true);
        DecimalNumber number;
        const char* q = scan_decimal(buf, buf + length, number);
        errno = 0;
        char* next = 0;
        const double want = std::strtod(buf, &next);
        const bool want_overflow = errno == ERANGE && std::fabs(want) > 1.0;
        if (q == buf) {
            CHECK_MSG(next == buf, buf);
            continue;
        }
        double got = 0;
        const bool ok = decimal_to_double(number, got);
        if (q - buf != next - buf || ok == want_overflow || (ok && !same_double(got, want))) {
            std::printf("  mismatch: %s (consumed %d vs %d, ok=%d, got %.17g want %.17g)\n", buf,
                        (int)(q - buf), (int)(next - buf), ok ? 1 : 0, got, want);
            CHECK_MSG(false, "random double matches strtod");
            return;
        }
        compared++;
    }
    CHECK(compared > 300000);
}

PPPP_TEST(random_floats_match_strtof) {
    char buf[96];
    for (int iter = 0; iter < 400000; iter++) {
        size_t length;
        random_number(buf, length, true);
        // keep exponents in the float range most of the time
        DecimalNumber number;
        const char* q = scan_decimal(buf, buf + length, number);
        if (q == buf) {
            continue;
        }
        errno = 0;
        char* next = 0;
        const float want = std::strtof(buf, &next);
        const bool want_overflow = errno == ERANGE && std::fabs(want) > 1.0f;
        float got = 0;
        const bool ok = decimal_to_float(number, got);
        if (q - buf != next - buf || ok == want_overflow || (ok && !same_float(got, want))) {
            std::printf("  mismatch: %s (ok=%d, got %.9g want %.9g)\n", buf, ok ? 1 : 0, got, want);
            CHECK_MSG(false, "random float matches strtof");
            return;
        }
    }
    CHECK(true);
}

PPPP_TEST(long_significands) {
    const char* cases[] = {
        "342.857142857142857142857",
        "123456789012345678901",
        "0.6999999999999999556",
        "9007199254740993",
        "9007199254740993.0000000000000001",
        "2.2250738585072011e-308",
        "2.2250738585072012e-308",
        "1.00000000000000011102230246251565404236316680908203125",
        "1.000000000000000111022302462515654042363166809082031250000000000000000000000000000001"};
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char padded[256];
        std::memset(padded, 0, sizeof(padded));
        std::strcpy(padded, cases[i]);
        double got = 0;
        const char* q = parse_double(padded, padded + std::strlen(cases[i]), got);
        CHECK_MSG(q == padded + std::strlen(cases[i]), cases[i]);
        CHECK_MSG(same_double(got, std::strtod(cases[i], 0)), cases[i]);
        float gotf = 0;
        CHECK_MSG(parse_osu_float(padded, padded + std::strlen(cases[i]), gotf, 1e38f) != padded, cases[i]);
        CHECK_MSG(same_float(gotf, std::strtof(cases[i], 0)), cases[i]);
    }
}

PPPP_TEST(osu_number_acceptance) {
    char buf[64];
    std::memset(buf, 0, sizeof(buf));
    double d = 0;
    float f = 0;
    fosu_int64 i = 0;
    std::strcpy(buf, " 12.5 ,");
    CHECK(parse_osu_double(buf, buf + 7, d) == buf + 6 && d == 12.5); // surrounding spaces consumed
    std::strcpy(buf, "+5");
    CHECK(parse_osu_double(buf, buf + 2, d) == buf + 2 && d == 5.0);
    std::strcpy(buf, "++5");
    CHECK(parse_osu_double(buf, buf + 3, d) == buf);
    std::strcpy(buf, "+-5");
    CHECK(parse_osu_double(buf, buf + 3, d) == buf);
    std::strcpy(buf, "nan");
    CHECK(parse_osu_double(buf, buf + 3, d) == buf);
    std::strcpy(buf, "2147483648");
    CHECK(parse_osu_double(buf, buf + 10, d) == buf); // beyond the int32 limit
    CHECK(parse_osu_int(buf, buf + 10, i) == buf);
    std::strcpy(buf, "-2147483648");
    CHECK(parse_osu_int(buf, buf + 11, i) == buf); // symmetric osu! bound
    std::strcpy(buf, "2147483647");
    CHECK(parse_osu_int(buf, buf + 10, i) == buf + 10 && i == 2147483647);
    std::strcpy(buf, "131072.5");
    CHECK(parse_osu_float(buf, buf + 8, f, 131072.0f) == buf);
    std::strcpy(buf, "131072");
    CHECK(parse_osu_float(buf, buf + 6, f, 131072.0f) == buf + 6 && f == 131072.0f);
    std::strcpy(buf, "1e-50");
    CHECK(parse_osu_float(buf, buf + 5, f) == buf + 5 && f == 0.0f); // underflow to zero accepted
    std::strcpy(buf, "-1e-400");
    CHECK(parse_osu_double(buf, buf + 7, d) == buf + 7 && same_double(d, -0.0));
    std::strcpy(buf, "1e400");
    CHECK(parse_osu_double(buf, buf + 5, d) == buf);
    std::strcpy(buf, "0.7");
    CHECK(parse_osu_float(buf, buf + 3, f) == buf + 3 && f == 0.7f);
}

PPPP_TEST_MAIN()
