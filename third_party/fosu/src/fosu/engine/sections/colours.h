#ifndef FOSU_ENGINE_SECTIONS_COLOURS_H
#define FOSU_ENGINE_SECTIONS_COLOURS_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/field_values.h>
#include <fosu/engine/parsing/lines.h>

namespace fosu { namespace internal {

    inline Optional<fosu_uint32> parse_colour(StringView input) {
        fosu_uint32 rgb = 0;
        for (int i = 0; i < 3; ++i) {
            const size_t comma = input.find(',');
            const Optional<fosu_int32> component = parse_field_integer(input.substr(0, comma));
            if (!component.has_value() || *component < 0 || *component > 255) {
                return Optional<fosu_uint32>();
            }
            rgb = (rgb << 8) | static_cast<fosu_uint32>(*component);
            if (i < 2) {
                if (comma == StringView::npos) {
                    return Optional<fosu_uint32>();
                }
                input.remove_prefix(comma + 1);
            } else if (comma != StringView::npos && input.find(',', comma + 1) != StringView::npos) {
                return Optional<fosu_uint32>();
            }
        }
        // The legacy decoder accepts a fourth component but ignores alpha.
        return Optional<fosu_uint32>(rgb);
    }

    struct ColoursSectionLine {
        Beatmap* beatmap;
        size_t* colour_count;
        void operator()(StringView line) {
            const size_t comment = line.find(StringView("//"));
            if (comment != StringView::npos) {
                line = line.substr(0, comment);
            }
            const char* line_end = line.data() + line.size();
            const char* colon = find_byte(':', line.data(), line_end);
            Optional<fosu_uint32> colour;
            if (colon != line_end) {
                colour = parse_colour(trim(colon + 1, line_end));
            }
            if (!colour.has_value()) {
                ++beatmap->stats.malformed_lines;
                return;
            }
            const StringView key = trim(line.data(), colon);
            if (key.substr(0, 5) != StringView("Combo")) {
                return;
            }
            const Optional<fosu_int32> index = parse_field_integer(key.substr(5));
            if (index.has_value() && *index >= 1 && *index <= 8) {
                beatmap->combo_colours[(*colour_count)++] = *colour;
            }
        }
    };

    inline const char* parse_colours_section(Beatmap& beatmap, size_t& colour_count, const char* p,
                                             const char* end) {
        ColoursSectionLine parse_line;
        parse_line.beatmap = &beatmap;
        parse_line.colour_count = &colour_count;
        return for_each_section_line(p, end, parse_line);
    }

}} // namespace fosu::internal

#endif
