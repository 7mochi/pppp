/* The C install test program. It includes the public header as plain C, loads a beatmap from a
   string, runs the osu! difficulty calculator, and checks the version. A broken header, a missing
   pppp_c package, or a missing parser engine makes the test fail. */
#include <pppp/capi.h>
#include <stdio.h>

int main(void) {
    if (Pppp_Version() == 0) {
        return 1;
    }
    if (Pppp_VersionNumber() != PPPP_VERSION_NUMBER) {
        return 2;
    }
    puts(Pppp_Version());
    return 0;
}
