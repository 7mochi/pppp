// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/osu/osu_beatmap_converter.h"
#include "pppp/beatmaps/legacy_hit_object_type.h"
#include "pppp/osu/mods/osu_mod_hidden.h"
#include "pppp/osu/mods/osu_mod_random.h"
#include "pppp/osu/mods/osu_mod_target_practice.h"
#include "pppp/osu/object/osu_hit_object.h"
#include "pppp/osu/object/osu_playfield.h"
#include "pppp/utils/linq/csharp.h"
#include <algorithm>

namespace pppp { namespace osu {
    namespace linq = ::pppp::utils::linq;

    Status convert(OsuBeatmap& pb, const beatmaps::Beatmap& beatmap,
                   const beatmaps::ObjectConverted<object::OsuHitObject>& object_converted) {
        pb = OsuBeatmap();
        pb.source = &beatmap;
        pb.format_version = beatmap.format_version;
        pb.stack_leniency = beatmap.stack_leniency;
        pb.circle_size = beatmap.difficulty.circle_size;
        pb.approach_rate = beatmap.difficulty.approach_rate;
        pb.overall_difficulty = beatmap.difficulty.overall_difficulty;
        pb.drain_rate = beatmap.difficulty.drain_rate;

        pb.objects.resize(beatmap.hit_objects.size());
        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            const beatmaps::HitObject& ho = beatmap.hit_objects[i];
            object::OsuHitObject& o = pb.objects[i];
            o.type = (ho.type & beatmaps::hold) != 0 && (ho.type & beatmaps::slider) == 0
                         ? (ho.type & ~static_cast<unsigned>(beatmaps::hold)) | beatmaps::spinner
                         : ho.type;
            o.time = ho.start_time;
            o.end_time = ho.end_time;
            o.duration = ho.duration;
            utils::Vector2 raw = ho.position;
            if (ho.type & 8) {
                raw.x = 256.0f;
                raw.y = 192.0f;
            }
            o.position = raw;
            o.new_combo = ho.new_combo;
            o.combo_offset = ho.combo_offset;
            o.combo_index = 0;
            o.index_in_current_combo = 0;
            o.last_in_combo = false;
            o.kiai = false;
            o.stack_height = 0;
            o.time_preempt = 0.0;
            o.time_fade_in = 0.0;
            o.slider = (ho.type & 2) ? ho.slider : -1;

            if (object_converted.invoke != 0) {
                object_converted.invoke(i, &pb.objects[i], 1, object_converted.context);
            }
        }

        pb.sliders.resize(beatmap.sliders.size());
        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            const beatmaps::HitObject& ho = beatmap.hit_objects[i];
            if (!(ho.type & 2) || ho.slider < 0 || static_cast<size_t>(ho.slider) >= beatmap.sliders.size()) {
                continue;
            }
            const beatmaps::Slider& src = beatmap.sliders[ho.slider];
            object::PlayableSlider& sl = pb.sliders[ho.slider];
            sl.owner_object = static_cast<unsigned>(i);
            sl.slides = src.slides;
            sl.length = src.expected_length;

            utils::Vector2 head = pb.objects[i].position;
            sl.control_points.resize(src.control_points.size());
            for (size_t k = 0; k < src.control_points.size(); k++) {
                const utils::Vector2& p = src.control_points[k];
                sl.control_points[k].x = p.x - head.x;
                sl.control_points[k].y = p.y - head.y;
            }
            sl.path = src.path;
            sl.cumulative_lengths = src.cumulative_lengths;
            sl.events.resize(src.events.size());
            for (size_t k = 0; k < src.events.size(); k++) {
                const beatmaps::SliderEventDescriptor& e = src.events[k];
                sl.events[k].type = e.type;
                sl.events[k].time = e.time;
                sl.events[k].span_index = e.span_index;
                sl.events[k].span_start_time = e.span_start_time;
                sl.events[k].path_progress = e.path_progress;
                sl.events[k].position = e.position;
            }
        }

        beatmaps::control_points::ControlPointInfo original;
        original.build(beatmap);
        pb.info = original.deep_clone();

