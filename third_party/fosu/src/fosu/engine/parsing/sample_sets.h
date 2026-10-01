#ifndef FOSU_ENGINE_PARSING_SAMPLE_SETS_H
#define FOSU_ENGINE_PARSING_SAMPLE_SETS_H

#include <fosu/compiler.h>
#include <fosu/enums.h>
#include <fosu/span.h>

namespace fosu { namespace internal {

    inline Optional<SampleSet::Value> parse_sample_set(fosu_int64 value) {
        if (value < 0 || value > 3) {
            return Optional<SampleSet::Value>();
        }
        return Optional<SampleSet::Value>(static_cast<SampleSet::Value>(value));
    }

}} // namespace fosu::internal

#endif
