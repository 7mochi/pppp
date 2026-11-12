#include <pppp/capi.h>

int main(void) {
    if (Pppp_Version() == 0) {
        return 1;
    }
    if (Pppp_VersionNumber() != PPPP_VERSION_NUMBER) {
        return 2;
    }
    return 0;
}
