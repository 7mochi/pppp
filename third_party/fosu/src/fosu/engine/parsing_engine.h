#ifndef FOSU_ENGINE_PARSING_ENGINE_H
#define FOSU_ENGINE_PARSING_ENGINE_H

#include <fosu/beatmap.h>
#include <fosu/parse_options.h>
#include <fosu/types.h>

#include <cstddef>

namespace fosu {
    struct EngineKind {
        enum Value { Scalar, Avx2, Neon };
    };

    // Private core/engine ABI. Input excludes its readable zero padding.
    // Beatmap spans initially describe writable capacity; the engine fills them
    // and shortens them to actual counts. No allocation, I/O, or retained state.
    struct ParsingEngine {
        EngineKind::Value kind;
        ParseError (*parse_document)(const char* data, size_t size, Beatmap& beatmap, ParseOptions options);
    };
} // namespace fosu

#endif
