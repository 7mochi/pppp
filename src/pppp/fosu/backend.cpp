#include "pppp/backend.h"

#include <fosu/engine/runtime/loader.h>

namespace pppp {
    EngineKind::Value backend() {
        switch (fosu::internal::selected_engine()->kind) {
        case ::fosu::EngineKind::Scalar: return EngineKind::SCALAR;
        case ::fosu::EngineKind::Avx2: return EngineKind::AVX2;
        case ::fosu::EngineKind::Neon: return EngineKind::NEON;
        }
        return EngineKind::SCALAR;
    }
} // namespace pppp
