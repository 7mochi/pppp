#ifndef FOSU_ENGINE_HIT_OBJECTS_OBJECT_TYPES_H
#define FOSU_ENGINE_HIT_OBJECTS_OBJECT_TYPES_H

#include <fosu/engine/hit_objects/samples.h>
#include <fosu/engine/primitives/packed_digits.h>

namespace fosu { namespace internal {

    struct HitObjectKind {
        enum Value { Circle, Slider, Spinner, Hold, Invalid };
    };

    // A type value may contain several kind bits. The official decoder resolves
    // them in this order while leaving combo and colour-skip bits untouched.
    inline HitObjectKind::Value classify_hitobject_kind(fosu_uint32 type) {
        if (type & 1) {
            return HitObjectKind::Circle;
        }
        if (type & 2) {
            return HitObjectKind::Slider;
        }
        if (type & 8) {
            return HitObjectKind::Spinner;
        }
        if (type & 128) {
            return HitObjectKind::Hold;
        }
        return HitObjectKind::Invalid;
    }

    struct CircleDetails {
        StringView hit_sample;
    };

    inline bool parse_circle_details(const char* p, const char* end, CircleDetails& out) {
        out.hit_sample = StringView();
        if (p == end) {
            return true;
        }
        if (*p != ',') {
            return false;
        }
        return parse_hit_sample(p + 1, end, out.hit_sample);
    }

    struct TimedHitObjectDetails {
        double end_time;
        StringView hit_sample;
    };

    // Raw timestamps remain unshifted and unclamped.
    inline bool parse_spinner_details(const char* p, const char* end, TimedHitObjectDetails& out) {
        if (p == end || *p != ',') {
            return false;
        }
        double end_time;
        const char* next = parse_osu_double(p + 1, end, end_time);
        if (next == p + 1 || (next < end && *next != ',')) {
            return false;
        }
        StringView sample;
        if (!parse_hit_sample(next < end ? next + 1 : end, end, sample)) {
            return false;
        }
        out.end_time = end_time;
        out.hit_sample = sample;
        return true;
    }

    // Omitted endpoints and the ':' separator follow ConvertHitObjectParser.
    inline bool parse_hold_details(double start_time, const char* p, const char* end,
                                   TimedHitObjectDetails& out) {
        out.end_time = start_time;
        out.hit_sample = StringView();
        if (p == end || (p + 1 == end && *p == ',')) {
            return true;
        }
        if (*p != ',') {
            return false;
        }
        double end_time = 0;
        const char* field = p + 1;
        const char* next = field;
        const fosu_uint32 digits = digit_run8(field);
        if (digits && digits <= static_cast<size_t>(end - field) &&
            (field + digits == end || field[digits] == ',' || field[digits] == ':')) {
            const fosu_uint64 value = swar_parse_u64(field, digits);
            if (value <= static_cast<fosu_uint64>(kInt32Max)) {
                end_time = static_cast<double>(value);
                next = field + digits;
            }
        }
        if (next == field) {
            next = parse_osu_double(field, end, end_time);
        }
        if (next == p + 1 || (next < end && *next != ',' && *next != ':')) {
            return false;
        }
        // The official decoder ignores a comma-separated value here; a hold's
        // hit sample belongs after the ':' in objectParams.
        out.end_time = end_time;
        if (next < end && *next == ',') {
            return true;
        }
        return parse_hit_sample(next < end ? next + 1 : end, end, out.hit_sample);
    }

}} // namespace fosu::internal

#endif
