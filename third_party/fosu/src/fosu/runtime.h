#ifndef FOSU_RUNTIME_H
#define FOSU_RUNTIME_H

#include <fosu/compiler.h>
#include <fosu/engine/parsing_engine.h>

namespace fosu {
    // Select once from FOSU_BACKEND (auto/scalar/avx2/neon). The embedded scalar
    // engine is the fallback when an explicitly requested engine is unavailable.
    // Valid for the library lifetime.
    FOSU_EXPORT const ParsingEngine* runtime_engine();
} // namespace fosu

#endif
