#include <pppp/beatmap.h>
#include <pppp/capi.h>
#include <pppp/config.h>
#include <pppp/pppp.h>

#include <cstddef>
#include <cstring>
#include <new>
#include <vector>

#define PPPP_STRINGIFY_INNER(value) #value
#define PPPP_STRINGIFY(value) PPPP_STRINGIFY_INNER(value)
#define PPPP_VERSION_STRING                                                                                  \
    PPPP_STRINGIFY(PPPP_VERSION_MAJOR)                                                                       \
    "." PPPP_STRINGIFY(PPPP_VERSION_MINOR) "." PPPP_STRINGIFY(PPPP_VERSION_PATCH)

namespace {

    PPPP_STATIC_ASSERT(hit_result_count_matches, PPPP_HIT_RESULT_COUNT == pppp::common::HIT_RESULT_COUNT);
    PPPP_STATIC_ASSERT(int32_is_32_bits, sizeof(pppp_int32) == 4);
    PPPP_STATIC_ASSERT(int64_is_64_bits, sizeof(pppp_int64) == 8);
    PPPP_STATIC_ASSERT(result_ok_matches, PPPP_OK == static_cast<int>(pppp::StatusCode::OK));
    PPPP_STATIC_ASSERT(result_invalid_argument_matches,
                       PPPP_INVALID_ARGUMENT == static_cast<int>(pppp::StatusCode::INVALID_ARGUMENT));
    PPPP_STATIC_ASSERT(result_allocation_matches,
                       PPPP_ALLOCATION == static_cast<int>(pppp::StatusCode::ALLOCATION));
    PPPP_STATIC_ASSERT(result_parse_matches, PPPP_PARSE == static_cast<int>(pppp::StatusCode::PARSE));
    PPPP_STATIC_ASSERT(result_no_slider_path_backend_matches,
                       PPPP_NO_SLIDER_PATH_BACKEND ==
                           static_cast<int>(pppp::StatusCode::NO_SLIDER_PATH_BACKEND));
    PPPP_STATIC_ASSERT(result_slider_path_matches,
                       PPPP_SLIDER_PATH == static_cast<int>(pppp::StatusCode::SLIDER_PATH));
    PPPP_STATIC_ASSERT(ruleset_osu_matches, PPPP_RULESET_OSU == static_cast<int>(pppp::Ruleset::OSU));
    PPPP_STATIC_ASSERT(ruleset_taiko_matches, PPPP_RULESET_TAIKO == static_cast<int>(pppp::Ruleset::TAIKO));
    PPPP_STATIC_ASSERT(ruleset_catch_matches, PPPP_RULESET_CATCH == static_cast<int>(pppp::Ruleset::CATCH));
    PPPP_STATIC_ASSERT(ruleset_mania_matches, PPPP_RULESET_MANIA == static_cast<int>(pppp::Ruleset::MANIA));
    PPPP_STATIC_ASSERT(slider_event_tick_matches,
                       PPPP_SLIDER_EVENT_TICK == static_cast<int>(pppp::beatmaps::SLIDER_EVENT_TICK));
    PPPP_STATIC_ASSERT(slider_event_legacy_last_tick_matches,
                       PPPP_SLIDER_EVENT_LEGACY_LAST_TICK ==
                           static_cast<int>(pppp::beatmaps::SLIDER_EVENT_LEGACY_LAST_TICK));
    PPPP_STATIC_ASSERT(slider_event_head_matches,
                       PPPP_SLIDER_EVENT_HEAD == static_cast<int>(pppp::beatmaps::SLIDER_EVENT_HEAD));
    PPPP_STATIC_ASSERT(slider_event_tail_matches,
                       PPPP_SLIDER_EVENT_TAIL == static_cast<int>(pppp::beatmaps::SLIDER_EVENT_TAIL));
    PPPP_STATIC_ASSERT(slider_event_repeat_matches,
                       PPPP_SLIDER_EVENT_REPEAT == static_cast<int>(pppp::beatmaps::SLIDER_EVENT_REPEAT));

    pppp_vector2 to_point(const pppp::utils::Vector2& value) {
        pppp_vector2 point;
        point.x = value.x;
        point.y = value.y;
        return point;
    }

