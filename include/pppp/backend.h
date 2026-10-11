#ifndef PPPP_BACKEND_H
#define PPPP_BACKEND_H

namespace pppp {
    /// The parsing engine the library loaded.
    struct EngineKind {
        enum Value { SCALAR = 0, AVX2, NEON };
    };

    /// The parsing engine in use: `SCALAR`, `AVX2` or `NEON`. The engine is chosen once, on first use
    /// (a parse, or this call), from the build's `PPPP_ISA` and the `FOSU_BACKEND` override; this only
    /// reports it.
    EngineKind::Value backend();
}

#endif
