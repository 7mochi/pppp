// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/preprocessing/rhythm/taiko_rhythm_difficulty_preprocessor.h"
#include "pppp/taiko/difficulty/utils/delta_time_normaliser.h"
#include "pppp/utils/math/csharp.h"
#include <cmath>
#include <limits>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace rhythm {
    data::SameRhythmHitObjectGrouping::SameRhythmHitObjectGrouping(
        SameRhythmHitObjectGrouping* previous, const std::vector<TaikoDifficultyHitObject*>& hit_objects)
        : hit_objects(hit_objects),
          previous(previous),
          hit_object_interval_ratio(1.0),
          interval(std::numeric_limits<double>::infinity()) {
        std::vector<double> delta_times;
        delta_times.reserve(hit_objects.size());
        for (size_t i = 0; i < hit_objects.size(); i++) {
            delta_times.push_back(hit_objects[i]->delta_time);
        }

        std::vector<double> normalised;
        utils::normalise_delta_times(normalised, delta_times, SNAP_TOLERANCE);

        const bool has_delta = normalised.size() > 1;
        const double modal_delta = has_delta ? pppp::utils::math::round_half_even(normalised[1]) : 0.0;

        if (has_delta) {
            if (previous && previous->hit_object_interval.has_value() &&
                std::fabs(modal_delta - previous->hit_object_interval.value()) <= SNAP_TOLERANCE) {
                hit_object_interval = previous->hit_object_interval.value();
            } else {
                hit_object_interval = modal_delta;
            }
        }

        hit_object_interval_ratio =
            previous && previous->hit_object_interval.has_value() && hit_object_interval.has_value()
                ? hit_object_interval.value() / previous->hit_object_interval.value()
                : 1.0;

        if (previous) {
            const double start = hit_objects[0]->start_time;
            const double previous_start = previous->hit_objects[0]->start_time;
            interval = std::fabs(start - previous_start) <= SNAP_TOLERANCE ? 0.0 : start - previous_start;
        }
    }

    double data::SamePatternsGroupedHitObjects::group_interval() const {
        return groups.size() > 1 ? groups[1]->interval : groups[0]->interval;
    }

    double data::SamePatternsGroupedHitObjects::interval_ratio() const {
        if (!previous) {
            return 1.0;
        }
        return group_interval() / previous->group_interval();
    }

    void process_and_assign(const std::vector<TaikoDifficultyHitObject*>& note_objects,
                            std::vector<SameRhythmHitObjectGrouping>& rhythm_groupings,
                            std::vector<SamePatternsGroupedHitObjects>& pattern_groupings) {
        rhythm_groupings.clear();
        pattern_groupings.clear();

        if (note_objects.empty()) {
            return;
        }

        std::vector<double> note_intervals;
        note_intervals.reserve(note_objects.size());
        for (size_t i = 0; i < note_objects.size(); i++) {
            note_intervals.push_back(note_objects[i]->delta_time);
        }

        std::vector<std::vector<int> > grouped;
        utils::group_by_interval(grouped, note_intervals);

        rhythm_groupings.reserve(grouped.size());

        for (size_t g = 0; g < grouped.size(); g++) {
            std::vector<TaikoDifficultyHitObject*> group_hit_objects;
            group_hit_objects.reserve(grouped[g].size());
            for (size_t k = 0; k < grouped[g].size(); k++) {
                group_hit_objects.push_back(note_objects[static_cast<size_t>(grouped[g][k])]);
            }

            const size_t index = rhythm_groupings.size();
            SameRhythmHitObjectGrouping* previous = index > 0 ? &rhythm_groupings[index - 1] : 0;
            rhythm_groupings.push_back(SameRhythmHitObjectGrouping(previous, group_hit_objects));

            for (size_t k = 0; k < rhythm_groupings[index].hit_objects.size(); k++) {
                rhythm_groupings[index].hit_objects[k]->rhythm.same_rhythm_grouping =
                    &rhythm_groupings[index];
            }
        }

        std::vector<double> group_intervals;
        group_intervals.reserve(rhythm_groupings.size());
        for (size_t i = 0; i < rhythm_groupings.size(); i++) {
            group_intervals.push_back(rhythm_groupings[i].interval);
        }

        std::vector<std::vector<int> > pattern_grouped;
        utils::group_by_interval(pattern_grouped, group_intervals);

        pattern_groupings.reserve(pattern_grouped.size());

        for (size_t g = 0; g < pattern_grouped.size(); g++) {
            SamePatternsGroupedHitObjects pattern;
            pattern.previous = g > 0 ? &pattern_groupings[g - 1] : 0;
            for (size_t k = 0; k < pattern_grouped[g].size(); k++) {
                pattern.groups.push_back(&rhythm_groupings[static_cast<size_t>(pattern_grouped[g][k])]);
            }
            pattern_groupings.push_back(pattern);

            for (size_t k = 0; k < pattern_groupings[g].groups.size(); k++) {
                SameRhythmHitObjectGrouping* rhythm_group = pattern_groupings[g].groups[k];
                for (size_t o = 0; o < rhythm_group->hit_objects.size(); o++) {
                    rhythm_group->hit_objects[o]->rhythm.same_patterns_grouping = &pattern_groupings[g];
                }
            }
        }
    }
}}}}} // namespace pppp::taiko::difficulty::preprocessing::rhythm
