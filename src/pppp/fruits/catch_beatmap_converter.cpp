// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/catch_beatmap_converter.h"
#include "pppp/fruits/object/banana_shower.h"
#include "pppp/fruits/object/juice_stream.h"
#include "pppp/utils/math/csharp.h"

namespace pppp { namespace fruits {
    namespace {
        /// Flattens a juice stream's / banana shower's nested objects into `out`, in order.
        void flatten(std::vector<object::CatchHitObject*>& out,
                     std::vector<object::CatchHitObject>& objects) {
            for (size_t i = 0; i < objects.size(); i++) {
                out.push_back(&objects[i]);
                flatten(out, objects[i].nested);
            }
        }
    } // namespace

    Result::Value convert(CatchBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap,
                          const pppp::beatmaps::ObjectConverted<object::CatchHitObject>& object_converted) {
        for (size_t i = 0; i < beatmap.hit_objects.size(); i++) {
            const pppp::beatmaps::HitObject& ho = beatmap.hit_objects[i];
            const size_t first = pb.objects.size();

            if (ho.slider >= 0 && static_cast<size_t>(ho.slider) < beatmap.sliders.size()) {
                const pppp::beatmaps::Slider& slider = beatmap.sliders[ho.slider];
                // prior to v8, speed multipliers don't adjust for how many ticks are generated over the same
                // distance. this results in more (or less) ticks being generated in <v8 maps for the same
                // time duration.
                pb.objects.push_back(object::create_juice_stream(
                    slider, utils::math::clamp(ho.position.x, 0.0, object::PLAYFIELD_WIDTH), ho.start_time,
                    static_cast<int>(ho.slider)));
            } else if ((ho.type & 8) != 0) {
                pb.objects.push_back(object::create_banana_shower(ho.start_time, ho.end_time));
            } else {
                object::CatchHitObject fruit;
                fruit.kind = object::OBJECT_FRUIT;
                fruit.time = ho.start_time;
                fruit.original_x = ho.position.x;
                fruit.x_offset = 0.0;
                fruit.distance_to_hyper_dash = 0.0;
                fruit.hyper_dash = false;
                fruit.slider_index = -1;
                pb.objects.push_back(fruit);
            }

            if (object_converted.invoke != 0) {
                object_converted.invoke(i, pb.objects.size() > first ? &pb.objects[first] : 0,
                                        pb.objects.size() - first, object_converted.context);
            }
        }

        flatten(pb.all_objects, pb.objects);
        return Result::OK;
    }
}} // namespace pppp::fruits
