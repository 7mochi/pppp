#ifndef PPPP_MODS_MOD_SETTING_VALUE_H
#define PPPP_MODS_MOD_SETTING_VALUE_H

#include "pppp/config.h" // IWYU pragma: export

namespace pppp { namespace mods {
    bool clamp_and_round(double* target, double value, double min_value, double max_value, double precision);

    bool clamp_and_round(float* target, float value, float min_value, float max_value, float precision);

    float clamp_to_extended_limits(float value, float min_value, float max_value,
                                   nonstd::optional<float> extended_min_value,
                                   nonstd::optional<float> extended_max_value);
}} // namespace pppp::mods

#endif
