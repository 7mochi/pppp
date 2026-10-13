#include "pppp/backend.h"

#include <cstdlib>
#include <cstring>
#include <doctest.h>
#include <fosu/engine/runtime/loader.h>

namespace internal = ::fosu::internal;

TEST_CASE("backend reports the selected engine") {
    const pppp::EngineKind::Value kind = pppp::backend();
    CHECK(pppp::backend() == kind);

    // backend() must agree with the loader's own name for the engine it selected.
    const char* expected = internal::engine_name(internal::selected_engine()->kind);
    const char* actual = kind == pppp::EngineKind::AVX2   ? "avx2"
                         : kind == pppp::EngineKind::NEON ? "neon"
                                                          : "scalar";
    CHECK(std::strcmp(actual, expected) == 0);

    const char* override = std::getenv("FOSU_BACKEND");
    if (override != 0 && std::strcmp(override, "scalar") == 0) {
        CHECK(kind == pppp::EngineKind::SCALAR);
    }
}
