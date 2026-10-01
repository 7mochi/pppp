// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/catch_beatmap.h"
#include "pppp/fruits/catch_beatmap_converter.h"
#include "pppp/fruits/catch_beatmap_processor.h"
#include "pppp/fruits/mods/catch_mod_mirror.h"
#include "pppp/utils/precision.h"
#include <algorithm>

namespace pppp { namespace fruits {
    CatchBeatmap::CatchBeatmap()
        : clock_rate(1.0),
          circle_size(5.0),
          approach_rate(5.0),
          is_convert(false) {}

    int max_combo(const CatchBeatmap& pb) {
        int combo = 0;
        for (size_t i = 0; i < pb.all_objects.size(); i++) {
            if (pb.all_objects[i]->kind == object::OBJECT_FRUIT ||
                pb.all_objects[i]->kind == object::OBJECT_DROPLET) {
                combo++;
            }
        }
        return combo;
    }

    void palpable_objects(std::vector<object::CatchHitObject*>& out, CatchBeatmap& pb) {
        out.clear();

        // We want to only consider fruits that contribute to the combo.
        for (size_t i = 0; i < pb.all_objects.size(); i++) {
            if (pb.all_objects[i]->kind == object::OBJECT_FRUIT ||
                pb.all_objects[i]->kind == object::OBJECT_DROPLET) {
                out.push_back(pb.all_objects[i]);
            }
        }

        std::stable_sort(out.begin(), out.end(), object::hit_object_earlier);
    }

    ModdedDifficulty modded_difficulty(const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                       size_t mod_count) {
        ModdedDifficulty out;
        out.circle_size = beatmap.difficulty.circle_size;
        out.approach_rate = beatmap.difficulty.approach_rate;
        out.overall_difficulty = beatmap.difficulty.overall_difficulty;
        out.drain_rate = beatmap.difficulty.drain_rate;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_EZ:
                out.circle_size = utils::f32(utils::f32(out.circle_size) * 0.5);
                out.approach_rate = utils::f32(utils::f32(out.approach_rate) * 0.5);
                out.drain_rate = utils::f32(utils::f32(out.drain_rate) * 0.5);
                out.overall_difficulty = utils::f32(utils::f32(out.overall_difficulty) * 0.5);
                break;
            case pppp::mods::MOD_HR:
                out.circle_size = std::min(utils::f32(utils::f32(out.circle_size) * 1.2999999523162842),
                                           10.0); // CS uses a custom 1.3 ratio.
                out.approach_rate =
                    std::min(utils::f32(utils::f32(out.approach_rate) * 1.399999976158142), 10.0);
                out.drain_rate = std::min(utils::f32(utils::f32(out.drain_rate) * 1.399999976158142), 10.0);
                out.overall_difficulty =
                    std::min(utils::f32(utils::f32(out.overall_difficulty) * 1.399999976158142), 10.0);
                break;
            case pppp::mods::MOD_DA: {
                const pppp::mods::DifficultyAdjustSettings& st = mods[i].difficulty_adjust;
                if (st.circle_size.value() >= 0.0) {
                    out.circle_size = st.circle_size.value();
                }
                if (st.approach_rate.value() >= 0.0) {
                    out.approach_rate = st.approach_rate.value();
                }
                if (st.drain_rate.value() >= 0.0) {
                    out.drain_rate = st.drain_rate.value();
                }
                if (st.overall_difficulty.value() >= 0.0) {
                    out.overall_difficulty = st.overall_difficulty.value();
                }
                break;
            }
            default: break;
            }
        }

        return out;
    }

    Result::Value build(CatchBeatmap& pb, const pppp::beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                        size_t mod_count,
                        const pppp::beatmaps::ObjectConverted<object::CatchHitObject>& object_converted) {
        pb = CatchBeatmap();
        pb.clock_rate = pppp::mods::mod_calculate_rate(mods, mod_count);
        pb.is_convert = beatmap.mode == 0;

        convert(pb, beatmap, object_converted);

        ModdedDifficulty modded = modded_difficulty(beatmap, mods, mod_count);
        pb.circle_size = modded.circle_size;
        pb.approach_rate = modded.approach_rate;

        bool hard_rock_offsets = false;
        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id == pppp::mods::MOD_HR) {
                hard_rock_offsets = true;
            }
            if (mods[i].id == pppp::mods::MOD_DA && mods[i].difficulty_adjust.hard_rock_offsets) {
                hard_rock_offsets = true;
            }
        }

        apply_position_offsets(pb, beatmap, hard_rock_offsets);

        for (size_t i = 0; i < mod_count; i++) {
            if (mods[i].id == pppp::mods::MOD_MR) {
                mods::apply_mirror(pb);
            }
        }

        return Result::OK;
    }
}} // namespace pppp::fruits
