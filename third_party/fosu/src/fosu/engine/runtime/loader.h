#ifndef FOSU_ENGINE_RUNTIME_LOADER_H
#define FOSU_ENGINE_RUNTIME_LOADER_H

#include <fosu/engine/parsing_engine.h>
#include <fosu/nonstd_config.h>

namespace fosu { namespace internal {
    const ParsingEngine* selected_engine();
    bool engine_available(EngineKind::Value kind);
    const char* engine_name(EngineKind::Value kind);
    const EngineKind::Value* find_engine_kind(nonstd::string_view name);
    void unload_engine();
}} // namespace fosu::internal

#endif
