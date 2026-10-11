#ifndef FOSU_FORMAT_H
#define FOSU_FORMAT_H

#include <fosu/parse_options.h>
#include <fosu/types.h>

namespace fosu {

    // The rules a map's format version implies for the target client. Its
    // "osu file format vN" line fixes them before any section is read, so parsing
    // chooses one Format per map and passes it as a template argument: inside the
    // parse, every rule is a compile-time constant. Other differences between the
    // clients are separate; see Client in parse_options.h.
    struct Format {
        // v128+, the format lazer writes: segmented slider syntax, unrounded
        // coordinates and any number of velocity presets.
        bool lazer_format;
        // Below v5: timestamps are shifted 24 ms later.
        i32 time_offset;

        Format()
            : lazer_format(false),
              time_offset(0) {}
        Format(bool lazer, i32 offset)
            : lazer_format(lazer),
              time_offset(offset) {}
    };

    // Compile-time tags for the three formats (spec §3: with_format becomes tag
    // dispatch). Each tag exposes the same two values as a Format, as constants.
    struct FormatBeforeV5 {
        enum { lazer_format = 0, time_offset = 24 };
    };
    struct FormatV5 {
        enum { lazer_format = 0, time_offset = 0 };
    };
    struct FormatV128 {
        enum { lazer_format = 1, time_offset = 0 };
    };

    inline Format kFormatBeforeV5() { return Format(false, 24); }
    inline Format kFormatV5() { return Format(false, 0); }
    inline Format kFormatV128() { return Format(true, 0); }

    // stable has no rules past v14, its latest format, so it reads a v128 map
    // like any other v5+ map. The map still reports version 128.
    inline bool reads_lazer_format(i32 version, Client::Value client) {
        return version >= 128 && client == Client::Lazer;
    }

    // port-only: the runtime carrier for the two parse axes. Master threads these
    // as template arguments <Format F, Client C>; the C++98 port threads one value
    // through the section functions instead (see divergences.txt).
    struct ParseContext {
        Format format;
        Client::Value client;

        ParseContext()
            : format(),
              client(Client::Stable) {}
        ParseContext(const Format& f, Client::Value c)
            : format(f),
              client(c) {}
    };
} // namespace fosu

#endif
