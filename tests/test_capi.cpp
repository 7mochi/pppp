#include <doctest.h>
#include <pppp/capi.h>
#include <string>

TEST_CASE("the C ABI reports the header version") {
    CHECK(std::string(Pppp_Version()) == PPPP_VERSION_STRING);
    CHECK(Pppp_VersionNumber() == PPPP_VERSION_NUMBER);
    CHECK(Pppp_VersionNumber() / 1000000 == PPPP_VERSION_MAJOR);
}

TEST_CASE("the C ABI reports a backend name") {
    const std::string backend(Pppp_Backend());
    CHECK((backend == "scalar" || backend == "avx2" || backend == "neon"));
}
