#ifndef FOSU_PARSE_OPTIONS_H
#define FOSU_PARSE_OPTIONS_H

#include <fosu/compiler.h>
#include <fosu/mods.h>

namespace fosu {

    enum SectionBits {
        kSectionGeneral = 1u << 1,
        kSectionEditor = 1u << 2,
        kSectionMetadata = 1u << 3,
        kSectionDifficulty = 1u << 4,
        kSectionEvents = 1u << 5,
        kSectionTimingPoints = 1u << 6,
        kSectionColours = 1u << 7,
        kSectionHitObjects = 1u << 8,
        kAllSections = kSectionGeneral | kSectionEditor | kSectionMetadata | kSectionDifficulty |
                       kSectionEvents | kSectionTimingPoints | kSectionColours | kSectionHitObjects
    };

    struct ParseOptions {
        fosu_uint32 sections;
        bool calculate_slider_end_times;
        bool calculate_slider_paths;
        bool calculate_slider_events;
        bool apply_stacking;
        bool apply_offsets;
        fosu_uint32 mods; // Mods::Value bits

        ParseOptions()
            : sections(kAllSections),
              calculate_slider_end_times(false),
              calculate_slider_paths(false),
              calculate_slider_events(false),
              apply_stacking(false),
              apply_offsets(true),
              mods(Mods::None) {}
    };

} // namespace fosu

#endif
