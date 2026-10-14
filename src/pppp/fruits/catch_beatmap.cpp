// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/fruits/catch_beatmap.h"
#include "pppp/fruits/catch_beatmap_converter.h"
#include "pppp/fruits/catch_beatmap_processor.h"
#include "pppp/fruits/mods/catch_mod_mirror.h"
#include "pppp/mods/mod_setting_value.h"
#include "pppp/utils/linq/csharp.h"
#include <algorithm>

namespace pppp { namespace fruits {
    namespace linq = ::pppp::utils::linq;

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
        std::vector<object::CatchHitObject*> objects =
            linq::from(pb.all_objects)
                .where(linq::bind(&object::CatchHitObject::kind, linq::_1) == object::OBJECT_FRUIT ||
                       linq::bind(&object::CatchHitObject::kind, linq::_1) == object::OBJECT_DROPLET ||
                       linq::bind(&object::CatchHitObject::kind, linq::_1) == object::OBJECT_TINY_DROPLET ||
                       linq::bind(&object::CatchHitObject::kind, linq::_1) == object::OBJECT_BANANA)
                .order_by(linq::bind(&object::CatchHitObject::time, linq::_1))
                .to_list();
        out.swap(objects);
    }

    ModdedDifficulty modded_difficulty(const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                                       size_t mod_count) {
        ModdedDifficulty out;
        out.circle_size = beatmap.difficulty.circle_size;
        out.approach_rate = beatmap.difficulty.approach_rate;
        out.overall_difficulty = beatmap.difficulty.overall_difficulty;
        out.drain_rate = beatmap.difficulty.drain_rate;

        for (size_t i = 0; i < mod_count; i++) {
            switch (mods[i].id) {
            case pppp::mods::MOD_EZ:
                out.circle_size *= pppp::mods::EASY_ADJUST_RATIO;
                out.approach_rate *= pppp::mods::EASY_ADJUST_RATIO;
                out.drain_rate *= pppp::mods::EASY_ADJUST_RATIO;

                out.overall_difficulty *= pppp::mods::EASY_ADJUST_RATIO;
                break;
            case pppp::mods::MOD_HR:
                out.drain_rate = std::min(out.drain_rate * pppp::mods::HARD_ROCK_ADJUST_RATIO, 10.0f);

                out.overall_difficulty =
                    std::min(out.overall_difficulty * pppp::mods::HARD_ROCK_ADJUST_RATIO, 10.0f);
                out.circle_size = std::min(out.circle_size * 1.3f, 10.0f); // CS uses a custom 1.3 ratio.
                out.approach_rate = std::min(out.approach_rate * pppp::mods::HARD_ROCK_ADJUST_RATIO, 10.0f);
                break;
            case pppp::mods::MOD_DA: {
                const pppp::mods::DifficultyAdjustSettings& st = mods[i].difficulty_adjust;
                if (st.drain_rate.has_value()) {
                    out.drain_rate = pppp::mods::clamp_to_extended_limits(st.drain_rate.value(), 0, 10,
                                                                          nonstd::optional<float>(), 11.0f);
                }
                if (st.overall_difficulty.has_value()) {
                    out.overall_difficulty = pppp::mods::clamp_to_extended_limits(
                        st.overall_difficulty.value(), 0, 10, nonstd::optional<float>(), 11.0f);
                }
                if (st.circle_size.has_value()) {
                    out.circle_size = pppp::mods::clamp_to_extended_limits(st.circle_size.value(), 0, 10,
                                                                           nonstd::optional<float>(), 11.0f);
                }
                if (st.approach_rate.has_value()) {
                    out.approach_rate = pppp::mods::clamp_to_extended_limits(
                        st.approach_rate.value(), 0, 10, nonstd::optional<float>(), 11.0f);
                }
                break;
            }
            default: break;
            }
        }

        return out;
    }

    Status build(CatchBeatmap& pb, const beatmaps::Beatmap& beatmap, const pppp::mods::Mod* mods,
                 size_t mod_count,
                 const beatmaps::ObjectConverted<object::CatchHitObject>& object_converted) {
        pb = CatchBeatmap();
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
            if (mods[i].id == pppp::mods::MOD_DA) {
                hard_rock_offsets = mods[i].difficulty_adjust.hard_rock_offsets;
            }
        }

        apply_position_offsets(pb, beatmap, hard_rock_offsets);

        std::vector<pppp::mods::Mod> playable_mods(mods, mods + mod_count);
        for (size_t i = 0; i < playable_mods.size(); i++) {
            if (playable_mods[i].id == pppp::mods::MOD_MR) {
                mods::apply_mirror(pb);
            }
            if (playable_mods[i].id == pppp::mods::MOD_WU || playable_mods[i].id == pppp::mods::MOD_WD) {
                pppp::mods::mod_time_ramp_apply_to_beatmap(
                    &playable_mods[i], !pb.objects.empty(), pb.objects.empty() ? 0.0 : pb.objects[0].time,
                    pb.objects.empty() ? 0.0
                                       : linq::from(pb.objects)
                                             .max(linq::bind(&object::CatchHitObject::end_time, linq::_1)));
            }
        }

        pb.clock_rate = pppp::mods::mod_calculate_rate(playable_mods.empty() ? 0 : &playable_mods[0],
                                                       playable_mods.size());
        return StatusCode::OK;
    }
}} // namespace pppp::fruits
