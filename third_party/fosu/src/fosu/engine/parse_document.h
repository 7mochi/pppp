#ifndef FOSU_ENGINE_PARSE_DOCUMENT_H
#define FOSU_ENGINE_PARSE_DOCUMENT_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/key_value.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/parsing/section_names.h>
#include <fosu/engine/parsing_engine.h>
#include <fosu/engine/sections/colours.h>
#include <fosu/engine/sections/difficulty.h>
#include <fosu/engine/sections/editor.h>
#include <fosu/engine/sections/events.h>
#include <fosu/engine/sections/general.h>
#include <fosu/engine/sections/hit_objects.h>
#include <fosu/engine/sections/metadata.h>
#include <fosu/engine/sections/timing_points.h>
#include <fosu/format.h>
#include <fosu/parse_options.h>

namespace fosu { namespace internal {
    FOSU_STATIC_ASSERT(section_bits_mirror_ordinals, kSectionGeneral == (1u << Section::General) &&
                                                         kSectionDifficulty == (1u << Section::Difficulty) &&
                                                         kSectionHitObjects == (1u << Section::HitObjects));

    // The values each client gives fields a map omits, where they differ.
    inline void apply_client_defaults(Beatmap& beatmap, Client::Value client) {
        if (client == Client::Stable) {
            beatmap.countdown = 1; // Countdown.Normal
            beatmap.beatmap_id = 0;
        } else {
            beatmap.countdown = 0; // CountdownType.None
            beatmap.beatmap_id = -1;
        }
    }

    // Both clients read the version as the whole integer after the header's last 'v'
    // ("osu file format v5v4" is version 4) and refuse a map whose integer does not parse. stable
    // reads it with Int32.Parse, and from a line that only needs to start with "osu file format";
    // lazer reads it with its ParseInt.
    inline bool parse_format_version(Beatmap& beatmap, nonstd::string_view line, Client::Value client) {
        const nonstd::string_view magic = client == Client::Stable ? nonstd::string_view("osu file format")
                                                                   : nonstd::string_view("osu file format v");
        if (!line.starts_with(magic)) {
            return true;
        }
        // Without a 'v', stable parses the whole line, which fails.
        const size_t last_v = line.rfind('v');
        const char* number = last_v == nonstd::string_view::npos ? line.data() : line.data() + last_v + 1;
        const char* end = line.data() + line.size();
        fosu::i64 value;
        const char* next;
        if (client == Client::Stable) {
            const char* first = skip_numeric_space(number, end);
            const char* digits_end = parse_i64(first, end, value);
            if (digits_end == first || value < kInt32Min || value > kInt32Max) {
                return false;
            }
            next = skip_numeric_space(digits_end, end);
        } else {
            next = parse_osu_int(number, end, value);
            if (next == number) {
                return false;
            }
        }
        if (next != end) {
            return false;
        }
        beatmap.format_version = static_cast<fosu::i32>(value);
        return true;
    }

    inline const char* skip_bom(const char* p, const char* end) {
        if (end - p >= 3 && static_cast<fosu::u8>(p[0]) == 0xEF && static_cast<fosu::u8>(p[1]) == 0xBB &&
            static_cast<fosu::u8>(p[2]) == 0xBF) {
            p += 3;
        }
        return p;
    }

    // The 1-based number of the line starting at `line`. Like .NET's StreamReader, "\r\n", "\n"
    // and a lone "\r" each end a line.
    inline fosu::u32 line_number(const char* begin, const char* line) {
        fosu::u32 number = 1;
        for (const char* p = begin; p < line; ++p) {
            const bool crlf = *p == '\r' && p + 1 < line && p[1] == '\n';
            if (*p == '\n' || (*p == '\r' && !crlf)) {
                ++number;
            }
        }
        return number;
    }

    // stable reads General, Metadata, Difficulty and TimingPoints with a separate header reader
    // that never leaves [HitObjects] once it enters it.
    inline bool read_by_stable_header_reader(Section::Value section) {
        return section == Section::General || section == Section::Metadata ||
               section == Section::Difficulty || section == Section::TimingPoints;
    }

    // Where the sections start, and the section in effect before the first header.
    struct Preamble {
        const char* body;
        Section::Value section;
        ParseError error;

        Preamble() : body(0), section(Section::Unknown), error() {}
        Preamble(const char* b, Section::Value s, const ParseError& e) : body(b), section(s), error(e) {}
    };

    // stable takes the version from the first line only, whatever it is: a header there starts its
    // section for the object reader alone. Lazer reads it from the first non-blank line, and reads
    // every line before the first header as [General].
    inline Preamble parse_preamble(Beatmap& beatmap, const char* begin, const char* end,
                                   Client::Value client) {
        const char* p = skip_bom(begin, end);
        if (client == Client::Stable) {
            const Line first = read_line(p, end);
            if (!parse_format_version(beatmap, first.text, client)) {
                return Preamble(end, Section::Unknown, ParseError(ParseErrorCode::Unloadable, 1));
            }
            Section::Value section = Section::Unknown;
            if (section_header_line(first.text.data(), first.text.data() + first.text.size(), client)) {
                section = match_section(first.text, section, client);
                if (read_by_stable_header_reader(section)) {
                    section = Section::Unknown;
                }
            }
            return Preamble(first.next, section, ParseError());
        }
        for (const char* q = p; q < end;) {
            const Line line = read_line(q, end);
            const nonstd::string_view text = trim_field(line.text);
            if (!text.empty()) {
                if (!parse_format_version(beatmap, text, client)) {
                    return Preamble(end, Section::Unknown,
                                    ParseError(ParseErrorCode::Unloadable, line_number(begin, q)));
                }
                break;
            }
            q = line.next;
        }
        // The version line itself has no key, so it reads as no General field.
        return Preamble(p, Section::General, ParseError());
    }

