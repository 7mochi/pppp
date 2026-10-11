#include <fosu/engine/parse_document.h>
#include <fosu/engine/parsing/string_lookup.h>
#include <fosu/engine/parsing_engine.h>
#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/engine/runtime/cpu_features.h>
#include <fosu/engine/runtime/loader.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cwchar>
#include <windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#endif

namespace fosu { namespace internal {
#if FOSU_SIMD
#error "the core embeds only the scalar engine"
#endif
    namespace {
        // Trivial lifetime permits a host's late exit callback to initialize again
        // after library cleanup. No parser or arena lives in an engine library.
        const ParsingEngine& embedded_scalar_engine = compiled_engine;
        const ParsingEngine* selected = 0;
        nonstd::atomic_flag selecting;
#if defined(_WIN32)
        typedef HMODULE EngineLibrary;
#else
        typedef void* EngineLibrary;
#endif
        EngineLibrary library = 0;
        const StringEntry<EngineKind::Value> engine_kind_entries[] = {
            {"scalar", EngineKind::Scalar},
            {"avx2", EngineKind::Avx2},
            {"neon", EngineKind::Neon},
        };
        const StringLookup<EngineKind::Value> kEngineKinds(engine_kind_entries, 3);

#if defined(_WIN32)
        const size_t kPathCapacity = 32768;
        int module_anchor;

        bool engine_path(const char* name, wchar_t (&path)[kPathCapacity]) {
            HMODULE module;
            if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                    reinterpret_cast<const wchar_t*>(&module_anchor), &module)) {
                return false;
            }
            const DWORD length = GetModuleFileNameW(module, path, kPathCapacity);
            if (!length || length >= kPathCapacity) {
                return false;
            }
            wchar_t* slash = std::wcsrchr(path, L'\\');
            if (!slash) {
                return false;
            }
            const wchar_t* filename =
                std::strcmp(name, "avx2") == 0 ? L"fosu_engine_avx2.dll" : L"fosu_engine_neon.dll";
            const size_t directory_length = static_cast<size_t>(slash - path) + 1;
            const size_t filename_length = std::wcslen(filename) + 1;
            if (directory_length + filename_length > kPathCapacity) {
                return false;
            }
            std::wmemcpy(path + directory_length, filename, filename_length);
            return true;
        }
#else
        bool engine_path(const char* name, char (&path)[PATH_MAX]) {
            Dl_info info = {};
            if (!dladdr(reinterpret_cast<const void*>(&engine_path), &info)) {
                return false;
            }
            const char* slash = std::strrchr(info.dli_fname, '/');
            if (!slash) {
                return false;
            }
            const int length = std::snprintf(path, sizeof(path), "%.*s/libfosu_engine_%s" FOSU_ENGINE_SUFFIX,
                                             static_cast<int>(slash - info.dli_fname), info.dli_fname, name);
            return length > 0 && static_cast<size_t>(length) < sizeof(path);
        }
#endif

        const ParsingEngine* load_engine_library(const char* name) {
#if defined(_WIN32)
            wchar_t path[kPathCapacity];
            if (!engine_path(name, path)) {
                return 0;
            }
            HMODULE loaded = LoadLibraryW(path);
            if (!loaded) {
                return 0;
            }
            const ParsingEngine* (*entry)() =
                reinterpret_cast<const ParsingEngine* (*)()>(GetProcAddress(loaded, "fosu_engine_v2"));
            if (!entry) {
                FreeLibrary(loaded);
                return 0;
            }
#else
            char path[PATH_MAX];
            if (!engine_path(name, path)) {
                return 0;
            }
            void* loaded = dlopen(path, RTLD_NOW | RTLD_LOCAL);
            if (!loaded) {
                return 0;
            }
            const ParsingEngine* (*entry)() =
                reinterpret_cast<const ParsingEngine* (*)()>(dlsym(loaded, "fosu_engine_v2"));
            if (!entry) {
                dlclose(loaded);
                return 0;
            }
#endif
            library = loaded;
            return entry();
        }

