// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/mods/catch_mod_mirror.h"

namespace pppp { namespace fruits { namespace mods {
    namespace {
        /// Mirrors the effective X position of an object and its nested hit objects.
        void mirror_effective_x(object::CatchHitObject& object) {
            object.original_x = object::PLAYFIELD_WIDTH - object.original_x;
            object.x_offset = -object.x_offset;

            for (size_t i = 0; i < object.nested.size(); i++) {
                object.nested[i].original_x = object::PLAYFIELD_WIDTH - object.nested[i].original_x;
                object.nested[i].x_offset = -object.nested[i].x_offset;
            }
        }

        /// Mirrors X positions of all bananas in a banana shower.
        void mirror_banana_shower(object::CatchHitObject& object) {
            for (size_t i = 0; i < object.nested.size(); i++) {
                object.nested[i].x_offset = object::PLAYFIELD_WIDTH - object.nested[i].x_offset;
            }
        }

        void apply_to_hit_object(object::CatchHitObject& object) {
            switch (object.kind) {
            case object::OBJECT_FRUIT:
            case object::OBJECT_JUICE_STREAM: mirror_effective_x(object); break;

            case object::OBJECT_BANANA_SHOWER: mirror_banana_shower(object); break;

            default: break;
            }
        }
    } // namespace

    void apply_mirror(CatchBeatmap& pb) {
        for (size_t i = 0; i < pb.objects.size(); i++) {
            apply_to_hit_object(pb.objects[i]);
        }
    }
}}} // namespace pppp::fruits::mods
