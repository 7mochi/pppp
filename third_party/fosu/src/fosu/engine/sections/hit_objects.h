#ifndef FOSU_ENGINE_SECTIONS_HIT_OBJECTS_H
#define FOSU_ENGINE_SECTIONS_HIT_OBJECTS_H

#include <algorithm>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/compiler.h>
#include <fosu/engine/hit_objects/common_fields.h>
#include <fosu/engine/hit_objects/object_types.h>
#include <fosu/engine/hit_objects/samples.h>
#include <fosu/engine/hit_objects/slider.h>
#include <fosu/engine/parsing/lines.h>
#include <fosu/engine/parsing/numbers.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/engine/primitives/digit_groups.h>
#include <fosu/engine/primitives/vector_ops.h>
#include <fosu/format.h>
#include <fosu/types.h>

namespace fosu { namespace internal {
    // Parses one complete [HitObjects] section and publishes accepted objects.
    // Object-specific syntax is delegated after the common fields are decoded.

    // Everything after the "x,y,time,type,hitSound" prefix: slider params,
    // spinner/hold end times, or a trailing hit sample.
    FOSU_NOINLINE inline bool parse_hitobject_details(Beatmap& beatmap, HitObjectCounts& counts,
                                                      HitObject& object, const char* p, const char* end,
                                                      bool lazer_format, Client::Value client,
                                                      const HitObjectParseConstants& constants) {
        switch (classify_hitobject_kind(object.type)) {
        case HitObjectKind::Circle: {
            CircleDetails details;
            if (!parse_circle_details(p, end, details)) {
                return false;
            }
            object.end_time = 0;
            object.hit_sample = details.hit_sample;
            return true;
        }
        case HitObjectKind::Slider:
            return p < end && *p == ',' &&
                   parse_slider(beatmap, counts, object, p + 1, end, lazer_format, client, constants);
        case HitObjectKind::Spinner: {
            TimedHitObjectDetails details;
            if (!parse_spinner_details(p, end, details)) {
                return false;
            }
            object.end_time = details.end_time;
            object.hit_sample = details.hit_sample;
            return true;
        }
        case HitObjectKind::Hold: {
            TimedHitObjectDetails details;
            if (!parse_hold_details(object.time, p, end, details)) {
                return false;
            }
            object.end_time = details.end_time;
            object.hit_sample = details.hit_sample;
            return true;
        }
        case HitObjectKind::Invalid: return false;
        }
        return false;
    }

    struct HitObjectPrefix {
        HitObject object;
        const char* rest; // After hitSound: empty, or starting with ','.
    };

    // The x,y,time,type,hitSound prefix of a line: signed, decimal, spaced or wide fields.
    inline bool parse_hitobject_prefix(const char* p, const char* line_end, bool lazer_format,
                                       Client::Value client, HitObjectPrefix& out) {
        float x;
        const char* next = parse_osu_float(p, line_end, x, coordinate_limit(client));
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        float y;
        next = parse_osu_float(p, line_end, y, coordinate_limit(client));
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        double time;
        next = parse_osu_double(p, line_end, time);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        fosu::i64 type;
        next = parse_osu_int(p, line_end, type);
        if (next == p || next >= line_end || *next != ',') {
            return false;
        }
        p = next + 1;

        fosu::i64 hitsound;
        next = parse_osu_int(p, line_end, hitsound);
        if (next == p || (next < line_end && *next != ',')) {
            return false;
        }

        x = x < 0.0f ? 0.0f : (x > 512.0f ? 512.0f : x);
        y = y < 0.0f ? 0.0f : (y > 512.0f ? 512.0f : y);
        if (!lazer_format) {
            x = static_cast<float>(static_cast<fosu::i32>(x));
            y = static_cast<float>(static_cast<fosu::i32>(y));
        }

        out.object.x = x;
        out.object.y = y;
        out.object.type = static_cast<fosu::u32>(type);
        out.object.hitsound = static_cast<fosu::u32>(hitsound);
        out.object.time = time;
        out.object.end_time = 0;
        out.object.slider = HitObject::kNoSlider;
        out.object.new_combo = false;
        out.object.combo_skip = 0;
        out.object.hit_sample = nonstd::string_view();
        out.rest = next;
        return true;
    }

