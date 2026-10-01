#ifndef FOSU_ENGINE_PARSING_SECTION_NAMES_H
#define FOSU_ENGINE_PARSING_SECTION_NAMES_H

#include <fosu/engine/parsing/string_lookup.h>
#include <fosu/span.h>

namespace fosu { namespace internal {

    struct Section {
        enum Value {
            None,
            General,
            Editor,
            Metadata,
            Difficulty,
            Events,
            TimingPoints,
            Colours,
            HitObjects,
            Unknown
        };
    };

    // Defined in engine/parsing/lookup_tables.cpp (upstream builds every table at compile time;
    // the port builds them once at program start-up).
    extern const StringLookup<Section::Value> kSectionNames;

    inline Section::Value match_section(StringView line) {
        const Section::Value* section = kSectionNames.find(line);
        return section ? *section : Section::Unknown;
    }

}} // namespace fosu::internal

#endif
