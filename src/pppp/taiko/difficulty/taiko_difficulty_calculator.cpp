// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/taiko/difficulty/taiko_difficulty_calculator.h"
#include "pppp/taiko/difficulty/preprocessing/colour/taiko_colour_difficulty_preprocessor.h"
#include "pppp/taiko/difficulty/preprocessing/rhythm/taiko_rhythm_difficulty_preprocessor.h"
#include "pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.h"
#include "pppp/taiko/difficulty/skills/colour.h"
#include "pppp/taiko/difficulty/skills/reading.h"
#include "pppp/taiko/difficulty/skills/rhythm.h"
#include "pppp/taiko/difficulty/skills/stamina.h"
#include "pppp/taiko/taiko_beatmap.h"
#include "pppp/utils/difficulty_calculation_utils.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace pppp { namespace taiko { namespace difficulty {
    namespace {
        /// Applies a final re-scaling of the star rating.
        /// @param star_rating The raw star rating value before re-scaling.
        double rescale(double star_rating) {
            if (star_rating < 0) {
                return star_rating;
            }

            return 10.43 * std::log(star_rating / 8.0 + 1.0);
        }

        struct TaikoDifficultyCalculator {
            double strain_length_bonus;
            double pattern_multiplier;
            bool is_relax;
            bool is_convert;

            TaikoDifficultyCalculator()
                : strain_length_bonus(0.0),
                  pattern_multiplier(0.0),
                  is_relax(false),
                  is_convert(false) {}

            void create_difficulty_attributes(TaikoDifficultyAttributes& out, const TaikoBeatmap& pb,
                                              skills::Rhythm& rhythm, skills::Reading& reading,
                                              skills::Colour& colour, skills::Stamina& stamina,
                                              skills::Stamina& single_colour_stamina) {
                double stamina_difficulty_value = stamina.difficulty_value();

                double rhythm_skill = rhythm.difficulty_value() * RHYTHM_SKILL_MULTIPLIER;
                double reading_skill = reading.difficulty_value() * READING_SKILL_MULTIPLIER;
                double colour_skill = colour.difficulty_value() * COLOUR_SKILL_MULTIPLIER;
                double stamina_skill = stamina_difficulty_value * STAMINA_SKILL_MULTIPLIER;
                double mono_stamina_skill =
                    single_colour_stamina.difficulty_value() * STAMINA_SKILL_MULTIPLIER;
                double mono_stamina_factor =
                    stamina_skill == 0 ? 1.0 : pppp::utils::pow(mono_stamina_skill / stamina_skill, 5);

                double stamina_difficult_strains =
                    stamina.count_top_weighted_strains(stamina_difficulty_value);

                // As we don't have pattern integration in osu!taiko, we apply the other two skills relative
                // to rhythm.
                pattern_multiplier = pppp::utils::pow(stamina_skill * colour_skill, 0.10);
                strain_length_bonus =
                    1.0 + 0.15 * pppp::utils::reverse_lerp(stamina_difficult_strains, 1000.0, 1555.0);

                double consistency_factor = 0.0;
                double combined_rating =
                    combined_difficulty_value(consistency_factor, rhythm, reading, colour, stamina);

                double star_rating = rescale(combined_rating * 1.4);

                // Calculate proportional contribution of each skill to the combinedRating.
                double skill_rating =
                    star_rating / (rhythm_skill + reading_skill + colour_skill + stamina_skill);

                out.star_rating = star_rating;
                out.rhythm_difficulty = rhythm_skill * skill_rating;
                out.reading_difficulty = reading_skill * skill_rating;
                out.colour_difficulty = colour_skill * skill_rating;
                out.stamina_difficulty = stamina_skill * skill_rating;
                // Mechanical difficulty is the sum of colour and stamina difficulties.
                out.mechanical_difficulty = out.colour_difficulty + out.stamina_difficulty;
                out.mono_stamina_factor = mono_stamina_factor;
                out.stamina_top_strains = stamina_difficult_strains;
                out.consistency_factor = consistency_factor;
                out.max_combo = max_combo(pb);
            }

            /// Returns the combined star rating of the beatmap, calculated using peak strains from
            /// all sections of the map. For each section, the peak strains of all separate skills
            /// are combined into a single peak strain for the section. The resulting partial rating
            /// of the beatmap is a weighted sum of the combined peaks, higher peaks weighted more.
            double combined_difficulty_value(double& consistency_factor, const skills::Rhythm& rhythm,
                                             const skills::Reading& reading, const skills::Colour& colour,
                                             const skills::Stamina& stamina) {
                std::vector<double> rhythm_peaks, reading_peaks, colour_peaks, stamina_peaks;
                rhythm.get_current_strain_peaks(rhythm_peaks);
                reading.get_current_strain_peaks(reading_peaks);
                colour.get_current_strain_peaks(colour_peaks);
                stamina.get_current_strain_peaks(stamina_peaks);

                std::vector<double> peaks;
                combine_peaks(peaks, rhythm_peaks, reading_peaks, colour_peaks, stamina_peaks);

                consistency_factor = 0.0;

                if (peaks.empty()) {
                    return 0.0;
                }

                double combined_rating = 0.0;
                std::sort(peaks.begin(), peaks.end(), std::greater<double>());

                double weight = 1.0;
                for (size_t i = 0; i < peaks.size(); i++) {
                    combined_rating += peaks[i] * weight;
                    weight *= 0.9;
                }

                std::vector<double> object_peaks;
                combine_peaks(object_peaks, rhythm.get_object_difficulties(),
                              reading.get_object_difficulties(), colour.get_object_difficulties(),
                              stamina.get_object_difficulties());

                if (object_peaks.empty()) {
                    return 0.0;
                }

                // The average of the top 5% of strain peaks from hit objects.
                std::sort(object_peaks.begin(), object_peaks.end(), std::greater<double>());

                size_t top_count = 1 + object_peaks.size() / 20;
                double top_sum = 0.0;
                for (size_t i = 0; i < top_count; i++) {
                    top_sum += object_peaks[i];
                }
                double top_average = top_sum / static_cast<double>(top_count);

                double total = 0.0;
                for (size_t i = 0; i < object_peaks.size(); i++) {
                    total += object_peaks[i];
                }

                // Calculates a consistency factor as the sum of difficulty from hit objects compared to if
                // every object were as hard as the hardest.
                // The top average strain is used instead of the very hardest to prevent exceptionally hard
                // objects lowering the factor.
                consistency_factor = total / (top_average * static_cast<double>(object_peaks.size()));

                return combined_rating;
            }

            /// Combines lists of peak strains from multiple skills into a list of single peak
            /// strains for each section.
            void combine_peaks(std::vector<double>& out, const std::vector<double>& rhythm_peaks,
                               const std::vector<double>& reading_peaks,
                               const std::vector<double>& colour_peaks,
                               const std::vector<double>& stamina_peaks) {
                out.clear();

                for (size_t i = 0; i < colour_peaks.size(); i++) {
                    double rhythm_peak = rhythm_peaks[i] * RHYTHM_SKILL_MULTIPLIER * pattern_multiplier;
                    double reading_peak = reading_peaks[i] * READING_SKILL_MULTIPLIER;
                    // There is no colour difficulty in relax.
                    double colour_peak = is_relax ? 0.0 : colour_peaks[i] * COLOUR_SKILL_MULTIPLIER;
                    double stamina_peak = stamina_peaks[i] * STAMINA_SKILL_MULTIPLIER * strain_length_bonus;
                    // Available finger count is increased by 150%, thus we adjust accordingly.
                    stamina_peak /= is_convert || is_relax ? 1.5 : 1.0;

                    double peak = pppp::utils::norm(2.0, pppp::utils::norm(1.5, colour_peak, stamina_peak),
                                                    rhythm_peak, reading_peak);

                    // Sections with 0 strain are excluded to avoid worst-case time complexity of the
                    // following sort (e.g. /b/2351871).
                    // These sections will not contribute to the difficulty.
                    if (peak > 0) {
                        out.push_back(peak);
                    }
                }
            }
        };
    } // namespace

    void
    preprocessing::create_hit_objects(std::vector<preprocessing::TaikoDifficultyHitObject>& out,
                                      std::vector<preprocessing::DifficultyHitObject*>& object_ptrs,
                                      std::vector<preprocessing::TaikoDifficultyHitObject*>& centre_objects,
                                      std::vector<preprocessing::TaikoDifficultyHitObject*>& rim_objects,
                                      std::vector<preprocessing::TaikoDifficultyHitObject*>& note_objects,
                                      const TaikoBeatmap& pb) {
        out.clear();
        object_ptrs.clear();
        centre_objects.clear();
        rim_objects.clear();
        note_objects.clear();

        if (pb.objects.size() < 3) {
            return;
        }

        double clock_rate = pb.clock_rate;
        out.reserve(pb.objects.size() - 2);
        object_ptrs.reserve(pb.objects.size() - 2);

        // Generate TaikoDifficultyHitObjects from the beatmap's hit objects.
        for (size_t i = 2; i < pb.objects.size(); i++) {
            const TaikoHitObject& ho = pb.objects[i];
            const TaikoHitObject& last_ho = pb.objects[i - 1];

            out.push_back(preprocessing::TaikoDifficultyHitObject(ho, last_ho, clock_rate, object_ptrs,
                                                                  centre_objects, rim_objects, note_objects,
                                                                  static_cast<int>(out.size()), pb));
            preprocessing::TaikoDifficultyHitObject* object = &out.back();
            object_ptrs.push_back(object);

            if (ho.kind == object::OBJECT_HIT) {
                std::vector<preprocessing::TaikoDifficultyHitObject*>& mono =
                    ho.type == object::HIT_TYPE_CENTRE ? centre_objects : rim_objects;
                mono.push_back(object);
                note_objects.push_back(object);
            }
        }
    }

    Status calculate_difficulty(TaikoDifficultyAttributes& out, const pppp::beatmaps::Beatmap& beatmap,
                                const pppp::mods::Mod* mods, size_t mod_count) {
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        TaikoBeatmap pb;
        TaikoDifficultyCalculator c;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        out = TaikoDifficultyAttributes();

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        std::vector<preprocessing::TaikoDifficultyHitObject> dhos;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        std::vector<preprocessing::TaikoDifficultyHitObject*> centre_objects;
        std::vector<preprocessing::TaikoDifficultyHitObject*> rim_objects;
        std::vector<preprocessing::TaikoDifficultyHitObject*> note_objects;
        preprocessing::create_hit_objects(dhos, dho_ptrs, centre_objects, rim_objects, note_objects, pb);

        std::vector<preprocessing::MonoStreak> mono_streaks;
        std::vector<preprocessing::AlternatingMonoPattern> alternating_mono_patterns;
        std::vector<preprocessing::RepeatingHitPatterns> repeating_hit_patterns;
        preprocessing::colour::process_and_assign(dhos, mono_streaks, alternating_mono_patterns,
                                                  repeating_hit_patterns);

        std::vector<preprocessing::SameRhythmHitObjectGrouping> rhythm_groupings;
        std::vector<preprocessing::SamePatternsGroupedHitObjects> pattern_groupings;
        preprocessing::rhythm::process_and_assign(note_objects, rhythm_groupings, pattern_groupings);

        c.is_relax = pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_RX);
        c.is_convert = pb.is_convert;

        skills::Rhythm rhythm(mods, mod_count);
        skills::Reading reading(mods, mod_count);
        skills::Colour colour(mods, mod_count);
        skills::Stamina stamina(mods, mod_count, false, pb.is_convert);
        skills::Stamina single_colour_stamina(mods, mod_count, true, pb.is_convert);

        for (size_t i = 0; i < dhos.size(); i++) {
            rhythm.process(dhos[i]);
            reading.process(dhos[i]);
            colour.process(dhos[i]);
            stamina.process(dhos[i]);
            single_colour_stamina.process(dhos[i]);
        }

        c.create_difficulty_attributes(out, pb, rhythm, reading, colour, stamina, single_colour_stamina);

        return StatusCode::OK;
    }

    Status calculate_timed_difficulty(std::vector<double>& times,
                                      std::vector<TaikoDifficultyAttributes>& attributes,
                                      const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                      size_t mod_count) {
        times.clear();
        attributes.clear();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        TaikoBeatmap pb;
        TaikoDifficultyCalculator c;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        if (pb.objects.empty()) {
            return StatusCode::OK;
        }

        c.is_relax = pppp::mods::mod_has(mods, mod_count, pppp::mods::MOD_RX);
        c.is_convert = pb.is_convert;

        skills::Rhythm rhythm(mods, mod_count);
        skills::Reading reading(mods, mod_count);
        skills::Colour colour(mods, mod_count);
        skills::Stamina stamina(mods, mod_count, false, pb.is_convert);
        skills::Stamina single_colour_stamina(mods, mod_count, true, pb.is_convert);

        TaikoBeatmap progressive = pb;
        progressive.objects.clear();

        std::vector<preprocessing::TaikoDifficultyHitObject> dhos;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        std::vector<preprocessing::TaikoDifficultyHitObject*> centre_objects;
        std::vector<preprocessing::TaikoDifficultyHitObject*> rim_objects;
        std::vector<preprocessing::TaikoDifficultyHitObject*> note_objects;
        preprocessing::create_hit_objects(dhos, dho_ptrs, centre_objects, rim_objects, note_objects, pb);

        std::vector<preprocessing::MonoStreak> mono_streaks;
        std::vector<preprocessing::AlternatingMonoPattern> alternating_mono_patterns;
        std::vector<preprocessing::RepeatingHitPatterns> repeating_hit_patterns;
        preprocessing::colour::process_and_assign(dhos, mono_streaks, alternating_mono_patterns,
                                                  repeating_hit_patterns);

        std::vector<preprocessing::SameRhythmHitObjectGrouping> rhythm_groupings;
        std::vector<preprocessing::SamePatternsGroupedHitObjects> pattern_groupings;
        preprocessing::rhythm::process_and_assign(note_objects, rhythm_groupings, pattern_groupings);

        size_t current = 0;

        for (size_t i = 0; i < pb.objects.size(); i++) {
            progressive.objects.push_back(pb.objects[i]);
            const double end_time = pb.objects[i].time + pb.objects[i].duration;

            while (current < dhos.size() &&
                   pb.objects[current + 2].time + pb.objects[current + 2].duration <= end_time) {
                rhythm.process(dhos[current]);
                reading.process(dhos[current]);
                colour.process(dhos[current]);
                stamina.process(dhos[current]);
                single_colour_stamina.process(dhos[current]);

                current++;
            }

            TaikoDifficultyAttributes step;
            c.create_difficulty_attributes(step, progressive, rhythm, reading, colour, stamina,
                                           single_colour_stamina);
            times.push_back(end_time);
            attributes.push_back(step);
        }

        return StatusCode::OK;
    }

    Status calculate_strains(TaikoStrains& out, const pppp::beatmaps::Beatmap& beatmap,
                             const pppp::mods::Mod* mods, size_t mod_count) {
        out = TaikoStrains();
        if (!mods && mod_count) {
            return StatusCode::INVALID_ARGUMENT;
        }

        TaikoBeatmap pb;
        Status status = build(pb, beatmap, mods, mod_count);
        if (!status.ok()) {
            return status;
        }

        const double section_length = 400;

        std::vector<preprocessing::TaikoDifficultyHitObject> dhos;
        std::vector<preprocessing::DifficultyHitObject*> dho_ptrs;
        std::vector<preprocessing::TaikoDifficultyHitObject*> centre_objects;
        std::vector<preprocessing::TaikoDifficultyHitObject*> rim_objects;
        std::vector<preprocessing::TaikoDifficultyHitObject*> note_objects;
        preprocessing::create_hit_objects(dhos, dho_ptrs, centre_objects, rim_objects, note_objects, pb);

        std::vector<preprocessing::MonoStreak> mono_streaks;
        std::vector<preprocessing::AlternatingMonoPattern> alternating_mono_patterns;
        std::vector<preprocessing::RepeatingHitPatterns> repeating_hit_patterns;
        preprocessing::colour::process_and_assign(dhos, mono_streaks, alternating_mono_patterns,
                                                  repeating_hit_patterns);

        std::vector<preprocessing::SameRhythmHitObjectGrouping> rhythm_groupings;
        std::vector<preprocessing::SamePatternsGroupedHitObjects> pattern_groupings;
        preprocessing::rhythm::process_and_assign(note_objects, rhythm_groupings, pattern_groupings);

        skills::Rhythm rhythm(mods, mod_count);
        skills::Reading reading(mods, mod_count);
        skills::Colour colour(mods, mod_count);
        skills::Stamina stamina(mods, mod_count, false, pb.is_convert);
        skills::Stamina single_colour_stamina(mods, mod_count, true, pb.is_convert);

        if (!pb.objects.empty()) {
            for (size_t i = 0; i < dhos.size(); i++) {
                rhythm.process(dhos[i]);
                reading.process(dhos[i]);
                colour.process(dhos[i]);
                stamina.process(dhos[i]);
                single_colour_stamina.process(dhos[i]);
            }
        }

        if (!dhos.empty()) {
            out.start_time =
                (std::ceil(dhos[0].start_time / section_length) * section_length - section_length) *
                pb.clock_rate;
        }
        out.section_length = section_length * pb.clock_rate;

        colour.get_current_strain_peaks(out.colour);
        reading.get_current_strain_peaks(out.reading);
        rhythm.get_current_strain_peaks(out.rhythm);
        stamina.get_current_strain_peaks(out.stamina);
        single_colour_stamina.get_current_strain_peaks(out.single_colour_stamina);

        return StatusCode::OK;
    }
}}} // namespace pppp::taiko::difficulty