    pppp_slider_event_descriptor to_event(const pppp::beatmaps::SliderEventDescriptor& value) {
        pppp_slider_event_descriptor event;
        event.type = static_cast<pppp_int32>(value.type);
        event.time = value.time;
        event.span_index = value.span_index;
        event.span_start_time = value.span_start_time;
        event.path_progress = value.path_progress;
        event.position = to_point(value.position);
        return event;
    }

    void copy_points(const std::vector<pppp::utils::Vector2>& source, std::vector<pppp_vector2>& out) {
        out.reserve(source.size());
        for (size_t i = 0; i < source.size(); i++) {
            out.push_back(to_point(source[i]));
        }
    }

    void copy_events(const std::vector<pppp::beatmaps::SliderEventDescriptor>& source,
                     std::vector<pppp_slider_event_descriptor>& out) {
        out.reserve(source.size());
        for (size_t i = 0; i < source.size(); i++) {
            out.push_back(to_event(source[i]));
        }
    }

    const pppp_uint32* data_or_null(const std::vector<pppp_uint32>& values) {
        return values.empty() ? 0 : &values[0];
    }

    const double* data_or_null(const std::vector<double>& values) { return values.empty() ? 0 : &values[0]; }

    const pppp_vector2* data_or_null(const std::vector<pppp_vector2>& values) {
        return values.empty() ? 0 : &values[0];
    }

    const pppp_slider_event_descriptor*
    data_or_null(const std::vector<pppp_slider_event_descriptor>& values) {
        return values.empty() ? 0 : &values[0];
    }

    void fill_osu_difficulty(const pppp::osu::difficulty::OsuDifficultyAttributes& value,
                             pppp_osu_difficulty_attributes& out) {
        out.star_rating = value.star_rating;
        out.max_combo = value.max_combo;
        out.aim_difficulty = value.aim_difficulty;
        out.speed_difficulty = value.speed_difficulty;
        out.reading_difficulty = value.reading_difficulty;
        out.flashlight_difficulty = value.flashlight_difficulty;
        out.slider_factor = value.slider_factor;
        out.aim_difficult_strain_count = value.aim_difficult_strain_count;
        out.speed_difficult_strain_count = value.speed_difficult_strain_count;
        out.reading_difficult_note_count = value.reading_difficult_note_count;
        out.aim_difficult_slider_count = value.aim_difficult_slider_count;
        out.aim_top_weighted_slider_factor = value.aim_top_weighted_slider_factor;
        out.speed_top_weighted_slider_factor = value.speed_top_weighted_slider_factor;
        out.speed_note_count = value.speed_note_count;
        out.hit_circle_count = value.hit_circle_count;
        out.slider_count = value.slider_count;
        out.large_tick_count = value.large_tick_count;
        out.spinner_count = value.spinner_count;
        out.nested_score_per_object = value.nested_score_per_object;
        out.legacy_score_base_multiplier = value.legacy_score_base_multiplier;
        out.maximum_legacy_combo_score = value.maximum_legacy_combo_score;
    }

    void fill_taiko_difficulty(const pppp::taiko::difficulty::TaikoDifficultyAttributes& value,
                               pppp_taiko_difficulty_attributes& out) {
        out.star_rating = value.star_rating;
        out.max_combo = value.max_combo;
        out.mechanical_difficulty = value.mechanical_difficulty;
        out.rhythm_difficulty = value.rhythm_difficulty;
        out.reading_difficulty = value.reading_difficulty;
        out.colour_difficulty = value.colour_difficulty;
        out.stamina_difficulty = value.stamina_difficulty;
        out.mono_stamina_factor = value.mono_stamina_factor;
        out.consistency_factor = value.consistency_factor;
        out.stamina_top_strains = value.stamina_top_strains;
    }

    void fill_catch_difficulty(const pppp::fruits::difficulty::CatchDifficultyAttributes& value,
                               pppp_catch_difficulty_attributes& out) {
        out.star_rating = value.star_rating;
        out.max_combo = value.max_combo;
    }

