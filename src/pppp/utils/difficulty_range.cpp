// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/utils/difficulty_range.h"
#include "pppp/utils/math/csharp.h"

namespace pppp { namespace utils {
    double difficulty_range(double difficulty, double min_val, double mid, double max_val) {
        if (difficulty > 5.0) {
            return mid + (max_val - mid) * difficulty_range(difficulty);
        }
        if (difficulty < 5.0) {
            return mid + (mid - min_val) * difficulty_range(difficulty);
        }
        return mid;
    }

    double difficulty_range(double difficulty) { return (difficulty - 5.0) / 5.0; }

    double difficulty_range(double difficulty, const DifficultyRange& range) {
        return difficulty_range(difficulty, range.min, range.mid, range.max);
    }

    int difficulty_range_int(double difficulty, const DifficultyRange& range) {
        return static_cast<int>(difficulty_range(difficulty, range.min, range.mid, range.max));
    }

    double inverse_difficulty_range(double difficulty_value, double diff0, double diff5, double diff10) {
        return math::sign(difficulty_value - diff5) == math::sign(diff10 - diff5)
                   ? (difficulty_value - diff5) / (diff10 - diff5) * 5.0 + 5.0
                   : (difficulty_value - diff5) / (diff5 - diff0) * 5.0 + 5.0;
    }
}} // namespace pppp::utils
