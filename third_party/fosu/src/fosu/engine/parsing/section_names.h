#ifndef FOSU_ENGINE_PARSING_SECTION_NAMES_H
#define FOSU_ENGINE_PARSING_SECTION_NAMES_H

#include <fosu/engine/parsing/string_lookup.h>
#include <fosu/nonstd_config.h>
#include <fosu/parse_options.h>

namespace fosu { namespace internal {
    // Ordinals match the public kSection bits; None only fills ordinal 0.
    // Unknown is any span fosu skips: before stable's first header, or a section
    // fosu does not read, including ones a client knows.
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
    extern const StringLookup<Section::Value> kLazerSectionNames;
    extern const StringLookup<Section::Value> kStableSectionNames;

    // The section a header line starts; see section_header_line. Both clients
    // match the name exactly; fosu does not follow the numbers, comma lists and
    // padding their enum parsers also accept. A lazer header naming no section
    // starts [General]; a stable one leaves `current` in effect.
    inline Section::Value match_section(nonstd::string_view line, Section::Value current,
                                        Client::Value client) {
        if (client == Client::Lazer) {
            while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
                line.remove_suffix(1);
            }
            const Section::Value* section = kLazerSectionNames.find(line.substr(1, line.size() - 2));
            return section ? *section : Section::General;
        }
        // stable trims brackets from both ends: "[[Events]" names Events.
        const size_t first = line.find_first_not_of("[]");
        if (first == nonstd::string_view::npos) {
            return current;
        }
        const size_t last = line.find_last_not_of("[]");
        const Section::Value* section = kStableSectionNames.find(line.substr(first, last + 1 - first));
        return section ? *section : current;
    }
}} // namespace fosu::internal

#endif
