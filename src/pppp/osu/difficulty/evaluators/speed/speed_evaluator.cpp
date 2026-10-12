// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/evaluators/speed/speed_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"

namespace pppp { namespace osu { namespace difficulty { namespace evaluators { namespace speed {
    namespace {
        double calculate_speed_bonus(const preprocessing::OsuDifficultyHitObject& osu_curr_obj) {
            const double min_speed_bonus = 200; // 200 BPM 1/4th
            const double speed_balancing_factor = 40;

            double strain_time = osu_curr_obj.adjusted_delta_time;

            // Cap deltatime to the OD 300 hitwindow.
            // 0.93 is derived from making sure 260bpm OD8 streams aren't nerfed harshly, whilst 0.92 limits
            // the effect of the cap.
            strain_time /=
                utils::math::clamp((strain_time / osu_curr_obj.hit_window_great) / 0.93, 0.92, 1.0);

            // speedBonus will be 0.0 for BPM < 200
            double speed_bonus = 0.0;

            // Add additional scaling bonus for streams/bursts higher than 200bpm
            if (utils::milliseconds_to_bpm(strain_time) > min_speed_bonus) {
                speed_bonus = 0.75 * utils::pow((utils::bpm_to_milliseconds(min_speed_bonus) - strain_time) /
                                                    speed_balancing_factor,
                                                2);
            }

            return (1.0 + speed_bonus) / strain_time;
        }
    } // namespace

    double SpeedEvaluator::evaluate_difficulty_of(const preprocessing::OsuDifficultyHitObject& current) {
        if (current.base_is_spinner && current.spins_required <= 0) {
            return 0.0;
        }

        const preprocessing::OsuDifficultyHitObject* next =
            static_cast<const preprocessing::OsuDifficultyHitObject*>(current.next(0));
        double double_tap_feasibility = 1.0 - (next ? calculate_double_tap_feasibility(current, *next) : 0.0);

        double speed_difficulty = utils::milliseconds_to_bpm(current.adjusted_delta_time);

        speed_difficulty *= calculate_speed_bonus(current);

        // Apply penalty if there's doubletappable doubles
        return speed_difficulty * double_tap_feasibility;
    }
}}}}} // namespace pppp::osu::difficulty::evaluators::speed
