#ifndef PPPP_BEATMAP_H
#define PPPP_BEATMAP_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/config.h"
#include <cstddef>

namespace fosu {
    struct Beatmap;
} // namespace fosu

namespace pppp { namespace beatmaps {
    /// Fill `out` from a `.osu` file. The beatmap owns everything it needs afterwards.
    Result::Value from_file(Beatmap& out, const char* path);

    /// Fill `out` from a `.osu` document already in memory. The beats' slider paths come from it.
    Result::Value from_bytes(Beatmap& out, const void* data, size_t size);

    /// Fill `out` from a beatmap parsed by the vendored parser. `parsed`, and whatever owns it,
    /// must outlive `out`: the slider paths are recomputed from it.
    Result::Value from_parsed(Beatmap& out, const ::fosu::Beatmap* parsed);
}} // namespace pppp::beatmaps

#endif