        const ParsingEngine* select_engine_from_environment() {
#if defined(_WIN32)
            char request_buffer[16];
            const DWORD request_length =
                GetEnvironmentVariableA("FOSU_BACKEND", request_buffer, sizeof(request_buffer));
            if (request_length >= sizeof(request_buffer)) {
                return &embedded_scalar_engine;
            }
            const char* request = request_length ? request_buffer : FOSU_DEFAULT_BACKEND;
#else
            const char* request = std::getenv("FOSU_BACKEND");
            if (!request) {
                request = FOSU_DEFAULT_BACKEND;
            }
#endif
            if (std::strcmp(request, "auto") == 0) {
                const EngineKind::Value kinds[2] = {EngineKind::Avx2, EngineKind::Neon};
                for (size_t i = 0; i < 2; i++) {
                    if (engine_available(kinds[i])) {
                        const ParsingEngine* engine = load_engine_library(engine_name(kinds[i]));
                        if (engine) {
                            return engine;
                        }
                    }
                }
                return &embedded_scalar_engine;
            }
            const EngineKind::Value* kind = find_engine_kind(request);
            if (!kind || *kind == EngineKind::Scalar) {
                return &embedded_scalar_engine;
            }
            if (engine_available(*kind)) {
                const ParsingEngine* engine = load_engine_library(request);
                if (engine) {
                    return engine;
                }
            }
            return &embedded_scalar_engine;
        }
    } // namespace

    const EngineKind::Value* find_engine_kind(nonstd::string_view name) { return kEngineKinds.find(name); }

    const char* engine_name(EngineKind::Value kind) {
        switch (kind) {
        case EngineKind::Scalar: return "scalar";
        case EngineKind::Avx2: return "avx2";
        case EngineKind::Neon: return "neon";
        }
        return 0;
    }

    bool engine_available(EngineKind::Value kind) {
        if (kind == EngineKind::Scalar) {
            return true;
        }
        bool supported = false;
#ifdef FOSU_HAS_AVX2
        if (kind == EngineKind::Avx2) {
            static const bool avx2 = fosu_dispatch::host_supports_avx2();
            supported = avx2;
        }
#endif
#ifdef FOSU_HAS_NEON
        if (kind == EngineKind::Neon) {
            static const bool neon = fosu_dispatch::host_supports_neon();
            supported = neon;
        }
#endif
#if defined(_WIN32)
        wchar_t path[kPathCapacity];
        return supported && engine_path(engine_name(kind), path) &&
               GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
#else
        char path[PATH_MAX];
        return supported && engine_path(engine_name(kind), path) && access(path, R_OK) == 0;
#endif
    }

    // atomic-lite's clear() is a compiler barrier only on GCC, so a release fence is needed
    // before publishing `selected` to keep a concurrent selector on a weakly-ordered target from
    // observing a stale null. MSVC's clear() already emits a hardware barrier.
    void release_selected() {
#if defined(_WIN32)
        MemoryBarrier();
#else
        __sync_synchronize();
#endif
    }

    // The embedded scalar engine is the never-null fallback: a requested engine
    // that the build or the host cannot provide resolves to scalar rather than to
    // no engine at all (upstream returns null in that case).
    const ParsingEngine* selected_engine() {
        const ParsingEngine* engine = selected;
        if (!engine) {
            while (selecting.test_and_set()) {}
            engine = selected;
            if (!engine) {
                engine = select_engine_from_environment();
                if (!engine) {
                    engine = &embedded_scalar_engine;
                }
                selected = engine;
            }
            release_selected();
            selecting.clear();
        }
        return engine;
    }

    void unload_engine() {
        selected = 0;
        if (library) {
#if defined(_WIN32)
            FreeLibrary(library);
#else
            dlclose(library);
#endif
        }
        library = 0;
    }
}} // namespace fosu::internal
