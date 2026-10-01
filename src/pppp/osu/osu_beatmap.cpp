// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/osu_beatmap.h"
#include "pppp/beatmaps/slider_path.h"
#include "pppp/mods/mod.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/osu/osu_beatmap_converter.h"
#include "pppp/osu/osu_beatmap_processor.h"

namespace pppp { namespace osu {
    OsuBeatmap::OsuBeatmap()
        : source(0),
          format_version(14),
          stack_leniency(0.7),
          circle_size(5.0),
          approach_rate(5.0),
          overall_difficulty(5.0),
          drain_rate(5.0),
          clock_rate(1.0) {}

    const pppp::beatmaps::control_points::TimingControlPoint* timing_point_at(const OsuBeatmap& pb,
                                                                              double time) {
        return pb.info.timing_point_at(time);
    }

    bool kiai_at(const OsuBeatmap& pb, double time) {
        const pppp::beatmaps::control_points::EffectControlPoint* ep = pb.info.effect_point_at(time);
        return ep ? ep->kiai_mode : false;
    }

    void set_control_point(pppp::utils::Vector2& current, pppp::utils::Vector2 value) {
        if (value.x == current.x && value.y == current.y) {
            return;
        }
        current = value;
    }

    pppp::utils::Vector2 slider_position_at(const object::PlayableSlider& slider, double progress) {
        return pppp::beatmaps::slider_position_at(slider.path, slider.cumulative_lengths, progress);
    }

    pppp::utils::Vector2 object_end_position(const OsuBeatmap& pb, size_t object_index) {
        const object::OsuHitObject& ho = pb.objects[object_index];
        if (ho.slider < 0) {
            return ho.position;
        }

        const object::PlayableSlider& sl = pb.sliders[ho.slider];
        for (size_t e = 0; e < sl.events.size(); e++) {
            if (sl.events[e].type == pppp::beatmaps::SLIDER_EVENT_TAIL) {
                return ho.position + sl.events[e].position;
            }
        }
        return ho.position + slider_position_at(sl, sl.slides % 2);
    }

    double object_radius(const OsuBeatmap& pb) {
        return object::OsuHitObject::calculate_radius(pb.circle_size);
    }

    double last_object_time(const OsuBeatmap& pb) {
        double last = 0.0;
        for (size_t i = 0; i < pb.objects.size(); i++) {
            if (i == 0 || pb.objects[i].end_time > last) {
                last = pb.objects[i].end_time;
            }
        }
        return last;
    }

    int max_combo(const OsuBeatmap& pb) {
        int combo = 0;
        for (size_t i = 0; i < pb.objects.size(); i++) {
            const object::OsuHitObject& ho = pb.objects[i];
            if (ho.slider >= 0) {
                const std::vector<pppp::beatmaps::SliderEventDescriptor>& events =
                    pb.sliders[ho.slider].events;
                for (size_t j = 0; j < events.size(); j++) {
                    if (events[j].type != pppp::beatmaps::SLIDER_EVENT_LEGACY_LAST_TICK) {
                        combo++;
                    }
                }
            } else {
                combo++;
            }
        }
        return combo;
    }

    Result::Value build(OsuBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                        size_t mod_count,
                        const pppp::beatmaps::ObjectConverted<object::OsuHitObject>& object_converted) {
        Result::Value rc = convert(pb, beatmap, object_converted);
        if (rc != Result::OK) {
            return rc;
        }

        pppp::mods::DifficultySettings ds;
        ds.drain_rate = pb.drain_rate;
        ds.circle_size = pb.circle_size;
        ds.overall_difficulty = pb.overall_difficulty;
        ds.approach_rate = pb.approach_rate;
        ds.slider_multiplier = beatmap.difficulty.slider_multiplier;
        ds.slider_tick_rate = beatmap.difficulty.slider_tick_rate;
        pppp::mods::mod_apply_to_difficulty(mods, mod_count, &ds);
        pb.circle_size = static_cast<float>(ds.circle_size);
        pb.approach_rate = static_cast<float>(ds.approach_rate);
        pb.overall_difficulty = static_cast<float>(ds.overall_difficulty);
        pb.drain_rate = static_cast<float>(ds.drain_rate);
        pb.clock_rate = pppp::mods::mod_calculate_rate(mods, mod_count);

        update_combo_information(pb);
        apply_defaults(pb);
        rc = reflect(pb, pppp::mods::mod_reflection(mods, mod_count));
        if (rc != Result::OK) {
            return rc;
        }
        apply_stacking(pb);
        return apply_beatmap_mods(pb, mods, mod_count);
    }
}} // namespace pppp::osu
