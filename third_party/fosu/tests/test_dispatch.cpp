#include "test_support.h"

#include <fosu/engine/hit_objects/common_fields.h>
#include <fosu/engine/parsing_engine.h>
#include <fosu/engine/runtime/cpu_features.h>
#include <fosu/engine/runtime/loader.h>
#include <fosu/runtime.h>

#include <cstdlib>
#include <stdlib.h> // IWYU pragma: keep (setenv and _putenv_s)
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <pthread.h>
#endif

static void set_backend(const char* value) {
#if defined(_WIN32)
    _putenv_s("FOSU_BACKEND", value);
#else
    setenv("FOSU_BACKEND", value, 1);
#endif
}

namespace {
    const int kDispatchThreadCount = 8;

    struct DispatchSelection {
        const fosu::ParsingEngine* engine;
        bool parsed;
        bool one_hit_object;
        float x;
    };

    void dispatch_selection(DispatchSelection& out) {
        out.engine = fosu::runtime_engine();
        out.parsed = false;
        out.one_hit_object = false;
        out.x = 0.0f;
        if (!out.engine) {
            return;
        }
        fosu::Parser parser;
        const char input[] = "[Metadata]\nTitle:dispatch\n[HitObjects]\n1,2,3,1,0\n";
        fosu::Beatmap* result = parser.parse(input, sizeof(input) - 1);
        out.parsed = !!result;
        if (result) {
            out.one_hit_object = result->hit_objects.size() == 1;
            if (out.one_hit_object) {
                out.x = result->hit_objects[0].x;
            }
        }
    }

#if defined(_WIN32)
    DWORD WINAPI dispatch_thread_entry(LPVOID argument) {
        dispatch_selection(*static_cast<DispatchSelection*>(argument));
        return 0;
    }
#else
    void* dispatch_thread_entry(void* argument) {
        dispatch_selection(*static_cast<DispatchSelection*>(argument));
        return 0;
    }
#endif
} // namespace

TEST_CASE("the atomic flag spins and clears") {
    nonstd::atomic_flag f;
    CHECK(f.test_and_set() == false);
    CHECK(f.test_and_set() == true);
    f.clear();
    CHECK(f.test_and_set() == false);
}

// Backend-independent surface of the ISA primitives. In the scalar build the vector engines are
// compiled out, so `vector_ops.h` contributes only the dispatch macros, `digit_groups.h` is empty,
// and `common_fields.h` leaves the empty HitObjectParseConstants. The vector-path equivalence is
// proved by Tasks 9/10 through FOSU_BACKEND=avx2.
TEST_CASE("the scalar surface of the ISA primitives") {
    CHECK_UNARY(FOSU_SIMD == 0);
    CHECK_UNARY(FOSU_SIMD_X86 == 0);
    CHECK_UNARY(FOSU_SIMD_NEON == 0);
    CHECK(fosu::internal::kNPrefixVariants == 3 * 3 * 10 * 3);
    fosu::internal::HitObjectParseConstants constants;
    (void)constants;
}

// Every required bit of leaf1/leaf7/extended is load-bearing, and AVX2 needs the OS to save both
// the XMM and the YMM state (xcr0 bits 1 and 2).
TEST_CASE("the AVX2 feature requirements") {
    using namespace fosu_dispatch;
    CpuFeatures full;
    full.leaf1 = required_leaf1;
    full.leaf7 = required_leaf7;
    full.extended = required_extended;
    full.xcr0 = 6;
    CHECK(supports_avx2(full));
    for (unsigned bit = 0; bit < 32; ++bit) {
        CpuFeatures f = full;
        f.leaf1 &= ~(1u << bit);
        CHECK(supports_avx2(f) == !(required_leaf1 & (1u << bit)));
        f = full;
        f.leaf7 &= ~(1u << bit);
        CHECK(supports_avx2(f) == !(required_leaf7 & (1u << bit)));
        f = full;
        f.extended &= ~(1u << bit);
        CHECK(supports_avx2(f) == !(required_extended & (1u << bit)));
    }
    for (unsigned x = 0; x < 8; ++x) {
        CpuFeatures f = full;
        f.xcr0 = x;
        CHECK(supports_avx2(f) == ((x & 6) == 6));
    }
}