    // Anything else the scalar prefix parser rejects is malformed.
    inline bool parse_hitobject_line_scalar(Beatmap& beatmap, HitObjectCounts& counts, const char* p,
                                            const char* line_end, bool lazer_format, Client::Value client,
                                            const HitObjectParseConstants& constants, HitObject& out) {
        HitObjectPrefix prefix;
        if (!parse_hitobject_prefix(p, line_end, lazer_format, client, prefix)) {
            return false;
        }
        ++beatmap.stats.slow_path_lines;
        if (!parse_hitobject_details(beatmap, counts, prefix.object, prefix.rest, line_end, lazer_format,
                                     client, constants)) {
            return false;
        }
        out = prefix.object;
        return true;
    }

    // osu! (lazer) creates a circle once its prefix parses, and a spinner once its end time also
    // parses, before reading the hit sample. A line rejected only for its sample therefore still
    // counts as the previous object when the next one decides whether it starts a new combo.
    // `rest` follows hitSound.
    inline bool preceding_spinner_after_rejection(fosu::u32 type, bool preceding_was_spinner,
                                                  const char* rest, const char* line_end) {
        switch (classify_hitobject_kind(type)) {
        case HitObjectKind::Circle: return false;
        case HitObjectKind::Spinner: {
            if (rest == line_end || *rest != ',') {
                return preceding_was_spinner;
            }
            double end_time;
            const char* next = parse_osu_double(rest + 1, line_end, end_time);
            return (next != rest + 1 && (next == line_end || *next == ',')) || preceding_was_spinner;
        }
        default: return preceding_was_spinner;
        }
    }

    // osu!stable cannot load a map with a hit object it fails to read, but ignores one whose type
    // names no kind. Lazer skips both.
    inline void reject_hitobject(Client::Value client, HitObjectCounts& counts, bool& preceding_was_spinner,
                                 fosu::u32 type, const char* line, const char* rest, const char* line_end) {
        if (client == Client::Stable) {
            if (!counts.unloadable_line && classify_hitobject_kind(type) != HitObjectKind::Invalid) {
                counts.unloadable_line = line;
            }
        } else {
            preceding_was_spinner =
                preceding_spinner_after_rejection(type, preceding_was_spinner, rest, line_end);
        }
    }

    FOSU_NOINLINE inline void reject_hitobject_line(Beatmap& beatmap, HitObjectCounts& counts,
                                                    bool& preceding_was_spinner, const char* p,
                                                    const char* line_end, bool lazer_format,
                                                    Client::Value client) {
        ++beatmap.stats.malformed_lines;
        HitObjectPrefix prefix;
        if (parse_hitobject_prefix(p, line_end, lazer_format, client, prefix)) {
            reject_hitobject(client, counts, preceding_was_spinner, prefix.object.type, p, prefix.rest,
                             line_end);
        } else if (client == Client::Stable && !counts.unloadable_line) {
            counts.unloadable_line = p;
        }
    }

    // Lines osu!stable skips before reading a hit object: blank, comments, and those indented with
    // ' ' or '_'.
    inline bool ignored_hitobject_line(Client::Value client, const char* p, const char* line_end) {
        if (client == Client::Stable && (*p == ' ' || *p == '_')) {
            return true;
        }
        return ignored_line(p, line_end);
    }

