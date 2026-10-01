#ifndef PPPP_TESTS_MAPS_H
#define PPPP_TESTS_MAPS_H

#include "pppp/beatmap.h"
#include "pppp/beatmaps/beatmap.h"
#include <doctest.h>

namespace pppp_test {
    inline void load(pppp::beatmaps::Beatmap& beatmap, const char* path) {
        INFO(path);
        REQUIRE(pppp::beatmaps::from_file(beatmap, path) == pppp::Result::OK);
    }
} // namespace pppp_test

#endif