    void fill_mania_difficulty(const pppp::mania::difficulty::ManiaDifficultyAttributes& value,
                               pppp_mania_difficulty_attributes& out) {
        out.star_rating = value.star_rating;
        out.max_combo = value.max_combo;
    }

    void fill_osu_performance(const pppp::osu::difficulty::OsuPerformanceAttributes& value,
                              pppp_osu_performance_attributes& out) {
        out.total = value.total;
        out.aim = value.aim;
        out.speed = value.speed;
        out.accuracy = value.accuracy;
        out.flashlight = value.flashlight;
        out.reading = value.reading;
        out.effective_miss_count = value.effective_miss_count;
        out.combo_based_estimated_miss_count = value.combo_based_estimated_miss_count;
        out.has_score_based_estimated_miss_count = value.score_based_estimated_miss_count.has_value();
        out.score_based_estimated_miss_count =
            out.has_score_based_estimated_miss_count ? value.score_based_estimated_miss_count.value() : 0.0;
        out.aim_estimated_slider_breaks = value.aim_estimated_slider_breaks;
        out.speed_estimated_slider_breaks = value.speed_estimated_slider_breaks;
        out.has_speed_deviation = value.speed_deviation.has_value();
        out.speed_deviation = out.has_speed_deviation ? value.speed_deviation.value() : 0.0;
    }

    void fill_taiko_performance(const pppp::taiko::difficulty::TaikoPerformanceAttributes& value,
                                pppp_taiko_performance_attributes& out) {
        out.total = value.total;
        out.difficulty = value.difficulty;
        out.accuracy = value.accuracy;
        out.has_estimated_unstable_rate = value.estimated_unstable_rate.has_value();
        out.estimated_unstable_rate =
            out.has_estimated_unstable_rate ? value.estimated_unstable_rate.value() : 0.0;
    }

    void fill_catch_performance(const pppp::fruits::difficulty::CatchPerformanceAttributes& value,
                                pppp_catch_performance_attributes& out) {
        out.total = value.total;
    }

    void fill_mania_performance(const pppp::mania::difficulty::ManiaPerformanceAttributes& value,
                                pppp_mania_performance_attributes& out) {
        out.total = value.total;
        out.difficulty = value.difficulty;
    }

    void read_osu_difficulty(const pppp_osu_difficulty_attributes& in,
                             pppp::osu::difficulty::OsuDifficultyAttributes& out) {
        out.star_rating = in.star_rating;
        out.max_combo = in.max_combo;
        out.aim_difficulty = in.aim_difficulty;
        out.speed_difficulty = in.speed_difficulty;
        out.reading_difficulty = in.reading_difficulty;
        out.flashlight_difficulty = in.flashlight_difficulty;
        out.slider_factor = in.slider_factor;
        out.aim_difficult_strain_count = in.aim_difficult_strain_count;
        out.speed_difficult_strain_count = in.speed_difficult_strain_count;
        out.reading_difficult_note_count = in.reading_difficult_note_count;
        out.aim_difficult_slider_count = in.aim_difficult_slider_count;
        out.aim_top_weighted_slider_factor = in.aim_top_weighted_slider_factor;
        out.speed_top_weighted_slider_factor = in.speed_top_weighted_slider_factor;
        out.speed_note_count = in.speed_note_count;
        out.hit_circle_count = in.hit_circle_count;
        out.slider_count = in.slider_count;
        out.large_tick_count = in.large_tick_count;
        out.spinner_count = in.spinner_count;
        out.nested_score_per_object = in.nested_score_per_object;
        out.legacy_score_base_multiplier = in.legacy_score_base_multiplier;
        out.maximum_legacy_combo_score = in.maximum_legacy_combo_score;
    }

    void read_taiko_difficulty(const pppp_taiko_difficulty_attributes& in,
                               pppp::taiko::difficulty::TaikoDifficultyAttributes& out) {
        out.star_rating = in.star_rating;
        out.max_combo = in.max_combo;
        out.mechanical_difficulty = in.mechanical_difficulty;
        out.rhythm_difficulty = in.rhythm_difficulty;
        out.reading_difficulty = in.reading_difficulty;
        out.colour_difficulty = in.colour_difficulty;
        out.stamina_difficulty = in.stamina_difficulty;
        out.mono_stamina_factor = in.mono_stamina_factor;
        out.consistency_factor = in.consistency_factor;
        out.stamina_top_strains = in.stamina_top_strains;
    }

