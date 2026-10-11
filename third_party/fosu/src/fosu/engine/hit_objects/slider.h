#ifndef FOSU_ENGINE_HIT_OBJECTS_SLIDER_H
#define FOSU_ENGINE_HIT_OBJECTS_SLIDER_H

// Parses the slider-specific portion of one hit object and publishes its
// Slider, control points and curve segments into the Beatmap. Section
// iteration and common hit-object fields belong to sections/hit_objects.h.

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>

#include <fosu/beatmap.h>
#include <fosu/compiler.h>
#include <fosu/engine/hit_objects/common_fields.h>
#include <fosu/engine/hit_objects/samples.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/engine/primitives/digit_groups.h>
#include <fosu/engine/primitives/packed_digits.h>
#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/enums.h>
#include <fosu/parse_options.h>
#include <fosu/types.h>

namespace fosu { namespace internal {
    // Running counters for one [HitObjects] section (upstream: HitObjectCounts).
    struct HitObjectCounts {
        size_t objects;
        size_t sliders;
        size_t slider_segments;
        size_t slider_points;
        // Whether osu!'s previous object, in source order, is a spinner. It can be a rejected line:
        // see preceding_spinner_after_rejection.
        bool preceding_was_spinner;
        // The first line that makes osu!stable refuse the map, if any.
        const char* unloadable_line;

        HitObjectCounts()
            : objects(0),
              sliders(0),
              slider_segments(0),
              slider_points(0),
              preceding_was_spinner(false),
              unloadable_line(0) {}
    };

    // Lazer bounds coordinates and slider lengths by +-131072. stable reads any finite length, and
    // fosu keeps its coordinates where the int32 truncation of a control point stays defined.
    inline float coordinate_limit(Client::Value client) {
        return client == Client::Lazer ? 131072.0f : 2147483520.0f;
    }
    inline double slider_length_limit(Client::Value client) {
        return client == Client::Lazer ? 131072.0 : std::numeric_limits<double>::max();
    }

    // Whether the token after the '|' at `p` is a single character.
    inline bool one_character_token(const char* p, const char* end) {
        if (p + 1 >= end || p[1] == '|' || p[1] == ',') {
            return false;
        }
        return p + 2 == end || p[2] == '|' || p[2] == ',';
    }

    inline bool is_ascii_letter(char value) { return static_cast<fosu::u8>((value | 0x20) - 'a') < 26; }

    inline bool parse_curve_type(char value, CurveType::Value& out) {
        switch (value) {
        case 'B': out = CurveType::Bezier; return true;
        case 'C': out = CurveType::Catmull; return true;
        case 'L': out = CurveType::Linear; return true;
        case 'P': out = CurveType::PerfectCurve; return true;
        default: return false;
        }
    }

#if FOSU_SIMD
    // SIMD point decoding is an implementation detail of parsing the point list. The table is
    // indexed by the digit widths of x and y.
    struct FOSU_SIMD_ALIGN(16) SliderPointShuffle {
        i8 bytes[16];
    };

    // Upstream builds this at compile time with `consteval`; the port fills the table once at
    // start-up in the SliderPointShuffleTable constructor below.
    inline void fill_slider_point_shuffles(SliderPointShuffle* out) {
        for (i32 x_digits = 1; x_digits <= 4; ++x_digits) {
            for (i32 y_digits = 1; y_digits <= 4; ++y_digits) {
                SliderPointShuffle& shuffle = out[(x_digits - 1) * 4 + y_digits - 1];
                for (i32 i = 0; i < 16; ++i) {
                    shuffle.bytes[i] = static_cast<i8>(0x80);
                }
                for (i32 i = 0; i < x_digits; ++i) {
                    shuffle.bytes[4 - x_digits + i] = static_cast<i8>(1 + i);
                }
                for (i32 i = 0; i < y_digits; ++i) {
                    shuffle.bytes[8 - y_digits + i] = static_cast<i8>(x_digits + 2 + i);
                }
            }
        }
    }

