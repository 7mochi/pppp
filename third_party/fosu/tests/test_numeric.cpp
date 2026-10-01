// parse_double against libc, integer parse_osu_float against an exact reference, and the
// Bézier subdivision against the general one
// (upstream tests/test_numeric.cc without its SIMD cases; tests/test_decimal.cpp covers the
// conversion itself).
#include "harness.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/slider_geometry.h>
#include <string>

using namespace fosu;
using namespace fosu::internal;

// Independent numeric oracle: libc conversion over a bounded copy.
static const char* reference_parse_double(const char* p, const char* end, double& out) {
    std::string bounded(p, end);
    char* next;
    out = std::strtod(bounded.c_str(), &next);
    const bool finite = out == out && out <= 1.7976931348623157e308 && out >= -1.7976931348623157e308;
    return finite ? p + (next - bounded.c_str()) : p;
}

static fosu_uint64 rng_state = (static_cast<fosu_uint64>(0x243F6A88u) << 32) | 0x85A308D3u;
static fosu_uint64 rng() {
    // splitmix64
    rng_state += (static_cast<fosu_uint64>(0x9E3779B9u) << 32) | 0x7F4A7C15u;
    fosu_uint64 z = rng_state;
    z = (z ^ (z >> 30)) * ((static_cast<fosu_uint64>(0xBF58476Du) << 32) | 0x1CE4E5B9u);
    z = (z ^ (z >> 27)) * ((static_cast<fosu_uint64>(0x94D049BBu) << 32) | 0x133111EBu);
    return z ^ (z >> 31);
}

PPPP_TEST(fuzz_parse_double) {
    char buf[96];
    int failures = 0;
    for (int iter = 0; iter < 300000 && !failures; ++iter) {
        const int int_digits = 1 + static_cast<int>(rng() % 9);
        const int frac_digits = static_cast<int>(rng() % 8);
        int len = 0;
        if (rng() % 3 == 0) {
            buf[len++] = '-';
        }
        for (int i = 0; i < int_digits; ++i) {
            buf[len++] = static_cast<char>('0' + (i == 0 ? rng() % 9 + (int_digits > 1) : rng() % 10));
        }
        if (frac_digits || rng() % 4 == 0) {
            buf[len++] = '.';
            for (int i = 0; i < frac_digits; ++i) {
                buf[len++] = static_cast<char>('0' + rng() % 10);
            }
        }
        const char* tail = ",4,2\r\n";
        const int payload = len;
        for (const char* t = tail; *t; ++t) {
            buf[len++] = *t;
        }
        std::memset(buf + len, 0, sizeof(buf) - static_cast<size_t>(len));

        double got = -1, want = -2;
        const char* gp = parse_double(buf, buf + payload, got);
        const char* wp = reference_parse_double(buf, buf + payload, want);
        if (gp != wp || std::memcmp(&got, &want, sizeof(got)) != 0) {
            std::printf("  failing double: %.*s\n", payload, buf);
            failures++;
        }
    }
    CHECK(failures == 0);
    // Long significands must agree with the independent libc result.
    static const char* const long_cases[] = {"342.857142857142857142857", "123456789012345678901",
                                             "0.6999999999999999556"};
    for (size_t i = 0; i < 3; i++) {
        std::string padded(long_cases[i]);
        padded.append(64, '\0');
        double got = 0;
        parse_double(padded.data(), padded.data() + std::strlen(long_cases[i]), got);
        CHECK_MSG(got == std::strtod(long_cases[i], 0), long_cases[i]);
    }
}

static float random_coordinate() {
    const fosu_uint32 bits =
        static_cast<fosu_uint32>(rng() & 0x807fffffu) | (static_cast<fosu_uint32>(126 + rng() % 10) << 23);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

// parse_osu_float on integer spellings. An integer of at most ten digits is exact in a double,
// so narrowing that double to float is the single correctly rounded conversion strtof would do.
PPPP_TEST(fuzz_parse_osu_float_integers) {
    char buf[96];
    for (int iter = 0; iter < 100000; ++iter) {
        int len = 0;
        if (rng() % 4 == 0) {
            buf[len++] = ' ';
        }
        bool negative = false;
        if (rng() % 3 == 0) {
            negative = rng() % 2 != 0;
            buf[len++] = negative ? '-' : '+';
        }
        const int digits = 1 + static_cast<int>(rng() % 10);
        double magnitude = 0;
        for (int i = 0; i < digits; ++i) {
            const int digit = static_cast<int>(rng() % 10);
            buf[len++] = static_cast<char>('0' + digit);
            magnitude = magnitude * 10 + digit;
        }
        if (rng() % 4 == 0) {
            buf[len++] = ' ';
        }
        const int comma = len;
        buf[len++] = ',';
        std::memset(buf + len, 0, sizeof(buf) - static_cast<size_t>(len));
        float got = 0;
        const char* next = parse_osu_float(buf, buf + len, got);
        const float want = static_cast<float>(negative ? -magnitude : magnitude);
        const bool valid = (want < 0 ? -want : want) <= static_cast<float>(2147483647);
        const std::string text(buf, static_cast<size_t>(len));
        CHECK_MSG(next - buf == (valid ? comma : 0), text.c_str());
        if (valid) {
            CHECK_MSG(std::memcmp(&got, &want, sizeof(float)) == 0, text.c_str());
        }
    }
}

PPPP_TEST(bezier_subdivision) {
    int mismatches = 0;
    for (size_t count = 4; count <= 10; ++count) {
        for (int sample = 0; sample < 1000; ++sample) {
            CurvePoint points[10];
            for (size_t i = 0; i < count; i++) {
                points[i] = CurvePoint::make(random_coordinate(), random_coordinate());
            }
            CurvePoint left[10], right[10], midpoints[10];
            subdivide_bezier(Span<const CurvePoint>(points, count), left, right, midpoints);

            CurvePoint expected_left[10], expected_right[10], work[10];
            std::copy(points, points + count, work);
            for (size_t i = 0; i < count; ++i) {
                expected_left[i] = work[0];
                expected_right[count - i - 1] = work[count - i - 1];
                for (size_t j = 0; j < count - i - 1; ++j) {
                    work[j] = (work[j] + work[j + 1]) * 0.5f;
                }
            }
            if (std::memcmp(left, expected_left, count * sizeof(CurvePoint)) != 0 ||
                std::memcmp(right, expected_right, count * sizeof(CurvePoint)) != 0) {
                mismatches++;
            }
        }
    }
    CHECK(mismatches == 0);
}

PPPP_TEST_MAIN()
