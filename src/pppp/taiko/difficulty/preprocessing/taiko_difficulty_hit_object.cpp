// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"
#include "pppp/taiko/taiko_hit_windows.h"
#include "pppp/utils/linq/csharp.h"
#include <cmath>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing {
    namespace linq = ::pppp::utils::linq;

    TaikoDifficultyHitObject::TaikoDifficultyHitObject(
        const TaikoHitObject& object, const TaikoHitObject& last, double rate,
        const std::vector<DifficultyHitObject*>& objects,
        std::vector<TaikoDifficultyHitObject*>& centre_objects_in,
        std::vector<TaikoDifficultyHitObject*>& rim_objects_in,
        std::vector<TaikoDifficultyHitObject*>& note_objects_in, int object_index, const TaikoBeatmap& pb)
        : DifficultyHitObject(object.time, last.time, rate, objects, object_index),
          kind(object::OBJECT_HIT),
          type(object::HIT_TYPE_CENTRE),
          mono_objects(0),
          note_objects(&note_objects_in),
          mono_index(-1),
          note_index(0),
          effective_bpm(0.0) {
        TaikoHitWindows hit_windows;
        hit_windows.set_difficulty(pb.overall_difficulty);

        end_time = (object.time + object.duration) / rate;
        // Drum rolls and swells carry empty hit windows, and so do their nested objects.
        hit_window_great = object.kind == object::OBJECT_HIT ? 2.0 * hit_windows.great / rate : 0.0;

        kind = object.kind;
        type = object.type;

        if (object.kind == object::OBJECT_HIT) {
            std::vector<TaikoDifficultyHitObject*>& mono =
                object.type == object::HIT_TYPE_CENTRE ? centre_objects_in : rim_objects_in;
            mono_objects = &mono;
            mono_index = static_cast<int>(mono.size());

            note_index = static_cast<int>(note_objects_in.size());
        }

        if (index >= 1) {
            const TaikoDifficultyHitObject* previous =
                static_cast<const TaikoDifficultyHitObject*>(this->previous(0));
            double actual_ratio = delta_time / previous->delta_time;
            double closest_ratio =
                linq::from(rhythm::COMMON_RATIOS)
                    .min_by(linq::bind(static_cast<double (*)(double)>(&std::fabs), linq::_1 - actual_ratio));

            rhythm.ratio = closest_ratio;
        }

        // Using `hitObject.StartTime` causes floating point error differences
        double normalised_start_time = start_time * rate;
        // Retrieve the timing point at the note's start time
        double beat_length = beat_length_at(pb, normalised_start_time);
        double scroll_speed = scroll_speed_at(pb, normalised_start_time);
        // Calculate the slider velocity at the note's start time.
        double slider_velocity = pb.slider_multiplier * scroll_speed * rate;
        effective_bpm = (60000.0 / beat_length) * slider_velocity;
    }

    TaikoDifficultyHitObject* TaikoDifficultyHitObject::previous_mono(int backwards) const {
        if (mono_objects == 0) {
            return 0;
        }
        return linq::from(*mono_objects).element_at_or_default(mono_index - (backwards + 1));
    }

    TaikoDifficultyHitObject* TaikoDifficultyHitObject::previous_note(int backwards) const {
        return linq::from(*note_objects).element_at_or_default(note_index - (backwards + 1));
    }

    TaikoDifficultyHitObject* TaikoDifficultyHitObject::next_note(int forwards) const {
        return linq::from(*note_objects).element_at_or_default(note_index + (forwards + 1));
    }

    double rhythm::data::SameRhythmHitObjectGrouping::start_time() const {
        return hit_objects[0]->start_time;
    }

    double rhythm::data::SameRhythmHitObjectGrouping::duration() const {
        return hit_objects[hit_objects.size() - 1]->start_time - hit_objects[0]->start_time;
    }
}}}} // namespace pppp::taiko::difficulty::preprocessing
