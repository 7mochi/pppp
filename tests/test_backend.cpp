#include "pppp/backend.h"

#include <cstdlib>
#include <cstring>
#include <doctest.h>
#include <fosu/engine/runtime/loader.h>

TEST_CASE("backend reports the selected engine") {
    const pppp::EngineKind::Value kind = pppp::backend();
    CHECK(pppp::backend() == kind);

    // backend() must agree with the loader's own name for the engine it selected.
    const char* expected = fosu::internal::engine_name(fosu::internal::selected_engine()->kind);
    const char* actual = kind == pppp::EngineKind::AVX2   ? "avx2"
                         : kind == pppp::EngineKind::NEON ? "neon"
                                                          : "scalar";
    CHECK(std::strcmp(actual, expected) == 0);

    const char* override = std::getenv("FOSU_BACKEND");
    if (override != 0 && std::strcmp(override, "scalar") == 0) {
        CHECK(kind == pppp::EngineKind::SCALAR);
    }
}