        pb.breaks.resize(beatmap.breaks.size());
        for (size_t i = 0; i < beatmap.breaks.size(); i++) {
            pb.breaks[i].start_time = beatmap.breaks[i].start_time;
            pb.breaks[i].end_time = std::max(beatmap.breaks[i].start_time, beatmap.breaks[i].end_time);
        }
        return StatusCode::OK;
    }

    void apply_defaults(OsuBeatmap& pb) {
        double preempt = object::OsuHitObject::time_preempt_for_ar(pb.approach_rate);
        for (size_t i = 0; i < pb.objects.size(); i++) {
            object::OsuHitObject& o = pb.objects[i];
            o.time_preempt = preempt;
            o.time_fade_in = object::OsuHitObject::time_fade_in_for_preempt(preempt);
            o.kiai = kiai_at(pb, o.time + 1.0);
        }
    }

    Status reflect(OsuBeatmap& pb, int axes) {
        if (axes == pppp::mods::MOD_REFLECTION_NONE) {
            return StatusCode::OK;
        }
        bool h = (axes & pppp::mods::MOD_REFLECTION_HORIZONTAL) != 0;
        bool v = (axes & pppp::mods::MOD_REFLECTION_VERTICAL) != 0;
        if (!pb.sliders.empty() && !can_recompute_sliders(pb)) {
            return StatusCode::NO_SLIDER_PATH_BACKEND;
        }
        for (size_t i = 0; i < pb.objects.size(); i++) {
            object::OsuHitObject& o = pb.objects[i];
            if (h) {
                o.position.x = object::PLAYFIELD_WIDTH - o.position.x;
            }
            if (v) {
                o.position.y = object::PLAYFIELD_HEIGHT - o.position.y;
            }
        }
        for (size_t s = 0; s < pb.sliders.size(); s++) {
            object::PlayableSlider& sl = pb.sliders[s];
            if (sl.control_points.empty() && sl.path.empty()) {
                continue;
            }
            for (size_t k = 0; k < sl.control_points.size(); k++) {
                utils::Vector2 p = sl.control_points[k];
                if (h) {
                    utils::Vector2 r = {-p.x, p.y};
                    set_control_point(sl.control_points[k], r);
                    p = sl.control_points[k];
                }
                if (v) {
                    utils::Vector2 r = {p.x, -p.y};
                    set_control_point(sl.control_points[k], r);
                }
            }

            Status rc = slider_recompute(pb, s);
            if (!rc.ok()) {
                return rc;
            }
        }
        return StatusCode::OK;
    }

    Status apply_beatmap_mods(OsuBeatmap& pb, pppp::mods::Mod* mods, size_t mod_count) {
        for (size_t m = 0; m < mod_count; m++) {
            switch (mods[m].id) {
            case pppp::mods::MOD_HD:
                for (size_t i = 0; i < pb.objects.size(); i++) {
                    // Sliders retain their default TimeFadeIn to match Stable
                    if (!(pb.objects[i].type & 2)) {
                        pb.objects[i].time_fade_in =
                            pb.objects[i].time_preempt * mods::FADE_IN_DURATION_MULTIPLIER;
                    }
                }
                break;
            case pppp::mods::MOD_FR: {
                double last_new_combo_time = 0.0;
                for (size_t i = 0; i < pb.objects.size(); i++) {
                    object::OsuHitObject& o = pb.objects[i];
                    if (o.new_combo) {
                        last_new_combo_time = o.time;
                    }
                    if (!(o.type & 8)) {
                        o.time_preempt += o.time - last_new_combo_time;
                    }
                }
                break;
            }
            case pppp::mods::MOD_RD: {
                Status rc = mods::apply_random(pb, mods[m]);
                if (!rc.ok()) {
                    return rc;
                }
            } break;
            case pppp::mods::MOD_TP: {
                Status rc = mods::apply_target_practice(pb, mods[m]);
                if (!rc.ok()) {
                    return rc;
                }
            } break;
            case pppp::mods::MOD_WU:
            case pppp::mods::MOD_WD:
                pppp::mods::mod_time_ramp_apply_to_beatmap(
                    &mods[m], !pb.objects.empty(), pb.objects.empty() ? 0.0 : pb.objects[0].time,
                    pb.objects.empty()
                        ? 0.0
                        : linq::from(pb.objects).max(linq::bind(&object::OsuHitObject::end_time, linq::_1)));
                break;
            default: break;
            }
        }
        return StatusCode::OK;
    }

    bool can_recompute_sliders(const OsuBeatmap& pb) {
        return pb.source != 0 && pb.source->slider_path.recompute != 0;
    }

    Status slider_recompute(OsuBeatmap& pb, size_t slider_index) {
        if (slider_index >= pb.sliders.size()) {
            return StatusCode::INVALID_ARGUMENT;
        }
        if (!can_recompute_sliders(pb)) {
            return StatusCode::NO_SLIDER_PATH_BACKEND;
        }
        object::PlayableSlider& sl = pb.sliders[slider_index];
        const beatmaps::SliderPathOps& backend = pb.source->slider_path;
        int rc = backend.recompute(backend.ctx, sl.owner_object,
                                   sl.control_points.empty() ? 0 : &sl.control_points[0],
                                   sl.control_points.size(), &sl.path, &sl.cumulative_lengths);
        if (rc != 0) {
            return StatusCode::SLIDER_PATH;
        }

        for (size_t e = 0; e < sl.events.size(); e++) {
            sl.events[e].position = slider_position_at(sl, sl.events[e].path_progress);
        }
        return StatusCode::OK;
    }

}} // namespace pppp::osu
