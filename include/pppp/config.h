#ifndef PPPP_CONFIG_H
#define PPPP_CONFIG_H

#include <algorithm> // IWYU pragma: keep

#define optional_CONFIG_NO_EXCEPTIONS 1
#include "nonstd/optional.hpp" // IWYU pragma: keep

#define PPPP_VERSION_MAJOR 0
#define PPPP_VERSION_MINOR 0
#define PPPP_VERSION_PATCH 0

#if defined(_MSC_VER) && _MSC_VER < 1310
typedef __int64 pppp_int64;
typedef unsigned __int64 pppp_uint64;
#elif defined(__GNUC__)
__extension__ typedef long long pppp_int64;
__extension__ typedef unsigned long long pppp_uint64;
#else
typedef long long pppp_int64;
typedef unsigned long long pppp_uint64;
#endif

#define PPPP_STATIC_ASSERT(name, cond) typedef char pppp_static_assert_##name[(cond) ? 1 : -1]

PPPP_STATIC_ASSERT(int64_is_64_bits, sizeof(pppp_int64) == 8);
PPPP_STATIC_ASSERT(uint64_is_64_bits, sizeof(pppp_uint64) == 8);
PPPP_STATIC_ASSERT(int_is_at_least_32_bits, sizeof(int) >= 4);

namespace pppp {
    struct Result {
        enum Value { OK = 0, INVALID_ARGUMENT, ALLOCATION, PARSE, NO_SLIDER_PATH_BACKEND, SLIDER_PATH };
    };
} // namespace pppp

#endif