    void read_catch_difficulty(const pppp_catch_difficulty_attributes& in,
                               pppp::fruits::difficulty::CatchDifficultyAttributes& out) {
        out.star_rating = in.star_rating;
        out.max_combo = in.max_combo;
    }

    void read_mania_difficulty(const pppp_mania_difficulty_attributes& in,
                               pppp::mania::difficulty::ManiaDifficultyAttributes& out) {
        out.star_rating = in.star_rating;
        out.max_combo = in.max_combo;
    }

    pppp::DifficultyAttributes read_difficulty(const pppp_difficulty_attributes& in) {
        switch (in.ruleset) {
        case PPPP_RULESET_TAIKO: {
            pppp::TaikoDifficultyAttributes out;
            read_taiko_difficulty(in.taiko, out);
            return out;
        }
        case PPPP_RULESET_CATCH: {
            pppp::CatchDifficultyAttributes out;
            read_catch_difficulty(in.fruits, out);
            return out;
        }
        case PPPP_RULESET_MANIA: {
            pppp::ManiaDifficultyAttributes out;
            read_mania_difficulty(in.mania, out);
            return out;
        }
        default: {
            pppp::OsuDifficultyAttributes out;
            read_osu_difficulty(in.osu, out);
            return out;
        }
        }
    }

    void fill_difficulty(const pppp::DifficultyAttributes& in, pppp_difficulty_attributes& out) {
        std::memset(&out, 0, sizeof(out));
        out.ruleset = static_cast<pppp_int32>(in.ruleset());
        out.star_rating = in.star_rating();
        out.max_combo = in.max_combo();
        if (in.osu()) {
            fill_osu_difficulty(*in.osu(), out.osu);
        } else if (in.taiko()) {
            fill_taiko_difficulty(*in.taiko(), out.taiko);
        } else if (in.fruits()) {
            fill_catch_difficulty(*in.fruits(), out.fruits);
        } else if (in.mania()) {
            fill_mania_difficulty(*in.mania(), out.mania);
        }
    }

    void fill_performance(const pppp::PerformanceAttributes& in, pppp_performance_attributes& out) {
        std::memset(&out, 0, sizeof(out));
        out.ruleset = static_cast<pppp_int32>(in.ruleset());
        out.total = in.total();
        if (in.osu()) {
            fill_osu_performance(*in.osu(), out.osu);
        } else if (in.taiko()) {
            fill_taiko_performance(*in.taiko(), out.taiko);
        } else if (in.fruits()) {
            fill_catch_performance(*in.fruits(), out.fruits);
        } else if (in.mania()) {
            fill_mania_performance(*in.mania(), out.mania);
        }
    }

} // namespace

// NOLINTNEXTLINE(readability-identifier-naming): the header declares the handle under this name.
struct pppp_timed_difficulty {
    std::vector<pppp_timed_difficulty_attributes> entries;
};

// NOLINTNEXTLINE(readability-identifier-naming): the header declares the handle under this name.
struct pppp_strains {
    pppp::Strains strains;
};

// NOLINTNEXTLINE(readability-identifier-naming): the header declares the handle under this name.
struct pppp_mods {
    pppp::Mods mods;
};

struct SliderView {
    std::vector<pppp_uint32> node_sounds;
    std::vector<pppp_vector2> control_points;
    std::vector<pppp_vector2> path;
    std::vector<double> cumulative_lengths;
    std::vector<pppp_vector2> undecimated_path;
    std::vector<double> undecimated_cumulative_lengths;
    std::vector<pppp_slider_event_descriptor> events;
    std::vector<pppp_slider_event_descriptor> catch_events;
};

// NOLINTNEXTLINE(readability-identifier-naming): the header declares the handle under this name.
struct pppp_beatmap {
    pppp::Beatmap map;

    std::vector<SliderView> slider_views;
    std::vector<pppp_slider> sliders;
    std::vector<pppp_hit_object> hit_objects;
    std::vector<pppp_timing_point> timing_points;
    std::vector<pppp_break_period> breaks;
};

