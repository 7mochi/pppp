#include <fosu/engine/parse_document.h>
#include <fosu/engine/parsing_engine.h>
#include <fosu/engine/primitives/vector_ops.h>

namespace fosu { namespace internal {
    // The ISA this code was compiled for, not a runtime choice based on the host CPU. One module
    // defines one engine: the scalar core this file, a SIMD engine library its own copy under
    // FOSU_COMPILE_AVX2/FOSU_DISABLE_SIMD.
    const ParsingEngine compiled_engine = {
#if FOSU_SIMD_X86
        EngineKind::Avx2,
#elif FOSU_SIMD_NEON
        EngineKind::Neon,
#else
        EngineKind::Scalar,
#endif
        parse_document};
}} // namespace fosu::internal
