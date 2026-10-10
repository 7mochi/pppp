#ifndef FOSU_ENGINE_PARSING_SAMPLE_SETS_H
#define FOSU_ENGINE_PARSING_SAMPLE_SETS_H

#include <fosu/compiler.h>
#include <fosu/enums.h>
#include <fosu/nonstd_config.h>

namespace fosu { namespace internal {
    inline nonstd::optional<SampleSet::Value> parse_sample_set(fosu::i64 value) {
        if (value < 0 || value > 3) {
            return nonstd::optional<SampleSet::Value>();
        }
        return nonstd::optional<SampleSet::Value>(static_cast<SampleSet::Value>(value));
    }
}} // namespace fosu::internal

#endif