    // Interpret a successfully decoded record of `kind` before publishing it to the arena. The
    // preceding object is in source order, including across repeated HitObjects sections.
    inline HitObject normalize_hitobject(HitObject object, HitObjectKind::Value kind, size_t preceding_count,
                                         bool preceding_was_spinner, int offset) {
        const bool explicit_combo = (object.type & 4) != 0;
        object.time += offset;
        object.new_combo = false;
        object.combo_skip = 0;
        if (kind == HitObjectKind::Circle || kind == HitObjectKind::Slider) {
            object.new_combo = !preceding_count || explicit_combo || preceding_was_spinner;
            object.combo_skip = explicit_combo ? static_cast<fosu::u8>((object.type >> 4) & 7) : 0;
            object.end_time = kind == HitObjectKind::Circle ? object.time : 0;
        } else if (kind == HitObjectKind::Spinner) {
            object.new_combo = explicit_combo;
            object.x = 256;
            object.y = 192;
            object.end_time = std::max(object.time, object.end_time + offset);
        } else {
            // Legacy holds clamp against the offset start before offsetting the end.
            object.end_time = std::max(object.time, object.end_time) + offset;
        }
        return object;
    }

#if FOSU_SIMD
    // Publishes an object whose x,y,time,type,hitSound prefix is decoded. `rest` is everything
    // after the prefix: empty, or starting with ','.
    FOSU_ALWAYS_INLINE void accept_hitobject(Beatmap& beatmap, HitObjectCounts& counts,
                                             const HitObjectParseConstants& constants,
                                             bool& preceding_was_spinner, HitObject object, const char* line,
                                             const char* rest, const char* line_end, bool lazer_format,
                                             Client::Value client, int time_offset) {
        const HitObjectKind::Value kind = classify_hitobject_kind(object.type);
        const size_t rest_length = static_cast<size_t>(line_end - rest);
        // Most objects are circles with no sample or the editor's "0:0:0:0:".
        if (kind == HitObjectKind::Circle &&
            (rest_length == 0 || (rest_length == 9 && short_sample(rest + 1)))) {
            if (rest_length) {
                object.hit_sample = nonstd::string_view(rest + 1, 8);
            }
        } else if (!(kind == HitObjectKind::Slider
                         ? (rest_length && parse_slider(beatmap, counts, object, rest + 1, line_end,
                                                        lazer_format, client, constants))
                         : parse_hitobject_details(beatmap, counts, object, rest, line_end, lazer_format,
                                                   client, constants))) {
            ++beatmap.stats.malformed_lines;
            reject_hitobject(client, counts, preceding_was_spinner, object.type, line, rest, line_end);
            return;
        }
        const size_t count = counts.objects++;
        beatmap.hit_objects[count] =
            normalize_hitobject(object, kind, count, preceding_was_spinner, time_offset);
        preceding_was_spinner = kind == HitObjectKind::Spinner;
    }

    // Prefix shapes for one time width, keyed by the comma bits of a line's first eight bytes,
    // which always include the commas after 1-3 digit x and y. Each entry also precomputes the
    // common case of one-digit type and hitSound: its delimiter mask (including the byte after
    // hitSound), prefix end and shuffle index. x == 0 marks any other key.
    struct PrefixLayout {
        u8 x, y, common_end;
        u16 common_index;
        u32 common_delimiters;
    };

    // Upstream builds this at compile time with `consteval`; the port fills the table once at
    // start-up in the PrefixLayoutTable constructor below.
    template <u32 TimeDigits>
    inline void fill_prefix_layouts(PrefixLayout* out) {
        for (u32 i = 0; i < 256; ++i) {
            out[i].x = 0;
            out[i].y = 0;
            out[i].common_end = 0;
            out[i].common_index = 0;
            out[i].common_delimiters = 0;
        }
        for (u32 x = 1; x <= 3; ++x) {
            for (u32 y = 1; y <= 3; ++y) {
                const u32 type_at = x + y + TimeDigits + 3;
                const u32 delimiters = (1u << x) | (1u << (x + y + 1)) | (1u << (type_at - 1)) |
                                       (1u << (type_at + 1)) | (1u << (type_at + 3));
                // The byte after hitSound may be a line ending rather than a comma, so it must
                // lie outside the key.
                if (type_at + 3 < 8) {
                    continue;
                }
                PrefixLayout& layout = out[delimiters & 0xff];
                layout.x = static_cast<u8>(x);
                layout.y = static_cast<u8>(y);
                layout.common_end = static_cast<u8>(type_at + 3);
                layout.common_index = static_cast<u16>((((x - 1) * 3 + y - 1) * 10 + TimeDigits - 1) * 3 * 2);
                layout.common_delimiters = delimiters;
            }
        }
    }

    template <u32 TimeDigits>
    struct PrefixLayoutTable {
        PrefixLayout entries[256];
        PrefixLayoutTable() { fill_prefix_layouts<TimeDigits>(entries); }
    };

