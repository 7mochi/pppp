#include <pppp/backend.h>

#include <cstdio>

int main() {
    switch (pppp::backend()) {
    case pppp::EngineKind::SCALAR: std::puts("scalar"); break;
    case pppp::EngineKind::AVX2: std::puts("avx2"); break;
    case pppp::EngineKind::NEON: std::puts("neon"); break;
    }
    return 0;
}