namespace {

    void build_hit_objects(const pppp::beatmaps::Beatmap& map, std::vector<pppp_hit_object>& out) {
        out.reserve(map.hit_objects.size());
        for (size_t i = 0; i < map.hit_objects.size(); i++) {
            const pppp::beatmaps::HitObject& value = map.hit_objects[i];
            pppp_hit_object object;
            object.position = to_point(value.position);
            object.type = value.type;
            object.hitsound = value.hitsound;
            object.start_time = value.start_time;
            object.end_time = value.end_time;
            object.new_combo = value.new_combo ? 1 : 0;
            object.combo_offset = value.combo_offset;
            object.slider = value.slider;
            out.push_back(object);
        }
    }

    void build_timing_points(const pppp::beatmaps::Beatmap& map, std::vector<pppp_timing_point>& out) {
        out.reserve(map.timing_points.size());
        for (size_t i = 0; i < map.timing_points.size(); i++) {
            const pppp::beatmaps::TimingPoint& value = map.timing_points[i];
            pppp_timing_point point;
            point.time = value.time;
            point.beat_length = value.beat_length;
            point.meter = value.meter;
            point.uninherited = value.uninherited ? 1 : 0;
            point.effects = value.effects;
            out.push_back(point);
        }
    }

    void build_breaks(const pppp::beatmaps::Beatmap& map, std::vector<pppp_break_period>& out) {
        out.reserve(map.breaks.size());
        for (size_t i = 0; i < map.breaks.size(); i++) {
            pppp_break_period period;
            period.start_time = map.breaks[i].start_time;
            period.end_time = map.breaks[i].end_time;
            out.push_back(period);
        }
    }

    void build_slider_views(const pppp::beatmaps::Beatmap& map, std::vector<SliderView>& out) {
        out.reserve(map.sliders.size());
        for (size_t i = 0; i < map.sliders.size(); i++) {
            const pppp::beatmaps::Slider& value = map.sliders[i];
            SliderView view;
            view.node_sounds.assign(value.node_sounds.begin(), value.node_sounds.end());
            copy_points(value.control_points, view.control_points);
            copy_points(value.path, view.path);
            view.cumulative_lengths = value.cumulative_lengths;
            copy_points(value.undecimated_path, view.undecimated_path);
            view.undecimated_cumulative_lengths = value.undecimated_cumulative_lengths;
            copy_events(value.events, view.events);
            copy_events(value.catch_events, view.catch_events);
            out.push_back(view);
        }
    }

    void build_views(pppp_beatmap& beatmap) {
        build_hit_objects(beatmap.map, beatmap.hit_objects);
        build_timing_points(beatmap.map, beatmap.timing_points);
        build_breaks(beatmap.map, beatmap.breaks);
        build_slider_views(beatmap.map, beatmap.slider_views);

        beatmap.sliders.reserve(beatmap.slider_views.size());
        for (size_t i = 0; i < beatmap.slider_views.size(); i++) {
            const pppp::beatmaps::Slider& source = beatmap.map.sliders[i];
            const SliderView& view = beatmap.slider_views[i];
            pppp_slider slider;
            slider.slides = source.slides;
            slider.expected_length = source.expected_length;
            slider.node_sounds = data_or_null(view.node_sounds);
            slider.node_sound_count = view.node_sounds.size();
            slider.control_points = data_or_null(view.control_points);
            slider.control_point_count = view.control_points.size();
            slider.path = data_or_null(view.path);
            slider.path_count = view.path.size();
            slider.cumulative_lengths = data_or_null(view.cumulative_lengths);
            slider.cumulative_length_count = view.cumulative_lengths.size();
            slider.undecimated_path = data_or_null(view.undecimated_path);
            slider.undecimated_path_count = view.undecimated_path.size();
            slider.undecimated_cumulative_lengths = data_or_null(view.undecimated_cumulative_lengths);
            slider.undecimated_cumulative_length_count = view.undecimated_cumulative_lengths.size();
            slider.events = data_or_null(view.events);
            slider.event_count = view.events.size();
            slider.catch_events = data_or_null(view.catch_events);
            slider.catch_event_count = view.catch_events.size();
            beatmap.sliders.push_back(slider);
        }
    }

} // namespace

