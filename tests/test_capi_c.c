#include <pppp/capi.h>
#include <string.h>

int main(void) {
    const char* backend;

    if (Pppp_Version() == 0) {
        return 1;
    }
    if (Pppp_VersionNumber() != PPPP_VERSION_NUMBER) {
        return 2;
    }

    backend = Pppp_Backend();
    if (backend == 0) {
        return 3;
    }
    if (strcmp(backend, "scalar") != 0 && strcmp(backend, "avx2") != 0 && strcmp(backend, "neon") != 0) {
        return 4;
    }
    return 0;
}
