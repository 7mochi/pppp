#include "pppp/capi.h"
#include "pppp/backend.h"

const char* Pppp_Version(void) { return PPPP_VERSION_STRING; }

PpppUint32 Pppp_VersionNumber(void) { return PPPP_VERSION_NUMBER; }

const char* Pppp_Backend(void) {
    switch (pppp::backend()) {
    case pppp::EngineKind::SCALAR: return "scalar";
    case pppp::EngineKind::AVX2: return "avx2";
    case pppp::EngineKind::NEON: return "neon";
    }

    return "scalar";
}
