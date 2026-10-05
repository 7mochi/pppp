#ifndef PPPP_BEATMAP_H
#define PPPP_BEATMAP_H

#include "pppp/beatmaps/beatmap.h"
#include "pppp/status.h"
#include <cstddef>

namespace fosu {
    struct Beatmap;
} // namespace fosu

namespace pppp {
    class Beatmap : public pppp::beatmaps::Beatmap {
    public:
        Status load_file(const char* path);
        Status load_buffer(const void* data, size_t size);
    };
} // namespace pppp

namespace pppp { namespace beatmaps {
    /// Fill `out` from a beatmap parsed by the vendored parser. `parsed`, and whatever owns it,
    /// must outlive `out`: the slider paths are recomputed from it.
    Status from_parsed(Beatmap& out, const ::fosu::Beatmap* parsed);
}} // namespace pppp::beatmaps

#endif
