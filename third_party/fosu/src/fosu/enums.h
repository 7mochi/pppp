#ifndef FOSU_ENUMS_H
#define FOSU_ENUMS_H

namespace fosu {

    // Scoped enumerations (enum class) become a struct with a nested enum: CurveType::Bezier is
    // spelled the same way, the values are the same, and there is no implicit int conversion in the
    // other direction.
    struct CurveType {
        enum Value { Bezier = 'B', Catmull = 'C', Linear = 'L', PerfectCurve = 'P' };
    };

    // None is the legacy zero value: a timing point uses the beatmap default;
    // at the beatmap level it denotes the default normal sample bank.
    struct SampleSet {
        enum Value { None = 0, Normal = 1, Soft = 2, Drum = 3 };
    };

} // namespace fosu

#endif
