#ifndef FOSU_TESTS_HARNESS_H
#define FOSU_TESTS_HARNESS_H

#include <doctest.h>

#define PPPP_TEST(name) TEST_CASE(#name)

#undef CHECK
#define CHECK(cond) CHECK_UNARY(cond)

#define CHECK_MSG(cond, msg)                                                                          \
    do {                                                                                              \
        const char* pppp_test_msg = (msg);                                                            \
        INFO(pppp_test_msg);                                                                          \
        CHECK_UNARY(cond);                                                                            \
    } while (0)

#define PPPP_TEST_MAIN()

#endif
