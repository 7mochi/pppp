#ifndef FOSU_ENGINE_SECTIONS_METADATA_H
#define FOSU_ENGINE_SECTIONS_METADATA_H

#include <fosu/engine/parsing/key_value.h>

namespace fosu { namespace internal {
    extern const StringLookup<FieldParser> kMetadataFields;

    inline const char* parse_metadata_section(Beatmap& beatmap, const char* p, const char* end,
                                              const ParseContext& context) {
        return parse_key_value_section(beatmap, kMetadataFields, p, end, context);
    }
}} // namespace fosu::internal

#endif
