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

    // Whose behaviour to follow where osu!stable and osu!lazer parse a map
    // differently. This is independent of the map's format version; see
    // docs/compatibility.md for the rules each client changes.
    struct Client {
        enum Value { Stable, Lazer };
    };

    // Why a parse returned no beatmap.
    struct ParseErrorCode {
        enum Value {
            None,
            InvalidOptions,
            OutOfMemory,
            ReadFailed, // parse_file could not read the file; errno has the cause.
            Unloadable  // The target client would refuse to load this map.
        };
    };

    struct ParseError {
        ParseErrorCode::Value code;
        fosu::u32 line; // 1-based line that makes the map unloadable.

        ParseError()
            : code(ParseErrorCode::None),
              line(0) {}
        explicit ParseError(ParseErrorCode::Value c, fosu::u32 l = 0)
            : code(c),
              line(l) {}
    };

    struct ParseOptions {
        fosu::u32 sections;
        bool calculate_slider_end_times;
        bool calculate_slider_paths;
        bool calculate_slider_events;
        bool apply_stacking;
        fosu::u32 mods; // Mods::Value bits
        Client::Value client;

        ParseOptions()
            : sections(kAllSections),
              calculate_slider_end_times(false),
              calculate_slider_paths(false),
              calculate_slider_events(false),
              apply_stacking(false),
              mods(Mods::None),
              client(Client::Stable) {}
    };
} // namespace fosu

#endif