    // The map's format rules for `version` and `client`. Master selects these at compile time with
    // with_format; the port picks the runtime Format (see divergences.txt).
    inline Format select_format(fosu::i32 version, Client::Value client) {
        if (reads_lazer_format(version, client)) {
            return kFormatV128();
        }
        if (version >= 5) {
            return kFormatV5();
        }
        return kFormatBeforeV5();
    }

    // Parses every section, with the map's format rules and the target client's behaviour fixed
    // once. Each section consumes its body and returns the next header or EOF; framing and scalar
    // fallbacks stay inside the section. Input has kBufferPadding readable zero bytes; string views
    // refer into it.
    inline ParseError parse_document(const char* data, size_t size, Beatmap& beatmap,
                                     ParseOptions options) {
        apply_client_defaults(beatmap, options.client);
        size_t velocity_preset_count = 0;
        bool velocity_presets_seen = false;
        const char* begin = data;
        const char* end = data + size;
        const Preamble preamble = parse_preamble(beatmap, begin, end, options.client);
        if (preamble.error.code != ParseErrorCode::None) {
            return preamble.error;
        }
        const ParseContext context(select_format(beatmap.format_version, options.client), options.client);
        size_t break_count = 0, colour_count = 0, timing_point_count = 0;
        HitObjectCounts counts;
        nonstd::optional<double> approach_rate;
        fosu::u32 pending = options.sections & 0x1FEu;
        Section::Value section = preamble.section;
        bool hit_objects_seen = false;
        const char* p = preamble.body;

        while (true) {
            const fosu::u32 bit = 1u << static_cast<int>(section);
            if (!(options.sections & bit)) {
                if (!pending) {
                    break;
                }
                p = skip_section(p, end, options.client);
            } else {
                pending &= ~bit;
                switch (section) {
                case Section::General: p = parse_general_section(beatmap, p, end, context); break;
                case Section::Editor:
                    p = parse_editor_section(beatmap, velocity_preset_count, velocity_presets_seen, p, end,
                                             context);
                    break;
                case Section::Metadata: p = parse_metadata_section(beatmap, p, end, context); break;
                case Section::Difficulty:
                    p = parse_difficulty_section(beatmap, approach_rate, p, end, context);
                    break;
                case Section::Events: p = parse_events_section(beatmap, break_count, p, end, context); break;
                case Section::TimingPoints:
                    p = parse_timing_points_section(beatmap, timing_point_count, p, end, context);
                    break;
                case Section::Colours: p = parse_colours_section(beatmap, colour_count, p, end, context); break;
                case Section::HitObjects:
                    p = parse_hitobjects_section(beatmap, counts, p, end, context);
                    break;
                case Section::None:
                case Section::Unknown: p = skip_section(p, end, options.client); break;
                }
            }
            if (p >= end) {
                break;
            }
            const Line header = read_line(p, end);
            p = header.next;
            section = match_section(header.text, section, options.client);
            if (options.client == Client::Stable) {
                if (hit_objects_seen && read_by_stable_header_reader(section)) {
                    section = Section::Unknown;
                }
                hit_objects_seen = hit_objects_seen || section == Section::HitObjects;
            }
        }

        if (counts.unloadable_line) {
            return ParseError(ParseErrorCode::Unloadable, line_number(begin, counts.unloadable_line));
        }
        // An omitted (or wholly invalid) AR inherits the final OD across sections.
        beatmap.ar = approach_rate.has_value() ? *approach_rate : beatmap.od;
        // In lazer, General may follow Difficulty or repeat; CS depends on the final mode.
        if (options.client == Client::Lazer) {
            beatmap.cs = beatmap.mode == 3 ? clamp_value(beatmap.cs, 1.0, 18.0)
                                           : clamp_value(beatmap.cs, 0.0, 10.0);
        }
        if ((options.sections & kSectionEditor) && !velocity_presets_seen &&
            beatmap.velocity_presets.size() >= 3) {
            velocity_preset_count = set_default_velocity_presets(beatmap);
        }
        beatmap.breaks = beatmap.breaks.first(break_count);
        beatmap.combo_colours = beatmap.combo_colours.first(colour_count);
        beatmap.timing_points = beatmap.timing_points.first(timing_point_count);
        beatmap.hit_objects = beatmap.hit_objects.first(counts.objects);
        beatmap.sliders = beatmap.sliders.first(counts.sliders);
        beatmap.slider_segments = beatmap.slider_segments.first(counts.slider_segments);
        beatmap.slider_points = beatmap.slider_points.first(counts.slider_points);
        beatmap.velocity_presets = beatmap.velocity_presets.first(velocity_preset_count);
        return ParseError();
    }

    // The ISA this code was compiled for, not a runtime choice based on the host CPU. C++98 has no
    // inline variables, so the definition lives in one TU per module (the scalar engine in the
    // core, the SIMD engine in an engine library); see engine/parse_document.cpp.
    extern const ParsingEngine compiled_engine;
}} // namespace fosu::internal

#endif