    struct SliderPointShuffleTable {
        SliderPointShuffle entries[16];
        SliderPointShuffleTable() { fill_slider_point_shuffles(entries); }
    };
    static const SliderPointShuffleTable kSliderPointShuffleTable;
    static const SliderPointShuffle (&kSliderPointShuffles)[16] = kSliderPointShuffleTable.entries;

    FOSU_STATIC_ASSERT(slider_point_layout, sizeof(SliderPoint) == 8 && offsetof(SliderPoint, x) == 0 &&
                                                offsetof(SliderPoint, y) == 4);

#if FOSU_SIMD_X86
    inline __m128i load16(const char* p) { return _mm_loadu_si128(reinterpret_cast<const __m128i*>(p)); }

    inline SliderPoint decode_slider_point(__m128i input, u32 x_digits, u32 y_digits,
                                           const HitObjectParseConstants& constants) {
        const SliderPointShuffle& shuffle = kSliderPointShuffles[(x_digits - 1) * 4 + y_digits - 1];
        const __m128i placed =
            _mm_shuffle_epi8(_mm_sub_epi8(input, _mm256_castsi256_si128(constants.zero)),
                             _mm_load_si128(reinterpret_cast<const __m128i*>(shuffle.bytes)));
        const __m128i coordinates =
            _mm_madd_epi16(_mm_maddubs_epi16(placed, constants.pair_weights), constants.word_weights);
        const __m128 positions = _mm_cvtepi32_ps(coordinates);
        SliderPoint point;
        const u64 bits = static_cast<u64>(_mm_cvtsi128_si64(_mm_castps_si128(positions)));
        std::memcpy(&point, &bits, sizeof(point));
        return point;
    }
#else
    inline uint8x16_t load16(const char* p) { return vld1q_u8(reinterpret_cast<const u8*>(p)); }

    inline SliderPoint decode_slider_point(uint8x16_t input, u32 x_digits, u32 y_digits,
                                           const HitObjectParseConstants& constants) {
        const SliderPointShuffle& shuffle = kSliderPointShuffles[(x_digits - 1) * 4 + y_digits - 1];
        const uint32x4_t coordinates = decimal_groups(vqtbl1q_u8(
            vsubq_u8(input, constants.zero), vld1q_u8(reinterpret_cast<const u8*>(shuffle.bytes))));
        const float32x4_t positions = vcvtq_f32_u32(coordinates);
        SliderPoint point;
        const u64 bits = vgetq_lane_u64(vreinterpretq_u64_f32(positions), 0);
        std::memcpy(&point, &bits, sizeof(point));
        return point;
    }
#endif

#if FOSU_SIMD_NEON
    // parse_ordinary_slider_points for points that fit in the 16 bytes after the '|', as nearly
    // every pair does. Each mask is then one narrowing shift rather than a pairwise reduction, so
    // the next point waits less. Mask positions count four bits per byte. Returns p when the first
    // point does not fit.
    // 1 when `mask` marks the byte at `offset`, else 0; an absent offset of 16 wraps the shift,
    // but every caller also requires the offset to be below 16.
    inline u64 slider_point_marks(u64 mask, u32 offset) { return (mask >> ((4 * offset) & 63)) & 1; }