    template <u32 TimeDigits>
    inline const PrefixLayout* kPrefixLayouts() {
        static const PrefixLayoutTable<TimeDigits> table;
        return table.entries;
    }

    // Parses lines whose time has exactly TimeDigits digits, with 1-3 digit x and y, 1-3 digit
    // type and 1-2 digit hitSound. Times only grow through a map, so one width usually covers a
    // long run of lines. Returns at the first other line.
    template <u32 TimeDigits>
    FOSU_NOINLINE const char* parse_hitobjects_fixed_time(Beatmap& beatmap, HitObjectCounts& counts,
                                                          const HitObjectParseConstants& constants,
                                                          bool& preceding_was_spinner_ref, const char* p,
                                                          const char* file_end, bool lazer_format,
                                                          Client::Value client, int time_offset) {
        bool preceding_was_spinner = preceding_was_spinner_ref;
        u32 fast_lines = 0;
        while (p < file_end) {
            const Bytes32 ascii = load32(p);
            const u32 commas = equal_mask32(ascii, constants.comma);
            const u32 nondigits = nondigit_mask32(ascii);

            const PrefixLayout& layout = kPrefixLayouts<TimeDigits>()[commas & 0xff];
            u32 prefix_end = layout.common_end;
            u32 mask_index = layout.common_index;
            const u32 common = (2u << prefix_end) - 1; // through the byte after
            if ((nondigits & common) != layout.common_delimiters ||
                (commas & layout.common_delimiters & (common >> 1)) !=
                    (layout.common_delimiters & (common >> 1))) {
                // Longer type or hitSound: measure both digit runs.
                const u32 type_at = layout.x + layout.y + TimeDigits + 3;
                const u32 type_digits = trailing_zeros(nondigits >> type_at);
                if (!layout.x || type_digits - 1 > 2) {
                    break;
                }
                const u32 sound_at = type_at + type_digits + 1;
                const u32 sound_digits = trailing_zeros(nondigits >> sound_at);
                if (sound_digits - 1 > 1) {
                    break;
                }
                prefix_end = sound_at + sound_digits;
                const u32 delimiters = (1u << layout.x) | (1u << (layout.x + layout.y + 1)) |
                                       (1u << (type_at - 1)) | (1u << (type_at + type_digits));
                if ((nondigits & ((1u << prefix_end) - 1)) != delimiters ||
                    (commas & delimiters) != delimiters) {
                    break;
                }
                mask_index += (type_digits - 1) * 2 + sound_digits - 1;
            }
            // A NUL is the zero padding only at the end of the input.
            const char after = p[prefix_end];
            if (after != ',' && after != '\r' && after != '\n' &&
                (after != '\0' || p + prefix_end != file_end)) {
                break;
            }

            u32 fields[4];
            double time;
#if FOSU_SIMD_X86
            const LaneMasks& masks = kLaneMasks[mask_index];
            const __m256i placed = _mm256_shuffle_epi8(
                _mm256_permutevar8x32_epi32(_mm256_sub_epi8(ascii, constants.zero),
                                            _mm256_load_si256(reinterpret_cast<const __m256i*>(masks.perm))),
                _mm256_load_si256(reinterpret_cast<const __m256i*>(masks.shuf)));
            const __m256i values = _mm256_madd_epi16(
                _mm256_maddubs_epi16(placed, _mm256_setr_epi8(0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1, 0,
                                                              0, 10, 1, 0, 0, 10, 1, 0, 0, 10, 1, 10, 1, 10,
                                                              1, 10, 1, 10, 1)),
                _mm256_setr_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 100, 1, 100, 1));
            const __m128i time_groups = _mm256_extracti128_si256(values, 1);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(fields),
                             _mm_add_epi32(_mm256_castsi256_si128(values), _mm_slli_si128(time_groups, 12)));
            const __m128i packed = _mm_packus_epi32(time_groups, time_groups);
            const __m128i combined = _mm_madd_epi16(packed, _mm_setr_epi16(0, 0, 10000, 1, 0, 0, 0, 0));
            _mm_store_sd(&time, _mm_cvtepi32_pd(_mm_shuffle_epi32(combined, _MM_SHUFFLE(0, 0, 0, 1))));
#else
            Bytes32 digits;
            digits.val[0] = vsubq_u8(ascii.val[0], constants.zero);
            digits.val[1] = vsubq_u8(ascii.val[1], constants.zero);
            const PrefixShuffle& masks = kPrefixShuffles[mask_index];
            vst1q_u32(fields, decimal_groups(vqtbl2q_u8(digits, vld1q_u8(masks.bytes))));
            const uint32x4_t time_groups = decimal_groups(vqtbl2q_u8(digits, vld1q_u8(masks.bytes + 16)));
            const u32 weights[2] = {10000, 1};
            const uint32x2_t terms = vmul_u32(vget_high_u32(time_groups), vld1_u32(weights));
            time = vgetq_lane_f64(vcvtq_f64_u64(vmovl_u32(vpadd_u32(terms, terms))), 0);
#endif