// No engine library is built in this configuration, so the embedded scalar engine is the one the
// runtime selects, whatever FOSU_BACKEND asks for.
TEST_CASE("the runtime engine falls back to the embedded scalar") {
    const fosu::ParsingEngine* engine = fosu::runtime_engine();
    CHECK_UNARY(engine != 0);
    CHECK(engine->kind == fosu::EngineKind::Scalar);
    CHECK(fosu::internal::selected_engine() == engine);
}

TEST_CASE("engine kinds resolve by name") {
    const fosu::EngineKind::Value* scalar = fosu::internal::find_engine_kind("scalar");
    CHECK_UNARY(scalar != 0);
    CHECK(*scalar == fosu::EngineKind::Scalar);
    const fosu::EngineKind::Value* avx2 = fosu::internal::find_engine_kind("avx2");
    CHECK_UNARY(avx2 != 0);
    CHECK(*avx2 == fosu::EngineKind::Avx2);
    const fosu::EngineKind::Value* neon = fosu::internal::find_engine_kind("neon");
    CHECK_UNARY(neon != 0);
    CHECK(*neon == fosu::EngineKind::Neon);
    CHECK(nonstd::string_view(fosu::internal::engine_name(fosu::EngineKind::Scalar)) ==
          nonstd::string_view("scalar"));
    CHECK(nonstd::string_view(fosu::internal::engine_name(fosu::EngineKind::Avx2)) ==
          nonstd::string_view("avx2"));
    CHECK(nonstd::string_view(fosu::internal::engine_name(fosu::EngineKind::Neon)) ==
          nonstd::string_view("neon"));
    CHECK(fosu::internal::engine_available(fosu::EngineKind::Scalar));
    CHECK_UNARY(fosu::internal::find_engine_kind("does-not-exist") == 0);
}

// A Parser with no explicit engine routes its parse through the runtime's selection.
TEST_CASE("the parser parses through the dispatcher") {
    const fosu::Beatmap& beatmap =
        parse_str("osu file format v14\n\n[General]\nAudioFilename: a.mp3\nMode:0\n");
    CHECK(beatmap.format_version == 14);
    CHECK_UNARY(beatmap.mode == 0);
}

// Race the first selection and parse: unload forces the next runtime_engine() to select again, so
// the double-checked flag in the loader is contended. Every thread must retain one engine and parse
// the same input through it.
TEST_CASE("concurrent first selection keeps one engine") {
    fosu::internal::unload_engine();
    DispatchSelection selections[kDispatchThreadCount] = {};
#if defined(_WIN32)
    HANDLE threads[kDispatchThreadCount];
    bool started[kDispatchThreadCount];
    for (int i = 0; i < kDispatchThreadCount; ++i) {
        threads[i] = CreateThread(0, 0, dispatch_thread_entry, &selections[i], 0, 0);
        started[i] = threads[i] != 0;
        CHECK(started[i]);
    }
    for (int i = 0; i < kDispatchThreadCount; ++i) {
        if (started[i]) {
            WaitForSingleObject(threads[i], INFINITE);
            CloseHandle(threads[i]);
        }
    }
#else
    pthread_t threads[kDispatchThreadCount];
    bool started[kDispatchThreadCount];
    for (int i = 0; i < kDispatchThreadCount; ++i) {
        started[i] = pthread_create(&threads[i], 0, dispatch_thread_entry, &selections[i]) == 0;
        CHECK(started[i]);
    }
    for (int i = 0; i < kDispatchThreadCount; ++i) {
        if (started[i]) {
            pthread_join(threads[i], 0);
        }
    }
#endif
    for (int i = 0; i < kDispatchThreadCount; ++i) {
        if (!started[i]) {
            continue;
        }
        CHECK(!!selections[i].engine);
        CHECK(selections[i].engine == selections[0].engine);
        CHECK(selections[i].parsed);
        CHECK(selections[i].one_hit_object);
        CHECK(selections[i].x == 1.0f);
    }
}

// An unrecognised FOSU_BACKEND must not disturb the engine the runtime already selected.
TEST_CASE("an unknown backend keeps the selected engine") {
    const fosu::ParsingEngine* before = fosu::runtime_engine();
    set_backend("unknown");
    CHECK(before == fosu::runtime_engine());
}