    FOSU_ALWAYS_INLINE const char* parse_short_slider_points(Beatmap& beatmap, size_t& slider_point_count,
                                                             const char* p,
                                                             const HitObjectParseConstants& constants) {
        const uint8x16_t text = vld1q_u8(reinterpret_cast<const u8*>(p + 1));
        const u64 colons = nibble_mask16(vceqq_u8(text, broadcast_byte<':'>()));
        const u64 pipes = nibble_mask16(vceqq_u8(text, broadcast_byte<'|'>()));
        const u64 commas = nibble_mask16(vceqq_u8(text, constants.comma));
        const u64 ends = pipes | commas;
        // One bit per byte, so clearing the lowest bit clears a whole boundary.
        u64 boundaries = nibble_mask16(nondigit_bytes16(text)) &
                         ((static_cast<u64>(0x11111111u) << 32) | static_cast<u64>(0x11111111u));
        // Offsets from p + 1; 16 when absent.
        const u32 first_colon = static_cast<u32>(trailing_zeros(boundaries)) >> 2;
        boundaries &= boundaries - 1;
        const u32 first_end = static_cast<u32>(trailing_zeros(boundaries)) >> 2;
        boundaries &= boundaries - 1;
        const u32 second_colon = static_cast<u32>(trailing_zeros(boundaries)) >> 2;
        boundaries &= boundaries - 1;
        const u32 second_end = static_cast<u32>(trailing_zeros(boundaries)) >> 2;

        const u32 first_y_digits = first_end - first_colon - 1;
        if (((first_colon - 1) | (first_y_digits - 1)) > 3 || first_end > 15 ||
            !(slider_point_marks(colons, first_colon) & slider_point_marks(ends, first_end))) {
            return p;
        }
        const u32 second_x_digits = second_colon - first_end - 1;
        const u32 second_y_digits = second_end - second_colon - 1;
        const bool second = (((second_x_digits - 1) | (second_y_digits - 1)) <= 3) & (second_end <= 15) &
                            (slider_point_marks(pipes, first_end) & slider_point_marks(colons, second_colon) &
                             slider_point_marks(ends, second_end));
        beatmap.slider_points[slider_point_count] =
            decode_slider_point(load16(p), first_colon, first_y_digits, constants);
        // As in parse_ordinary_slider_points, an absent second point is stored in the spare slot
        // and not counted.
        beatmap.slider_points[slider_point_count + 1] =
            decode_slider_point(load16(p + 1 + first_end), ((second_x_digits - 1) & 3) + 1,
                                ((second_y_digits - 1) & 3) + 1, constants);
        slider_point_count += 1 + second;
        const u32 points_end = second ? second_end : first_end;
        const u32 list_end = static_cast<u32>(trailing_zeros(commas)) >> 2;
        if (points_end == list_end) {
            return p + 1 + list_end;
        }
        return p + 1 + points_end;
    }
#endif

    // Decodes up to two "|x:y" points whose coordinates have one to four digits. One mask finds
    // every boundary. Both points are stored and only valid ones counted, so neither their widths
    // nor whether a second point follows costs a branch; across sliders both vary too much to
    // predict. Returns p when the first point is anything else.
    FOSU_ALWAYS_INLINE const char* parse_ordinary_slider_points(Beatmap& beatmap, size_t& slider_point_count,
                                                                const char* p,
                                                                const HitObjectParseConstants& constants) {
#if FOSU_SIMD_NEON
        const char* next = parse_short_slider_points(beatmap, slider_point_count, p, constants);
        if (next != p) {
            return next;
        }
#endif
        const Bytes32 bytes = load32(p);
        const u64 colons = equal_mask32(bytes, broadcast_byte<':'>());
        const u64 pipes = equal_mask32(bytes, broadcast_byte<'|'>());
        const u32 commas = equal_mask32(bytes, constants.comma);
        const u64 ends = pipes | commas;
        u32 boundaries = nondigit_mask32(bytes) & ~1u;
        const u32 first_colon = trailing_zeros(boundaries);
        boundaries &= boundaries - 1;
        const u32 first_end = trailing_zeros(boundaries);
        boundaries &= boundaries - 1;
        const u32 second_colon = trailing_zeros(boundaries);
        boundaries &= boundaries - 1;
        const u32 second_end = trailing_zeros(boundaries);

        const u32 first_x_digits = first_colon - 1;
        const u32 first_y_digits = first_end - first_colon - 1;
        if (((first_x_digits - 1) | (first_y_digits - 1)) > 3 ||
            !((colons >> first_colon) & (ends >> first_end) & 1)) {
            return p;
        }
        const u32 second_x_digits = second_colon - first_end - 1;
        const u32 second_y_digits = second_end - second_colon - 1;
        const bool second =
            (((second_x_digits - 1) | (second_y_digits - 1)) <= 3) &
            static_cast<bool>((pipes >> first_end) & (colons >> second_colon) & (ends >> second_end) & 1);
        beatmap.slider_points[slider_point_count] =
            decode_slider_point(load16(p), first_x_digits, first_y_digits, constants);
        // The array has a spare slot for an absent second point; its widths are wrapped into the
        // table's range.
        beatmap.slider_points[slider_point_count + 1] =
            decode_slider_point(load16(p + first_end), ((second_x_digits - 1) & 3) + 1,
                                ((second_y_digits - 1) & 3) + 1, constants);
        slider_point_count += 1 + second;
        // Most point lists end at the first comma after these points. Continuing from that comma,
        // behind a branch that rarely fails, keeps the rest of the line from waiting on the
        // boundary search.
        const u32 points_end = second ? second_end : first_end;
        const u32 list_end = trailing_zeros(commas);
        if (points_end == list_end) {
            return p + list_end;
        }
        return p + points_end;
    }
#endif