            const u32 first_ending = first_line_end32(ascii);
            const char* line_end = first_ending < 32 ? p + first_ending : find_line_end(p + 32, file_end);
            fields[0] = clamp_value(fields[0], static_cast<u32>(0), static_cast<u32>(512));
            fields[1] = clamp_value(fields[1], static_cast<u32>(0), static_cast<u32>(512));
            ++fast_lines;
            HitObject object;
            object.x = static_cast<float>(fields[0]);
            object.y = static_cast<float>(fields[1]);
            object.type = fields[2];
            object.hitsound = fields[3];
            object.time = time;
            object.end_time = 0;
            object.slider = HitObject::kNoSlider;
            object.new_combo = false;
            object.combo_skip = 0;
            object.hit_sample = nonstd::string_view();
            accept_hitobject(beatmap, counts, constants, preceding_was_spinner, object, p, p + prefix_end,
                             line_end, lazer_format, client, time_offset);
            p = after_line_ending(line_end, file_end);
        }
        beatmap.stats.fast_path_lines += fast_lines;
        preceding_was_spinner_ref = preceding_was_spinner;
        return p;
    }
#endif

    inline const char* parse_hitobjects_section(Beatmap& beatmap, HitObjectCounts& counts, const char* p,
                                                const char* file_end, const ParseContext& context) {
        const bool lazer_format = context.format.lazer_format;
        const int time_offset = context.format.time_offset;
        const Client::Value client = context.client;
        const HitObjectParseConstants constants;
        bool preceding_was_spinner = counts.preceding_was_spinner;
        while (p < file_end) {
#if FOSU_SIMD
            p = parse_hitobjects_fixed_time<1>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            p = parse_hitobjects_fixed_time<2>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            p = parse_hitobjects_fixed_time<3>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            p = parse_hitobjects_fixed_time<4>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            p = parse_hitobjects_fixed_time<5>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            p = parse_hitobjects_fixed_time<6>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            p = parse_hitobjects_fixed_time<7>(beatmap, counts, constants, preceding_was_spinner, p, file_end,
                                               lazer_format, client, time_offset);
            if (p >= file_end) {
                break;
            }
#endif
            const char c = *p;
            if (c == '\r' || c == '\n') {
                ++p;
                continue;
            }
            const char* line_end = find_line_end(p, file_end);
            if (c == '[' && section_header_line(p, line_end, client)) {
                break;
            }
            if (!ignored_hitobject_line(client, p, line_end)) {
                HitObject object;
                if (parse_hitobject_line_scalar(beatmap, counts, p, line_end, lazer_format, client, constants,
                                                object)) {
                    const size_t count = counts.objects++;
                    const HitObjectKind::Value kind = classify_hitobject_kind(object.type);
                    beatmap.hit_objects[count] =
                        normalize_hitobject(object, kind, count, preceding_was_spinner, time_offset);
                    preceding_was_spinner = kind == HitObjectKind::Spinner;
                } else {
                    reject_hitobject_line(beatmap, counts, preceding_was_spinner, p, line_end, lazer_format,
                                          client);
                }
            }
            p = after_line_ending(line_end, file_end);
        }
        counts.preceding_was_spinner = preceding_was_spinner;
        return p;
    }
}} // namespace fosu::internal

#endif
