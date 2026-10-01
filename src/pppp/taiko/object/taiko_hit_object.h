// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_TAIKO_OBJECT_TAIKO_HIT_OBJECT_H
#define PPPP_TAIKO_OBJECT_TAIKO_HIT_OBJECT_H

namespace pppp { namespace taiko { namespace object {
    /// The type of a Hit.
    enum HitType {
        /// A Hit that can be hit by the centre portion of the drum.
        HIT_TYPE_CENTRE = 0,
        /// A Hit that can be hit by the rim portion of the drum.
        HIT_TYPE_RIM = 1
    };

    enum ObjectKind { OBJECT_HIT = 0, OBJECT_DRUM_ROLL = 1, OBJECT_SWELL = 2 };

    const unsigned HIT_SOUND_WHISTLE = 2;
    const unsigned HIT_SOUND_FINISH = 4;
    const unsigned HIT_SOUND_CLAP = 8;

    struct TaikoHitObject {
        ObjectKind kind;
        HitType type;
        double time;
        double duration;

        /// Whether this HitObject is a "strong" type.
        /// Strong hit objects give more points for hitting the hit object with both keys.
        bool is_strong;

        static HitType hit_type_of(unsigned hitsound) {
            return (hitsound & (HIT_SOUND_WHISTLE | HIT_SOUND_CLAP)) != 0 ? HIT_TYPE_RIM : HIT_TYPE_CENTRE;
        }
    };
}}} // namespace pppp::taiko::object

namespace pppp { namespace taiko {
    typedef object::TaikoHitObject TaikoHitObject;
}} // namespace pppp::taiko

#endif
