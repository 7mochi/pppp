// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/difficulty/skills/aim.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/difficulty/evaluators/aim/agility_evaluator.h"
#include "pppp/osu/difficulty/evaluators/aim/flow_aim_evaluator.h"
#include "pppp/osu/difficulty/evaluators/aim/snap_aim_evaluator.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cmath>

namespace pppp { namespace osu { namespace difficulty { namespace skills {
    Aim::Aim(const pppp::mods::Mod* mods_in, size_t mod_count_in, bool include_sliders_in)
        : VariableLengthStrainSkill(mods_in, mod_count_in, 0.9, 400),
          include_sliders(include_sliders_in),
          current_strain(0.0) {}

    double Aim::strain_decay(double ms) const { return utils::pow(0.2, ms / 1000.0); }

    double Aim::calculate_initial_strain(double time, const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        double prev_start = current.last_object_start_time / current.clock_rate;
        return current_strain * strain_decay(time - prev_start);
    }

    double Aim::strain_value_at(const DifficultyHitObject& base_current) {
        const preprocessing::OsuDifficultyHitObject& current =
            static_cast<const preprocessing::OsuDifficultyHitObject&>(base_current);
        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_AP)) {
            return 0;
        }

        double decay = strain_decay(current.adjusted_delta_time);

        current_strain *= decay;
        current_strain += calculate_adjusted_difficulty(current) * (1.0 - decay);

        if (current.base_is_slider) {
            slider_strains.push_back(current_strain);
        }

        return current_strain;
    }

    double Aim::calculate_adjusted_difficulty(const preprocessing::OsuDifficultyHitObject& current) {
        const double skill_multiplier_snap = 70.9;
        const double skill_multiplier_agility = 2.35;
        const double skill_multiplier_flow = 242.0;

        double snap_difficulty = 0.0;
        double agility_difficulty = 0.0;
        double flow_difficulty = 0.0;

        snap_difficulty =
            evaluators::aim::SnapAimEvaluator::evaluate_difficulty_of(current, include_sliders) *
            skill_multiplier_snap;
        agility_difficulty =
            evaluators::aim::AgilityEvaluator::evaluate_difficulty_of(current) * skill_multiplier_agility;
        flow_difficulty =
            evaluators::aim::FlowAimEvaluator::evaluate_difficulty_of(current, include_sliders) *
            skill_multiplier_flow;

        double total_difficulty = calculate_total_value(snap_difficulty, agility_difficulty, flow_difficulty);

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_MG)) {
            for (size_t i = 0; i < mod_count; i++) {
                if (mods[i].id == pppp::mods::MOD_MG) {
                    total_difficulty *= 1.0 - mods[i].magnetised.strength;
                    break;
                }
            }
        }

        double od = std::max(0.0, current.overall_difficulty);
        total_difficulty *= 0.985 + utils::pow(od, 2) / 4000.0;

