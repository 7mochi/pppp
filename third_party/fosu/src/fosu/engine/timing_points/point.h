#ifndef FOSU_ENGINE_TIMING_POINTS_POINT_H
#define FOSU_ENGINE_TIMING_POINTS_POINT_H

#include <fosu/beatmap.h>
#include <fosu/compiler.h>
#include <fosu/engine/parsing/sample_sets.h>
#include <fosu/engine/primitives/byte_scan.h>
#include <fosu/engine/timing_points/beat_length.h>

namespace fosu { namespace internal {

    // Parse a timing point, including omitted legacy fields. A present field must parse
    // completely; NaN is meaningful only when inherited. Invalid lines return false.
    inline bool parse_timing_point(const char* p, const char* end, int time_offset, TimingPoint& out) {
        double time, beat_length;
        const char* q = parse_osu_double(p, end, time);
        if (q == p || q >= end || *q != ',') {
            return false;
        }
        p = q + 1;
        q = parse_beat_length(p, end, beat_length);
        if (q == p) {
            return false;
        }
        p = q;
        fosu_int64 rest[6] = {4, 0, 0, 100, 1, 0};
        for (int i = 0; i < 6 && p < end; ++i) {
            if (*p++ != ',') {
                return false;
            }
            const char* field_end = find_byte(',', p, end);
            if (p == field_end) {
                return false;
            }
            if (i == 4) {
                rest[i] = *p == '1';
            } else if (i == 0 && *p == '0') {
                // The official decoder treats any meter field beginning in 0 as
                // 4/4. Preserve a plain raw zero; use 4 for nonnumeric spellings.
                q = parse_osu_int(p, field_end, rest[i]);
                if (q != field_end) {
                    rest[i] = 4;
                }
            } else {
                q = parse_osu_int(p, field_end, rest[i]);
                if (q == p || q != field_end || (i == 0 && rest[i] <= 0)) {
                    return false;
                }
            }
            p = field_end;
        }
        // Additional legacy columns are ignored by the official decoder.
        if ((p < end && *p != ',') || (rest[4] != 0 && beat_length != beat_length)) {
            return false;
        }
        const Optional<SampleSet::Value> sample_set = parse_sample_set(rest[1]);
        if (!sample_set.has_value()) {
            return false;
        }
        out.time = time + time_offset;
        out.beat_length = beat_length;
        out.meter = clamp_i32(rest[0]);
        out.sample_set = *sample_set;
        out.sample_index = clamp_i32(rest[2]);
        out.volume = clamp_i32(rest[3]);
        out.uninherited = rest[4] != 0;
        out.effects = static_cast<fosu_uint32>(rest[5]);
        return true;
    }

}} // namespace fosu::internal

#endif