    inline CurveSegment make_curve_segment(CurveType::Value type, bool has_degree, fosu::u32 degree,
                                           size_t point_begin, size_t point_count) {
        CurveSegment segment;
        segment.type = type;
        segment.has_degree = has_degree;
        segment.degree = degree;
        segment.point_begin = static_cast<fosu::u32>(point_begin);
        segment.point_count = static_cast<fosu::u32>(point_count);
        return segment;
    }

    struct ParsedSliderPoint {
        SliderPoint point;
        const char* end;
    };

    inline bool parse_slider_point(const char* p, const char* end, bool lazer_format, float limit,
                                   ParsedSliderPoint& out) {
        const char* coordinate = p + 1;
        float x;
        fosu::u32 digits = digit_run8(coordinate);
        if (digits - 1 <= 3 && digits <= static_cast<size_t>(end - coordinate) && coordinate[digits] == ':') {
            x = static_cast<float>(swar_parse_u32(coordinate, digits));
            coordinate += digits;
        } else {
            const char* next = parse_osu_float(coordinate, end, x, limit);
            if (next == coordinate || next >= end || *next != ':') {
                return false;
            }
            if (!lazer_format) {
                x = static_cast<float>(static_cast<fosu::i32>(x));
            }
            coordinate = next;
        }

        ++coordinate;
        float y;
        digits = digit_run8(coordinate);
        if (digits - 1 <= 3 && digits <= static_cast<size_t>(end - coordinate) &&
            (coordinate[digits] == ':' || coordinate[digits] == '|' || coordinate[digits] == ',')) {
            y = static_cast<float>(swar_parse_u32(coordinate, digits));
            coordinate += digits;
        } else {
            const char* next = parse_osu_float(coordinate, end, y, limit);
            if (next == coordinate) {
                return false;
            }
            if (!lazer_format) {
                y = static_cast<float>(static_cast<fosu::i32>(y));
            }
            coordinate = next;
        }

        // osu! reads only the first two ':'-separated values of a point.
        if (coordinate < end && *coordinate == ':') {
            while (coordinate < end && *coordinate != '|' && *coordinate != ',') {
                ++coordinate;
            }
        }
        out.point.x = x;
        out.point.y = y;
        out.end = coordinate;
        return true;
    }

