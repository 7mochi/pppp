// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/preprocessing/colour/taiko_colour_difficulty_preprocessor.h"
#include <algorithm>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing { namespace colour {
    const int data::RepeatingHitPatterns::MAX_REPETITION_INTERVAL;

    nonstd::optional<object::HitType> data::MonoStreak::hit_type() const {
        const TaikoDifficultyHitObject* first = hit_objects[0];
        if (!first->is_hit()) {
            return nonstd::optional<object::HitType>();
        }
        return nonstd::optional<object::HitType>(first->type);
    }

    bool data::AlternatingMonoPattern::has_identical_mono_length(const AlternatingMonoPattern& other) const {
        return other.mono_streaks[0]->run_length() == mono_streaks[0]->run_length();
    }

    bool data::AlternatingMonoPattern::is_repetition_of(const AlternatingMonoPattern& other) const {
        return has_identical_mono_length(other) && other.mono_streaks.size() == mono_streaks.size() &&
               other.mono_streaks[0]->hit_type() == mono_streaks[0]->hit_type();
    }

    bool data::RepeatingHitPatterns::is_repetition_of(const RepeatingHitPatterns& other) const {
        if (alternating_mono_patterns.size() != other.alternating_mono_patterns.size()) {
            return false;
        }

        size_t limit = std::min<size_t>(alternating_mono_patterns.size(), 2);
        for (size_t i = 0; i < limit; i++) {
            if (!alternating_mono_patterns[i]->has_identical_mono_length(
                    *other.alternating_mono_patterns[i])) {
                return false;
            }
        }
        return true;
    }

    void data::RepeatingHitPatterns::find_repetition_interval() {
        if (!previous) {
            repetition_interval = MAX_REPETITION_INTERVAL + 1;
            return;
        }

        const RepeatingHitPatterns* other = previous;
        int interval = 1;
        while (interval < MAX_REPETITION_INTERVAL) {
            if (is_repetition_of(*other)) {
                repetition_interval = std::min(interval, MAX_REPETITION_INTERVAL);
                return;
            }

            other = other->previous;
            if (!other) {
                break;
            }
            interval++;
        }

        repetition_interval = MAX_REPETITION_INTERVAL + 1;
    }

    namespace {
        void encode_mono_streaks(std::vector<TaikoDifficultyHitObject>& objects,
                                 std::vector<MonoStreak>& mono_streaks) {
            nonstd::optional<size_t> current;

            for (size_t i = 0; i < objects.size(); i++) {
                TaikoDifficultyHitObject& object = objects[i];

                // This ignores all non-note objects, which may or may not be the desired behaviour
                const TaikoDifficultyHitObject* previous = object.previous_note(0);

                // If this is the first object in the list or the colour changed, create a new mono streak
                bool colour_changed = true;
                if (current.has_value() && previous) {
                    bool this_is_hit = object.is_hit();
                    bool previous_is_hit = previous->is_hit();
                    colour_changed =
                        this_is_hit != previous_is_hit || (this_is_hit && object.type != previous->type);
                }

                if (colour_changed) {
                    mono_streaks.push_back(MonoStreak());
                    current = mono_streaks.size() - 1;
                }

                // Add the current object to the encoded payload.
                mono_streaks[current.value()].hit_objects.push_back(&object);
            }
        }

        void
        encode_alternating_mono_patterns(std::vector<MonoStreak>& mono_streaks,
                                         std::vector<AlternatingMonoPattern>& alternating_mono_patterns) {
            nonstd::optional<size_t> current;

            for (size_t i = 0; i < mono_streaks.size(); i++) {
                // Start a new AlternatingMonoPattern if the previous MonoStreak has a different mono length,
                // or if this is the first MonoStreak in the list.
                if (!current.has_value() ||
                    mono_streaks[i].run_length() != mono_streaks[i - 1].run_length()) {
                    alternating_mono_patterns.push_back(AlternatingMonoPattern());
                    current = alternating_mono_patterns.size() - 1;
                }

                // Add the current MonoStreak to the encoded payload.
                alternating_mono_patterns[current.value()].mono_streaks.push_back(&mono_streaks[i]);
            }
        }

        void encode_repeating_hit_patterns(std::vector<AlternatingMonoPattern>& alternating_mono_patterns,
                                           std::vector<RepeatingHitPatterns>& repeating_hit_patterns) {
            nonstd::optional<size_t> current;
            size_t count = alternating_mono_patterns.size();

            repeating_hit_patterns.reserve(alternating_mono_patterns.size());

            for (size_t i = 0; i < count; i++) {
                // Start a new RepeatingHitPattern. AlternatingMonoPatterns that should be grouped together
                // will be handled later within this loop.
                RepeatingHitPatterns pattern;
                pattern.previous = current.has_value() ? &repeating_hit_patterns[current.value()] : 0;
                repeating_hit_patterns.push_back(pattern);
                current = repeating_hit_patterns.size() - 1;

                // Determine if future AlternatingMonoPatterns should be grouped.
                bool coupled = i + 2 < count && alternating_mono_patterns[i].is_repetition_of(
                                                    alternating_mono_patterns[i + 2]);

                if (!coupled) {
                    // If not, add the current AlternatingMonoPattern to the encoded payload and continue.
                    repeating_hit_patterns[current.value()].alternating_mono_patterns.push_back(
                        &alternating_mono_patterns[i]);
                } else {
                    // If so, add the current AlternatingMonoPattern to the encoded payload and start
                    // repeatedly checking if the subsequent AlternatingMonoPatterns should be grouped by
                    // increasing i and doing the appropriate isCoupled check.
                    while (coupled) {
                        repeating_hit_patterns[current.value()].alternating_mono_patterns.push_back(
                            &alternating_mono_patterns[i]);
                        i++;
                        coupled = i + 2 < count && alternating_mono_patterns[i].is_repetition_of(
                                                       alternating_mono_patterns[i + 2]);
                    }

                    // Skip over viewed data and add the rest to the payload
                    repeating_hit_patterns[current.value()].alternating_mono_patterns.push_back(
                        &alternating_mono_patterns[i]);
                    repeating_hit_patterns[current.value()].alternating_mono_patterns.push_back(
                        &alternating_mono_patterns[i + 1]);
                    i++;
                }
            }

            // Final pass to find repetition intervals
            for (size_t i = 0; i < repeating_hit_patterns.size(); i++) {
                repeating_hit_patterns[i].find_repetition_interval();
            }
        }
    } // namespace

    void process_and_assign(std::vector<TaikoDifficultyHitObject>& objects,
                            std::vector<MonoStreak>& mono_streaks,
                            std::vector<AlternatingMonoPattern>& alternating_mono_patterns,
                            std::vector<RepeatingHitPatterns>& repeating_hit_patterns) {
        mono_streaks.clear();
        alternating_mono_patterns.clear();
        repeating_hit_patterns.clear();

        if (objects.empty()) {
            return;
        }

        encode_mono_streaks(objects, mono_streaks);
        encode_alternating_mono_patterns(mono_streaks, alternating_mono_patterns);
        encode_repeating_hit_patterns(alternating_mono_patterns, repeating_hit_patterns);

        // Assign indexing and encoding data to all relevant objects.
        // The outermost loop is kept a ForEach loop since it doesn't need index information, and we want to
        // keep i and j for AlternatingMonoPattern's and MonoStreak's index respectively, to keep it in line
        // with documentation.
        for (size_t p = 0; p < repeating_hit_patterns.size(); p++) {
            RepeatingHitPatterns& pattern = repeating_hit_patterns[p];

            for (size_t i = 0; i < pattern.alternating_mono_patterns.size(); i++) {
                AlternatingMonoPattern& mono_pattern = *pattern.alternating_mono_patterns[i];
                mono_pattern.parent = &pattern;
                mono_pattern.index = static_cast<int>(i);

                for (size_t j = 0; j < mono_pattern.mono_streaks.size(); j++) {
                    MonoStreak& streak = *mono_pattern.mono_streaks[j];
                    streak.parent = &mono_pattern;
                    streak.index = static_cast<int>(j);

                    for (size_t k = 0; k < streak.hit_objects.size(); k++) {
                        TaikoDifficultyHitObject& object = *streak.hit_objects[k];
                        object.colour.repeating_hit_pattern = &pattern;
                        object.colour.alternating_mono_pattern = &mono_pattern;
                        object.colour.mono_streak = &streak;
                    }
                }
            }
        }
    }

    TaikoDifficultyHitObject* TaikoColourData::previous_colour_change() const {
        if (!mono_streak) {
            return 0;
        }
        return mono_streak->first_hit_object()->previous_note(0);
    }

    TaikoDifficultyHitObject* TaikoColourData::next_colour_change() const {
        if (!mono_streak) {
            return 0;
        }
        return mono_streak->last_hit_object()->next_note(0);
    }
}}}}} // namespace pppp::taiko::difficulty::preprocessing::colour
