#ifndef FOSU_ENGINE_PARSE_DOCUMENT_H
#define FOSU_ENGINE_PARSE_DOCUMENT_H

#include <fosu/beatmap.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/parsing/section_names.h>
#include <fosu/engine/sections/colours.h>
#include <fosu/engine/sections/difficulty.h>
#include <fosu/engine/sections/editor.h>
#include <fosu/engine/sections/events.h>
#include <fosu/engine/sections/general.h>
#include <fosu/engine/sections/hit_objects.h>
#include <fosu/engine/sections/metadata.h>
#include <fosu/engine/sections/timing_points.h>
#include <fosu/parse_options.h>

namespace fosu { namespace internal {

    FOSU_STATIC_ASSERT(section_bits_mirror_ordinals, kSectionGeneral == (1u << Section::General) &&
                                                         kSectionDifficulty == (1u << Section::Difficulty) &&
                                                         kSectionHitObjects == (1u << Section::HitObjects));

    struct PreambleLine {
        Beatmap* beatmap;
        void operator()(StringView line) {
            const size_t version = line.find(StringView("osu file format v"));
            if (version != StringView::npos) {
                fosu_int64 value;
                const char* number = line.data() + version + 17;
                if (parse_i64(number, line.data() + line.size(), value) != number) {
                    beatmap->format_version = clamp_i32(value);
                }
            }
        }
    };

    inline const char* parse_preamble(Beatmap& beatmap, const char* p, const char* end) {
        if (end - p >= 3 && static_cast<fosu_uint8>(p[0]) == 0xEF && static_cast<fosu_uint8>(p[1]) == 0xBB &&
            static_cast<fosu_uint8>(p[2]) == 0xBF) {
            p += 3;
        }
        PreambleLine parse_line;
        parse_line.beatmap = &beatmap;
        return for_each_section_line(p, end, parse_line);
    }

    // Each section consumes its body and returns the next header or EOF; framing stays inside the
    // section. Input has kBufferPadding readable zero bytes; string views refer into it.
    inline void parse_document(const char* data, size_t size, Beatmap& beatmap, const ParseOptions& options) {
        size_t velocity_preset_count = 0;
        bool velocity_presets_seen = false;
        if (size == 0) {
            if ((options.sections & kSectionEditor) && beatmap.velocity_presets.size() >= 3) {
                set_default_velocity_presets(beatmap);
            }
            return;
        }
        const char* end = data + size;
        const char* p = parse_preamble(beatmap, data, end);
        const int time_offset = options.apply_offsets && beatmap.format_version < 5 ? 24 : 0;
        size_t break_count = 0, colour_count = 0, timing_point_count = 0;
        size_t hit_object_count = 0, slider_count = 0, slider_point_count = 0;
        size_t slider_segment_count = 0;
        Optional<double> approach_rate;
        fosu_uint32 pending = options.sections & 0x1FEu;

        while (p < end) {
            const Line header = read_line(p, end);
            const Section::Value section = match_section(header.text);
            const fosu_uint32 bit = 1u << static_cast<int>(section);
            p = header.next;
            if (!(options.sections & bit)) {
                if (!pending) {
                    break;
                }
                p = skip_section(p, end);
                continue;
            }
            pending &= ~bit;

            switch (section) {
            case Section::General: p = parse_general_section(beatmap, p, end); break;
            case Section::Editor:
                p = parse_editor_section(beatmap, velocity_preset_count, velocity_presets_seen, p, end);
                break;
            case Section::Metadata: p = parse_metadata_section(beatmap, p, end); break;
            case Section::Difficulty: p = parse_difficulty_section(beatmap, approach_rate, p, end); break;
            case Section::Events: p = parse_events_section(beatmap, break_count, p, end, time_offset); break;
            case Section::TimingPoints:
                p = parse_timing_points_section(beatmap, timing_point_count, p, end, time_offset);
                break;
            case Section::Colours: p = parse_colours_section(beatmap, colour_count, p, end); break;
            case Section::HitObjects:
                p = parse_hitobjects_section(beatmap, hit_object_count, slider_count, slider_segment_count,
                                             slider_point_count, p, end, time_offset);
                break;
            case Section::None:
            case Section::Unknown: p = skip_section(p, end); break;
            }
        }

        // An omitted (or wholly invalid) AR inherits the final OD across sections.
        beatmap.ar = approach_rate.value_or(beatmap.od);
        // General may follow Difficulty or repeat; CS depends on the final mode.
        beatmap.cs =
            beatmap.mode == 3 ? clamp_value(beatmap.cs, 1.0, 18.0) : clamp_value(beatmap.cs, 0.0, 10.0);
        if ((options.sections & kSectionEditor) && !velocity_presets_seen &&
            beatmap.velocity_presets.size() >= 3) {
            velocity_preset_count = set_default_velocity_presets(beatmap);
        }
        beatmap.breaks = beatmap.breaks.first(break_count);
        beatmap.combo_colours = beatmap.combo_colours.first(colour_count);
        beatmap.timing_points = beatmap.timing_points.first(timing_point_count);
        beatmap.hit_objects = beatmap.hit_objects.first(hit_object_count);
        beatmap.sliders = beatmap.sliders.first(slider_count);
        beatmap.slider_segments = beatmap.slider_segments.first(slider_segment_count);
        beatmap.slider_points = beatmap.slider_points.first(slider_point_count);
        beatmap.velocity_presets = beatmap.velocity_presets.first(velocity_preset_count);
    }

}} // namespace fosu::internal

#endif