    // Slider params after "type,hitSound,":
    //   curveType|x:y|x:y...,slides,length[,edgeSounds,edgeSets][,hitSample]
    //
    // This is one parse transaction. Control points and completed lazer segments are written into
    // their arena arrays as they are accepted. Failures roll both arrays back so rejected sliders
    // leave no published data behind.
    //
    // Lazer starts a segment at each curve type in the point list; before v128 it later applies
    // legacy rules to each one. stable instead reads a one-character token as the curve type of the
    // whole slider, the last one winning, and ignores one naming no type, in every format; fosu
    // follows it in stable mode. Lazer would also read a longer token starting with a letter as a
    // type, which fosu rejects.
    FOSU_NOINLINE inline bool parse_slider(Beatmap& beatmap, HitObjectCounts& counts, HitObject& object,
                                           const char* p, const char* end, bool lazer_format,
                                           Client::Value client, const HitObjectParseConstants& constants) {
        (void)constants;
        if (p >= end) {
            return false;
        }

        const size_t slider_point_begin = counts.slider_points;
        const size_t slider_segment_begin = counts.slider_segments;
        const bool segments = client == Client::Lazer;
        const float limit = coordinate_limit(client);

        const char first_token = *p++;
        CurveType::Value known_curve_type;
        const bool known = parse_curve_type(first_token, known_curve_type);
        if (!known && !is_ascii_letter(first_token)) {
            // Lazer reads any other character as a point, which fails.
            if (client == Client::Lazer) {
                return false;
            }
            // stable ignores it as a marker, unless the token is empty.
            if (first_token == '|' || first_token == ',') {
                return false;
            }
        }
        // Both clients read a letter other than B, C, L or P as Catmull, which is also stable's
        // type when it ignores a marker.
        const CurveType::Value first_curve_type = known ? known_curve_type : CurveType::Catmull;

        bool first_curve_has_degree = false;
        fosu::u32 first_curve_degree = 0;
        if (first_curve_type == CurveType::Bezier && p < end && is_digit(*p)) {
            // B-spline degrees belong to lazer's v128 format. Otherwise the line is rejected:
            // stable cannot load it, and lazer before v128 (unlike us) reads a B-spline.
            if (!lazer_format) {
                return false;
            }
            fosu::i64 degree;
            const char* next = parse_osu_int(p, end, degree);
            if (next == p || degree <= 0 || degree > static_cast<fosu::i64>(kUint32Max)) {
                return false;
            }
            first_curve_has_degree = true;
            first_curve_degree = static_cast<fosu::u32>(degree);
            p = next;
        }

        CurveType::Value current_curve_type = first_curve_type;
        bool current_curve_has_degree = first_curve_has_degree;
        fosu::u32 current_curve_degree = first_curve_degree;
        size_t segment_point_begin = counts.slider_points;
        bool has_explicit_segments = false;

        while (p < end && *p == '|') {
#if FOSU_SIMD
            const char* ordinary_points_end =
                parse_ordinary_slider_points(beatmap, counts.slider_points, p, constants);
            if (ordinary_points_end != p) {
                p = ordinary_points_end;
                continue;
            }
#endif
            if (!segments && one_character_token(p, end)) {
                CurveType::Value type;
                if (parse_curve_type(p[1], type)) {
                    current_curve_type = type;
                }
                p += 2;
                continue;
            }

            const bool starts_segment = segments && p + 1 < end && is_ascii_letter(p[1]);

            CurveType::Value next_curve_type = current_curve_type;
            bool next_curve_has_degree = current_curve_has_degree;
            fosu::u32 next_curve_degree = current_curve_degree;
            if (starts_segment) {
                ++p;
                CurveType::Value parsed;
                next_curve_type = parse_curve_type(*p, parsed) ? parsed : CurveType::Catmull;
                ++p;
                next_curve_has_degree = false;
                next_curve_degree = 0;
                // As for the first type, fosu reads B-spline degrees only in lazer's v128 format.
                if (lazer_format && next_curve_type == CurveType::Bezier && p < end && is_digit(*p)) {
                    fosu::i64 degree;
                    const char* next = parse_osu_int(p, end, degree);
                    if (next == p || degree <= 0 || degree > static_cast<fosu::i64>(kUint32Max)) {
                        counts.slider_points = slider_point_begin;
                        counts.slider_segments = slider_segment_begin;
                        return false;
                    }
                    next_curve_has_degree = true;
                    next_curve_degree = static_cast<fosu::u32>(degree);
                    p = next;
                }
            }

            if (p >= end || *p != '|') {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }

            ParsedSliderPoint point;
            if (!parse_slider_point(p, end, lazer_format, limit, point)) {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }
            beatmap.slider_points[counts.slider_points++] = point.point;
            p = point.end;

            if (starts_segment) {
                if (counts.slider_segments == beatmap.slider_segments.size()) {
                    counts.slider_points = slider_point_begin;
                    counts.slider_segments = slider_segment_begin;
                    return false;
                }
                beatmap.slider_segments[counts.slider_segments++] = make_curve_segment(
                    current_curve_type, current_curve_has_degree, current_curve_degree,
                    segment_point_begin - slider_point_begin, counts.slider_points - segment_point_begin);
                has_explicit_segments = true;
                segment_point_begin = counts.slider_points - 1;
                current_curve_type = next_curve_type;
                current_curve_has_degree = next_curve_has_degree;
                current_curve_degree = next_curve_degree;
            }
        }

        // Everything after the point list is positional.
        if (p >= end || *p != ',') {
            counts.slider_points = slider_point_begin;
            counts.slider_segments = slider_segment_begin;
            return false;
        }
        ++p;

        fosu::i32 slides;
        const fosu::u32 first_slide_digit = static_cast<fosu::u8>(p[0] - '0');
        if (first_slide_digit <= 9 && p[1] == ',') {
            slides = static_cast<fosu::i32>(first_slide_digit);
            ++p;
        } else {
            const fosu::u32 digits = digit_run8(p);
            if (digits - 1 <= 6) {
                slides = static_cast<fosu::i32>(swar_parse_u64(p, digits));
                p = skip_numeric_space(p + digits, end);
            } else {
                fosu::i64 parsed_slides;
                const char* next = parse_osu_int(p, end, parsed_slides);
                if (next == p) {
                    counts.slider_points = slider_point_begin;
                    counts.slider_segments = slider_segment_begin;
                    return false;
                }
                slides = clamp_i32(parsed_slides);
                p = next;
            }
        }
        if (slides > 9000 || (p < end && *p != ',')) {
            counts.slider_points = slider_point_begin;
            counts.slider_segments = slider_segment_begin;
            return false;
        }

        double length = 0;
        if (p < end) {
            const char* length_begin = p + 1;
            const char* next = parse_osu_double(length_begin, end, length, slider_length_limit(client));
            if (next == length_begin || (next < end && *next != ',')) {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }
            p = next;
        }

        nonstd::string_view sound_fields[3];
        if (p < end) {
            const char* sound_begin = p + 1;
            for (int field = 0; field < 3; ++field) {
                const char* comma = find_byte(',', sound_begin, end);
                sound_fields[field] =
                    nonstd::string_view(sound_begin, static_cast<size_t>(comma - sound_begin));
                if (comma == end) {
                    break;
                }
                sound_begin = comma + 1;
            }
        }

        const nonstd::string_view edge_sounds = sound_fields[0];
        const nonstd::string_view edge_sets = sound_fields[1];
        const nonstd::string_view hit_sample = sound_fields[2];
        if (!valid_sample(hit_sample, true) || !valid_edge_sets(edge_sets, slides)) {
            counts.slider_points = slider_point_begin;
            counts.slider_segments = slider_segment_begin;
            return false;
        }

        if (segments && (has_explicit_segments || first_curve_has_degree)) {
            if (counts.slider_segments == beatmap.slider_segments.size()) {
                counts.slider_points = slider_point_begin;
                counts.slider_segments = slider_segment_begin;
                return false;
            }
            beatmap.slider_segments[counts.slider_segments++] = make_curve_segment(
                current_curve_type, current_curve_has_degree, current_curve_degree,
                segment_point_begin - slider_point_begin, counts.slider_points - segment_point_begin);
        }

        Slider& slider = beatmap.sliders[counts.sliders];
        slider.point_begin = static_cast<fosu::u32>(slider_point_begin);
        slider.point_count = static_cast<fosu::u32>(counts.slider_points - slider_point_begin);
        slider.segment_begin = static_cast<fosu::u32>(slider_segment_begin);
        slider.segment_count =
            segments ? static_cast<fosu::u32>(counts.slider_segments - slider_segment_begin) : 0;
        slider.slides = std::max(1, slides);
        slider.curve_type = segments ? first_curve_type : current_curve_type;
        slider.length = std::max(0.0, length);
        slider.edge_sounds = edge_sounds;
        slider.edge_sets = edge_sets;
        object.hit_sample = hit_sample;
        object.slider = static_cast<fosu::u32>(counts.sliders++);
        return true;
    }
}} // namespace fosu::internal

#endif
