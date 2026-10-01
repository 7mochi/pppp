// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/mania/patterns/pass_through_pattern_generator.h"

namespace pppp { namespace mania { namespace patterns {
    void PassThroughPatternGenerator::generate(std::vector<Pattern>& out) {
        const int column = get_column(static_cast<float>(hit_object.position.x), false);

        Pattern pattern;
        if (hit_object.has_duration) {
            pattern.add(make_hold(column, hit_object.start_time, hit_object.end_time));
        } else {
            pattern.add(make_note(column, hit_object.start_time));
        }

        out.push_back(pattern);
    }
}}} // namespace pppp::mania::patterns
