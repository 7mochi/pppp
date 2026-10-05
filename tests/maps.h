#ifndef PPPP_TESTS_MAPS_H
#define PPPP_TESTS_MAPS_H

#include "pppp/beatmap.h"
#include <doctest.h>

namespace pppp_test {
    inline void load(pppp::Beatmap& beatmap, const char* path) {
        INFO(path);
        REQUIRE(beatmap.load_file(path).ok());
    }
} // namespace pppp_test

#endif