        return total_difficulty;
    }

    double Aim::calculate_total_value(double snap_difficulty, double agility_difficulty,
                                      double flow_difficulty) {
        const double skill_multiplier_total = 1.12;
        const double combined_snap_norm_exponent = 1.2;

        // We compare flow to combined snap and agility because snap by itself doesn't have enough difficulty
        // to be above flow on streams Agility on the other hand is supposed to measure the rate of cursor
        // velocity changes while snapping So snapping every circle on a stream requires an enormous amount of
        // agility at which point it's easier to flow
        double combined_snap_difficulty =
            utils::norm(combined_snap_norm_exponent, snap_difficulty, agility_difficulty);

        double p_snap = calculate_snap_flow_probability(flow_difficulty / combined_snap_difficulty);
        double p_flow = 1.0 - p_snap;

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_TD)) {
            // we don't adjust agility here since agility represents TD difficulty in a decent enough way
            snap_difficulty = utils::pow(snap_difficulty, 0.89);
            combined_snap_difficulty =
                utils::norm(combined_snap_norm_exponent, snap_difficulty, agility_difficulty);
        }

        if (pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_RX)) {
            combined_snap_difficulty *= 0.75;
            flow_difficulty *= 0.6;
        }

        double total_difficulty = combined_snap_difficulty * p_snap + flow_difficulty * p_flow;

        double total_strain = total_difficulty * skill_multiplier_total;

        return total_strain;
    }

    double Aim::calculate_snap_flow_probability(double ratio) {
        const double k = 7.27;

        if (ratio == 0) {
            return 0;
        }

        if (ratio != ratio) { // NaN check
            return 1;
        }

        return utils::logistic(-k * std::log(ratio));
    }

    double Aim::get_difficult_sliders() {
        if (slider_strains.empty()) {
            return 0;
        }

        double max_slider_strain = *std::max_element(slider_strains.begin(), slider_strains.end());

        if (max_slider_strain == 0) {
            return 0;
        }

        double sum = 0.0;
        for (size_t i = 0; i < slider_strains.size(); i++) {
            sum += utils::logistic(slider_strains[i] / max_slider_strain, 0.5, 12.0);
        }
        return sum;
    }

    double Aim::count_top_weighted_sliders(double difficulty_value) {
        if (slider_strains.empty()) {
            return 0;
        }

        double consistent_top_strain =
            difficulty_value *
            (1.0 - decay_weight); // What would the top strain be if all strain values were identical

        if (consistent_top_strain == 0) {
            return 0;
        }

        // Use a weighted sum of all strains. Constants are arbitrary and give nice values
        double sum = 0.0;
        for (size_t i = 0; i < slider_strains.size(); i++) {
            sum += utils::logistic(slider_strains[i] / consistent_top_strain, 0.88, 10.0, 1.1);
        }
        return sum;
    }

    std::vector<StrainPeak> Aim::get_reduced_strain_peaks() {
        const int reduced_section_time = 4000;
        const double reduced_strain_baseline = 0.727;

        std::vector<StrainPeak> strains;
        const std::vector<StrainPeak>& raw_peaks = get_current_strain_peaks();
        for (size_t i = 0; i < raw_peaks.size(); i++) {
            // Sections with 0 strain are excluded to avoid worst-case time complexity of the following sort
            // (e.g. /b/2351871). These sections will not contribute to the difficulty.
            if (raw_peaks[i].value > 0) {
                strains.push_back(raw_peaks[i]);
            }
        }

        const int chunk_size = 20;
        double red_time = 0.0;
        int skip_count = 0;

        // We are reducing the highest strains first to account for extreme difficulty spikes
        // Strains are split into 20ms chunks to try to mitigate inconsistencies caused by reducing strains
        while (static_cast<int>(strains.size()) > skip_count && red_time < reduced_section_time) {
            StrainPeak strain = strains[skip_count];

            // NOLINTNEXTLINE(cert-flp30-c)
            for (double added_time = 0.0; added_time < strain.section_length; added_time += chunk_size) {
                double t = (red_time + added_time) / reduced_section_time;
                t = utils::math::clamp(t, 0.0, 1.0);
                double scale = std::log10(utils::math::lerp(1.0, 10.0, t));

                StrainPeak new_peak;
                // intentionally add at end and sort afterwards, should be cheaper.
                new_peak.value = strain.value * utils::math::lerp(reduced_strain_baseline, 1.0, scale);
                new_peak.section_length =
                    std::min(static_cast<double>(chunk_size), strain.section_length - added_time);
                strains.push_back(new_peak);
            }

            red_time += strain.section_length;
            skip_count++;
        }

        std::vector<StrainPeak> reduced(strains.begin() + skip_count, strains.end());
        // Stable, so equal peaks keep their order.
        std::stable_sort(reduced.begin(), reduced.end(), pppp::common::skills::strain_peak_greater);
        return reduced;
    }

    double Aim::difficulty_value() {
        double difficulty = 0.0;
        double time = 0.0;

        const std::vector<StrainPeak>& reduced = get_reduced_strain_peaks();

        // Difficulty is a continuous weighted sum of the sorted strains
        for (size_t i = 0; i < reduced.size(); i++) {
            // Weighting function can be thought of as:
            //      b
            //      ∫ DecayWeight^x dx
            //      a
            //  where a = startTime and b = endTime
            //
            //  Technically, the function below has been slightly modified from the equation above.
            //  The real function would be
            //      double weight = pow(decay_weight, start_time) - pow(decay_weight, end_time);
            //      ...
            //      return difficulty / log(1 / decay_weight);
            //  E.g. for a decay weight of 0.9, we're multiplying by 10 instead of 9.49122...
            //
            // This change makes it so that a map composed solely of MaxSectionLength chunks will have the
            // exact same value when summed in this class and StrainSkill. Doing this ensures the relationship
            // between strain values and difficulty values remains the same between the two classes.
            double start_time = time;
            double end_time = time + reduced[i].section_length / max_section_length;

            double weight = utils::pow(decay_weight, start_time) - utils::pow(decay_weight, end_time);

            difficulty += reduced[i].value * weight;
            time = end_time;
        }

        return difficulty / (1.0 - decay_weight);
    }
}}}} // namespace pppp::osu::difficulty::skills
