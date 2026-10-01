// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_DIFFICULTY_PREPROCESSING_TAIKO_DIFFICULTY_HIT_OBJECT_H
#define PPPP_TAIKO_DIFFICULTY_PREPROCESSING_TAIKO_DIFFICULTY_HIT_OBJECT_H

#include "pppp/common/preprocessing/difficulty_hit_object.h"
#include "pppp/config.h" // IWYU pragma: export
#include "pppp/taiko/difficulty/preprocessing/colour/taiko_colour_data.h"
#include "pppp/taiko/difficulty/preprocessing/rhythm/taiko_rhythm_data.h"
#include "pppp/taiko/object/taiko_hit_object.h"
#include "pppp/taiko/taiko_beatmap.h"
#include <vector>

namespace pppp { namespace taiko { namespace difficulty { namespace preprocessing {
    typedef pppp::common::preprocessing::DifficultyHitObject DifficultyHitObject;

    typedef colour::TaikoColourData ColourData;
    typedef rhythm::TaikoRhythmData RhythmData;
    typedef colour::data::MonoStreak MonoStreak;
    typedef colour::data::AlternatingMonoPattern AlternatingMonoPattern;
    typedef colour::data::RepeatingHitPatterns RepeatingHitPatterns;
    typedef rhythm::data::SameRhythmHitObjectGrouping SameRhythmHitObjectGrouping;
    typedef rhythm::data::SamePatternsGroupedHitObjects SamePatternsGroupedHitObjects;

    /// Represents a single hit object in taiko difficulty calculation.
    struct TaikoDifficultyHitObject : DifficultyHitObject {
        object::ObjectKind kind;
        object::HitType type;

        /// The list of all TaikoDifficultyHitObjects of the same colour as this TaikoDifficultyHitObject
        /// in the beatmap.
        const std::vector<TaikoDifficultyHitObject*>* mono_objects;

        /// The list of all TaikoDifficultyHitObject that is either a regular note or finisher in the beatmap
        const std::vector<TaikoDifficultyHitObject*>* note_objects;

        /// The index of this TaikoDifficultyHitObject in mono_objects.
        int mono_index;

        /// The index of this TaikoDifficultyHitObject in note_objects.
        int note_index;

        /// Colour data used by ColourEvaluator and StaminaEvaluator.
        /// This is populated via TaikoColourDifficultyPreprocessor.
        ColourData colour;

        /// Rhythm data used by RhythmEvaluator.
        /// This is populated via TaikoRhythmDifficultyPreprocessor.
        RhythmData rhythm;

        /// The adjusted BPM of this hit object, based on its slider velocity and scroll speed.
        double effective_bpm;

        /// Creates a new difficulty hit object.
        /// @param object The gameplay TaikoHitObject associated with this difficulty object.
        /// @param last The gameplay TaikoHitObject preceding object.
        /// @param rate The rate of the gameplay clock. Modified by speed-changing mods.
        /// @param objects The list of all DifficultyHitObjects in the current beatmap.
        /// @param centre_objects_in The list of centre (don) DifficultyHitObjects in the current beatmap.
        /// @param rim_objects_in The list of rim (kat) DifficultyHitObjects in the current beatmap.
        /// @param note_objects_in The list of DifficultyHitObjects that is a hit (i.e. not a drumroll or
        /// swell) in the current beatmap.
        /// @param object_index The position of this DifficultyHitObject in the objects list.
        /// @param pb The control point info and the global slider velocity of the beatmap.
        TaikoDifficultyHitObject(const TaikoHitObject& object, const TaikoHitObject& last, double rate,
                                 const std::vector<DifficultyHitObject*>& objects,
                                 std::vector<TaikoDifficultyHitObject*>& centre_objects_in,
                                 std::vector<TaikoDifficultyHitObject*>& rim_objects_in,
                                 std::vector<TaikoDifficultyHitObject*>& note_objects_in, int object_index,
                                 const TaikoBeatmap& pb);

        bool is_hit() const { return kind == object::OBJECT_HIT; }

        TaikoDifficultyHitObject* previous_mono(int backwards) const;
        TaikoDifficultyHitObject* previous_note(int backwards) const;
        TaikoDifficultyHitObject* next_note(int forwards) const;
    };

    void create_hit_objects(std::vector<TaikoDifficultyHitObject>& out,
                            std::vector<DifficultyHitObject*>& object_ptrs,
                            std::vector<TaikoDifficultyHitObject*>& centre_objects,
                            std::vector<TaikoDifficultyHitObject*>& rim_objects,
                            std::vector<TaikoDifficultyHitObject*>& note_objects, const TaikoBeatmap& pb);
}}}} // namespace pppp::taiko::difficulty::preprocessing

#endif