const char* pppp_version(void) { return PPPP_VERSION_STRING; }

const char* pppp_result_message(pppp_result result) {
    return pppp::Status(static_cast<pppp::StatusCode::Value>(result)).message();
}

pppp_result pppp_mods_parse(const char* specification, pppp_mods** out) {
    if (!specification || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp_mods* mods = new (std::nothrow) pppp_mods;
    if (!mods) {
        return PPPP_ALLOCATION;
    }
    const pppp::Status status = mods->mods.parse(specification);
    if (!status.ok()) {
        delete mods;
        return static_cast<pppp_result>(status.code());
    }
    *out = mods;
    return PPPP_OK;
}

pppp_result pppp_mods_from_legacy(pppp_uint32 bits, pppp_mods** out) {
    if (!out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp_mods* mods = new (std::nothrow) pppp_mods;
    if (!mods) {
        return PPPP_ALLOCATION;
    }
    mods->mods = pppp::Mods::from_legacy(bits);
    *out = mods;
    return PPPP_OK;
}

void pppp_mods_free(pppp_mods* mods) { delete mods; }

pppp_result pppp_beatmap_from_file(const char* path, pppp_beatmap** out) {
    if (!path || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp_beatmap* beatmap = new (std::nothrow) pppp_beatmap;
    if (!beatmap) {
        return PPPP_ALLOCATION;
    }
    const pppp::Status status = beatmap->map.load_file(path);
    if (!status.ok()) {
        delete beatmap;
        return static_cast<pppp_result>(status.code());
    }
    build_views(*beatmap);
    *out = beatmap;
    return PPPP_OK;
}

pppp_result pppp_beatmap_from_bytes(const void* data, size_t size, pppp_beatmap** out) {
    if ((!data && size) || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp_beatmap* beatmap = new (std::nothrow) pppp_beatmap;
    if (!beatmap) {
        return PPPP_ALLOCATION;
    }
    const pppp::Status status = beatmap->map.load_buffer(data, size);
    if (!status.ok()) {
        delete beatmap;
        return static_cast<pppp_result>(status.code());
    }
    build_views(*beatmap);
    *out = beatmap;
    return PPPP_OK;
}

void pppp_beatmap_free(pppp_beatmap* map) { delete map; }

pppp_int32 pppp_beatmap_format_version(const pppp_beatmap* map) { return map ? map->map.format_version : 0; }

pppp_int32 pppp_beatmap_mode(const pppp_beatmap* map) { return map ? map->map.mode : 0; }

double pppp_beatmap_stack_leniency(const pppp_beatmap* map) { return map ? map->map.stack_leniency : 0.0; }

pppp_result pppp_beatmap_get_difficulty(const pppp_beatmap* map, pppp_beatmap_difficulty* out) {
    if (!map || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    const pppp::beatmaps::BeatmapDifficulty& source = map->map.difficulty;
    out->drain_rate = source.drain_rate;
    out->circle_size = source.circle_size;
    out->overall_difficulty = source.overall_difficulty;
    out->approach_rate = source.approach_rate;
    out->slider_multiplier = source.slider_multiplier;
    out->slider_tick_rate = source.slider_tick_rate;
    return PPPP_OK;
}

pppp_result pppp_beatmap_hit_objects(const pppp_beatmap* map, const pppp_hit_object** out, size_t* count) {
    if (!map || !out || !count) {
        return PPPP_INVALID_ARGUMENT;
    }
    *out = map->hit_objects.empty() ? 0 : &map->hit_objects[0];
    *count = map->hit_objects.size();
    return PPPP_OK;
}

pppp_result pppp_beatmap_sliders(const pppp_beatmap* map, const pppp_slider** out, size_t* count) {
    if (!map || !out || !count) {
        return PPPP_INVALID_ARGUMENT;
    }
    *out = map->sliders.empty() ? 0 : &map->sliders[0];
    *count = map->sliders.size();
    return PPPP_OK;
}

pppp_result pppp_beatmap_timing_points(const pppp_beatmap* map, const pppp_timing_point** out,
                                       size_t* count) {
    if (!map || !out || !count) {
        return PPPP_INVALID_ARGUMENT;
    }
    *out = map->timing_points.empty() ? 0 : &map->timing_points[0];
    *count = map->timing_points.size();
    return PPPP_OK;
}

pppp_result pppp_beatmap_breaks(const pppp_beatmap* map, const pppp_break_period** out, size_t* count) {
    if (!map || !out || !count) {
        return PPPP_INVALID_ARGUMENT;
    }
    *out = map->breaks.empty() ? 0 : &map->breaks[0];
    *count = map->breaks.size();
    return PPPP_OK;
}

namespace {
    int read_difficulty_options(const pppp_difficulty_options* options, pppp::Difficulty& difficulty) {
        if (options && options->mods) {
            difficulty.mods(options->mods->mods);
        }
        if (options && options->has_ruleset) {
            if (options->ruleset < 0 || options->ruleset > 3) {
                return -1;
            }
            difficulty.ruleset(static_cast<pppp::Ruleset::Value>(options->ruleset));
        }
        if (options && options->has_clock_rate) {
            difficulty.clock_rate(options->clock_rate);
        }
        return 0;
    }
} // namespace

pppp_result pppp_calculate_difficulty(const pppp_beatmap* map, const pppp_difficulty_options* options,
                                      pppp_difficulty_attributes* out) {
    if (!map || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp::Difficulty difficulty;
    if (read_difficulty_options(options, difficulty) < 0) {
        return PPPP_INVALID_ARGUMENT;
    }

    pppp::DifficultyAttributes attributes;
    const pppp::Status status = difficulty.calculate(map->map, attributes);
    if (!status.ok()) {
        return static_cast<pppp_result>(status.code());
    }
    fill_difficulty(attributes, *out);
    return PPPP_OK;
}

pppp_result pppp_calculate_timed_difficulty(const pppp_beatmap* map, const pppp_difficulty_options* options,
                                            pppp_timed_difficulty** out) {
    if (!map || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp::Difficulty difficulty;
    if (read_difficulty_options(options, difficulty) < 0) {
        return PPPP_INVALID_ARGUMENT;
    }
    std::vector<pppp::TimedDifficultyAttributes> entries;
    const pppp::Status status = difficulty.calculate_timed(map->map, entries);
    if (!status.ok()) {
        return static_cast<pppp_result>(status.code());
    }
    pppp_timed_difficulty* timed = new (std::nothrow) pppp_timed_difficulty;
    if (!timed) {
        return PPPP_ALLOCATION;
    }

    timed->entries.resize(entries.size());
    for (size_t i = 0; i < entries.size(); i++) {
        timed->entries[i].time = entries[i].time;
        fill_difficulty(entries[i].attributes, timed->entries[i].attributes);
    }
    *out = timed;
    return PPPP_OK;
}

pppp_result pppp_timed_difficulty_entries(const pppp_timed_difficulty* timed,
                                          const pppp_timed_difficulty_attributes** out, size_t* count) {
    if (!timed || !out || !count) {
        return PPPP_INVALID_ARGUMENT;
    }
    *out = timed->entries.empty() ? 0 : &timed->entries[0];
    *count = timed->entries.size();
    return PPPP_OK;
}

void pppp_timed_difficulty_free(pppp_timed_difficulty* timed) { delete timed; }

namespace {
    void series(const std::vector<double>& in, const double** data, size_t* count) {
        *data = in.empty() ? 0 : &in[0];
        *count = in.size();
    }
} // namespace

pppp_result pppp_calculate_strains(const pppp_beatmap* map, const pppp_difficulty_options* options,
                                   pppp_strains** out) {
    if (!map || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp::Difficulty difficulty;
    if (read_difficulty_options(options, difficulty) < 0) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp_strains* strains = new (std::nothrow) pppp_strains;
    if (!strains) {
        return PPPP_ALLOCATION;
    }

    const pppp::Status status = difficulty.strains(map->map, strains->strains);
    if (!status.ok()) {
        delete strains;
        return static_cast<pppp_result>(status.code());
    }
    *out = strains;
    return PPPP_OK;
}

pppp_result pppp_strains_info(const pppp_strains* strains, pppp_int32* ruleset, double* start_time,
                              double* section_length) {
    if (!strains || !ruleset || !start_time || !section_length) {
        return PPPP_INVALID_ARGUMENT;
    }
    *ruleset = static_cast<pppp_int32>(strains->strains.ruleset());
    *start_time = strains->strains.start_time();
    *section_length = strains->strains.section_length();
    return PPPP_OK;
}

pppp_result pppp_strains_osu(const pppp_strains* strains, pppp_osu_strains* out) {
    const pppp::OsuStrains* in = strains ? strains->strains.osu() : 0;
    if (!in || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    series(in->aim, &out->aim, &out->aim_count);
    series(in->aim_no_sliders, &out->aim_no_sliders, &out->aim_no_sliders_count);
    series(in->speed, &out->speed, &out->speed_count);
    series(in->reading, &out->reading, &out->reading_count);
    series(in->flashlight, &out->flashlight, &out->flashlight_count);
    return PPPP_OK;
}

pppp_result pppp_strains_taiko(const pppp_strains* strains, pppp_taiko_strains* out) {
    const pppp::TaikoStrains* in = strains ? strains->strains.taiko() : 0;
    if (!in || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    series(in->colour, &out->colour, &out->colour_count);
    series(in->reading, &out->reading, &out->reading_count);
    series(in->rhythm, &out->rhythm, &out->rhythm_count);
    series(in->stamina, &out->stamina, &out->stamina_count);
    series(in->single_colour_stamina, &out->single_colour_stamina, &out->single_colour_stamina_count);
    return PPPP_OK;
}

pppp_result pppp_strains_catch(const pppp_strains* strains, pppp_catch_strains* out) {
    const pppp::CatchStrains* in = strains ? strains->strains.fruits() : 0;
    if (!in || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    series(in->movement, &out->movement, &out->movement_count);
    return PPPP_OK;
}

pppp_result pppp_strains_mania(const pppp_strains* strains, pppp_mania_strains* out) {
    const pppp::ManiaStrains* in = strains ? strains->strains.mania() : 0;
    if (!in || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    series(in->strain, &out->strain, &out->strain_count);
    return PPPP_OK;
}

void pppp_strains_free(pppp_strains* strains) { delete strains; }

pppp_result pppp_calculate_performance(const pppp_beatmap* map, const pppp_performance_options* options,
                                       pppp_performance_attributes* out) {
    if (!map || !out) {
        return PPPP_INVALID_ARGUMENT;
    }
    pppp::ScoreInfo score;
    if (options && options->has_score) {
        for (int i = 0; i < pppp::common::HIT_RESULT_COUNT; i++) {
            score.statistics[i] = options->score.statistics[i];
            score.maximum_statistics[i] = options->score.maximum_statistics[i];
        }
        score.max_combo = options->score.max_combo;
        score.accuracy = options->score.accuracy;
        if (options->score.has_legacy_total_score) {
            score.legacy_total_score = static_cast<pppp_int64>(options->score.legacy_total_score);
        }
    }

    pppp::Performance performance(map->map);
    if (options && options->has_difficulty) {
        if (!options->difficulty || options->difficulty->ruleset < 0 || options->difficulty->ruleset > 3) {
            return PPPP_INVALID_ARGUMENT;
        }
        performance.attributes(read_difficulty(*options->difficulty));
    }
    performance.score(score);
    if (options && options->mods && !options->mods->mods.empty()) {
        performance.mods(options->mods->mods);
    }
    if (options && options->has_combo) {
        performance.combo(options->combo);
    }
    if (options && options->has_accuracy) {
        performance.accuracy(options->accuracy);
    }
    if (options && options->has_misses) {
        performance.misses(options->misses);
    }

    pppp::PerformanceAttributes attributes;
    const pppp::Status status = performance.calculate(attributes);
    if (!status.ok()) {
        return static_cast<pppp_result>(status.code());
    }
    fill_performance(attributes, *out);
    return PPPP_OK;
}
