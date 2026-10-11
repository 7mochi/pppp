#ifndef FOSU_ENGINE_SECTIONS_COLOURS_H
#define FOSU_ENGINE_SECTIONS_COLOURS_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/format.h>

namespace fosu { namespace internal {
    inline nonstd::optional<fosu::u32> parse_colour(nonstd::string_view input) {
        fosu::u32 rgb = 0;
        for (int i = 0; i < 3; ++i) {
            const size_t comma = input.find(',');
            const nonstd::optional<fosu::i32> component = parse_field_integer(input.substr(0, comma));
            if (!component.has_value() || *component < 0 || *component > 255) {
                return nonstd::optional<fosu::u32>();
            }
            rgb = (rgb << 8) | static_cast<fosu::u32>(*component);
            if (i < 2) {
                if (comma == nonstd::string_view::npos) {
                    return nonstd::optional<fosu::u32>();
                }
                input.remove_prefix(comma + 1);
            } else if (comma != nonstd::string_view::npos &&
                       input.find(',', comma + 1) != nonstd::string_view::npos) {
                return nonstd::optional<fosu::u32>();
            }
        }
        // The legacy decoder accepts a fourth component but ignores alpha.
        return nonstd::optional<fosu::u32>(rgb);
    }

    struct ColoursSectionLine {
        Beatmap* beatmap;
        size_t* colour_count;
        Client::Value client;
        void operator()(nonstd::string_view line) {
            // Like storyboard commands, stable skips lines indented with ' ' or '_'.
            if (client == Client::Stable && (line.front() == ' ' || line.front() == '_')) {
                return;
            }
            const size_t comment = line.find(nonstd::string_view("//"));
            if (comment != nonstd::string_view::npos) {
                line = line.substr(0, comment);
            }
            const char* line_end = line.data() + line.size();
            const char* colon = find_byte(':', line.data(), line_end);
            nonstd::optional<fosu::u32> colour;
            if (colon != line_end) {
                colour = parse_colour(trim(colon + 1, line_end));
            }
            if (!colour.has_value()) {
                ++beatmap->stats.malformed_lines;
                return;
            }
            const nonstd::string_view key = trim(line.data(), colon);
            if (key.substr(0, 5) != nonstd::string_view("Combo")) {
                return;
            }
            const nonstd::optional<fosu::i32> index = parse_field_integer(key.substr(5));
            if (index.has_value() && *index >= 1 && *index <= 8) {
                beatmap->combo_colours[(*colour_count)++] = *colour;
            }
        }
    };

    inline const char* parse_colours_section(Beatmap& beatmap, size_t& colour_count, const char* p,
                                             const char* end, const ParseContext& context) {
        ColoursSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.colour_count = &colour_count;
        parse_line.client = context.client;
        return for_each_section_line(p, end, parse_line, context.client);
    }
}} // namespace fosu::internal

#endif
