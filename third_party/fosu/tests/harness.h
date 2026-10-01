// Minimal C++98 test harness.
//
//   PPPP_TEST(name) { CHECK(cond); CHECK_NEAR(a, b, eps, "msg"); ... }
//   PPPP_TEST_MAIN()
//
// The optional first argument of a suite executable is the resources directory
// (tests/fixtures by default); pppp_test::resource("x.osu") builds a path in it.
#ifndef FOSU_TESTS_HARNESS_H
#define FOSU_TESTS_HARNESS_H

#include <cmath> // IWYU pragma: keep
#include <cstdio>

namespace pppp_test {

    struct Case {
        const char* name;
        void (*run)();
        Case* next;
    };

    inline Case*& first_case() {
        static Case* head = 0;
        return head;
    }

    struct Registrar {
        explicit Registrar(Case* c) {
            c->next = 0;
            Case** slot = &first_case();
            while (*slot) {
                slot = &(*slot)->next;
            }
            *slot = c;
        }
    };

    inline int& checks() {
        static int n = 0;
        return n;
    }

    inline int& failures() {
        static int n = 0;
        return n;
    }

    inline const char*& resources_dir() {
        static const char* dir = "tests/fixtures";
        return dir;
    }

    // Path of a file under the resources directory (static buffer: copy it if you need two).
    inline const char* resource(const char* name) {
        static char buf[1024];
        std::sprintf(buf, "%s/%s", resources_dir(), name);
        return buf;
    }

    inline void fail(const char* file, int line, const char* what) {
        failures()++;
        std::printf("  FAIL %s:%d: %s\n", file, line, what);
    }

    inline void fail_near(const char* file, int line, const char* what, double got, double expected) {
        failures()++;
        std::printf("  FAIL %s:%d: %s (got %.17g, expected %.17g)\n", file, line, what, got, expected);
    }

    inline int run_all(int argc, char** argv) {
        if (argc >= 2) {
            resources_dir() = argv[1];
        }
        for (Case* c = first_case(); c; c = c->next) {
            std::printf("[ %s ]\n", c->name);
            c->run();
        }
        std::printf("\n%d checks, %d failures\n", checks(), failures());
        return failures() == 0 ? 0 : 1;
    }

} // namespace pppp_test

#define PPPP_TEST(name)                                                                                      \
    static void name();                                                                                      \
    static pppp_test::Case name##_case = {#name, name, 0};                                                   \
    static pppp_test::Registrar name##_registrar(&name##_case);                                              \
    static void name()

#define CHECK(cond)                                                                                          \
    do {                                                                                                     \
        pppp_test::checks()++;                                                                               \
        if (!(cond)) pppp_test::fail(__FILE__, __LINE__, #cond);                                             \
    } while (0)

#define CHECK_MSG(cond, msg)                                                                                 \
    do {                                                                                                     \
        pppp_test::checks()++;                                                                               \
        if (!(cond)) pppp_test::fail(__FILE__, __LINE__, msg);                                               \
    } while (0)

// absolute tolerance
#define CHECK_NEAR(a, b, eps, msg)                                                                           \
    do {                                                                                                     \
        pppp_test::checks()++;                                                                               \
        double pppp_test_a = (a), pppp_test_b = (b);                                                         \
        if (!(std::fabs(pppp_test_a - pppp_test_b) <= (eps)))                                                \
            pppp_test::fail_near(__FILE__, __LINE__, msg, pppp_test_a, pppp_test_b);                         \
    } while (0)

// relative tolerance (|a - b| <= tol * max(1, |b|))
#define CHECK_REL(a, b, tol, msg)                                                                            \
    do {                                                                                                     \
        pppp_test::checks()++;                                                                               \
        double pppp_test_a = (a), pppp_test_b = (b);                                                         \
        double pppp_test_scale = std::fabs(pppp_test_b) > 1.0 ? std::fabs(pppp_test_b) : 1.0;                \
        if (!(std::fabs(pppp_test_a - pppp_test_b) <= (tol) * pppp_test_scale))                              \
            pppp_test::fail_near(__FILE__, __LINE__, msg, pppp_test_a, pppp_test_b);                         \
    } while (0)

#define PPPP_TEST_MAIN()                                                                                     \
    int main(int argc, char** argv) { return pppp_test::run_all(argc, argv); }

#endif
