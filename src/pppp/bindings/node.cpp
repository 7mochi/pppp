// Construct detached JS values using Node-API.
#include <pppp/beatmap.h>
#include <pppp/config.h>
#include <pppp/pppp.h>

#include <cstdio>
#include <cstring>
#include <new>

#include <node_api.h>

namespace {

    napi_value number(napi_env env, double value) {
        napi_value result = NULL;
        napi_create_double(env, value, &result);
        return result;
    }

    napi_value integer(napi_env env, int value) {
        napi_value result = NULL;
        napi_create_int32(env, value, &result);
        return result;
    }

    napi_value unsigned_integer(napi_env env, unsigned value) {
        napi_value result = NULL;
        napi_create_uint32(env, value, &result);
        return result;
    }

    napi_value boolean(napi_env env, bool value) {
        napi_value result = NULL;
        napi_get_boolean(env, value, &result);
        return result;
    }

    napi_value point(napi_env env, const pppp::utils::Vector2& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        if (napi_set_named_property(env, object, "x", number(env, value.x)) != napi_ok ||
            napi_set_named_property(env, object, "y", number(env, value.y)) != napi_ok) {
            return NULL;
        }
        return object;
    }

    napi_value difficulty(napi_env env, const pppp::beatmaps::BeatmapDifficulty& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "drain_rate", number(env, value.drain_rate));
        napi_set_named_property(env, object, "circle_size", number(env, value.circle_size));
        napi_set_named_property(env, object, "overall_difficulty", number(env, value.overall_difficulty));
        napi_set_named_property(env, object, "approach_rate", number(env, value.approach_rate));
        napi_set_named_property(env, object, "slider_multiplier", number(env, value.slider_multiplier));
        napi_set_named_property(env, object, "slider_tick_rate", number(env, value.slider_tick_rate));
        return object;
    }

    napi_value hit_object(napi_env env, const pppp::beatmaps::HitObject& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "position", point(env, value.position));
        napi_set_named_property(env, object, "type", unsigned_integer(env, value.type));
        napi_set_named_property(env, object, "hitsound", unsigned_integer(env, value.hitsound));
        napi_set_named_property(env, object, "start_time", number(env, value.start_time));
        napi_set_named_property(env, object, "end_time", number(env, value.end_time));
        napi_set_named_property(env, object, "new_combo", boolean(env, value.new_combo));
        napi_set_named_property(env, object, "combo_offset", integer(env, value.combo_offset));
        napi_set_named_property(env, object, "slider", integer(env, value.slider));
        return object;
    }

    napi_value slider_event(napi_env env, const pppp::beatmaps::SliderEventDescriptor& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "type", integer(env, static_cast<int>(value.type)));
        napi_set_named_property(env, object, "time", number(env, value.time));
        napi_set_named_property(env, object, "span_index", integer(env, value.span_index));
        napi_set_named_property(env, object, "span_start_time", number(env, value.span_start_time));
        napi_set_named_property(env, object, "path_progress", number(env, value.path_progress));
        napi_set_named_property(env, object, "position", point(env, value.position));
        return object;
    }

    napi_value array_of_numbers(napi_env env, const std::vector<double>& values) {
        napi_value array = NULL;
        if (napi_create_array_with_length(env, values.size(), &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < values.size(); i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), number(env, values[i]));
        }
        return array;
    }

    napi_value array_of_integers(napi_env env, const std::vector<unsigned>& values) {
        napi_value array = NULL;
        if (napi_create_array_with_length(env, values.size(), &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < values.size(); i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), unsigned_integer(env, values[i]));
        }
        return array;
    }

    napi_value array_of_points(napi_env env, const std::vector<pppp::utils::Vector2>& values) {
        napi_value array = NULL;
        if (napi_create_array_with_length(env, values.size(), &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < values.size(); i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), point(env, values[i]));
        }
        return array;
    }

    napi_value array_of_events(napi_env env,
                               const std::vector<pppp::beatmaps::SliderEventDescriptor>& values) {
        napi_value array = NULL;
        if (napi_create_array_with_length(env, values.size(), &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < values.size(); i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), slider_event(env, values[i]));
        }
        return array;
    }

    napi_value slider(napi_env env, const pppp::beatmaps::Slider& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "slides", integer(env, value.slides));
        napi_set_named_property(env, object, "expected_length", number(env, value.expected_length));
        napi_set_named_property(env, object, "node_sounds", array_of_integers(env, value.node_sounds));
        napi_set_named_property(env, object, "control_points", array_of_points(env, value.control_points));
        napi_set_named_property(env, object, "path", array_of_points(env, value.path));
        napi_set_named_property(env, object, "cumulative_lengths",
                                array_of_numbers(env, value.cumulative_lengths));
        napi_set_named_property(env, object, "undecimated_path",
                                array_of_points(env, value.undecimated_path));
        napi_set_named_property(env, object, "undecimated_cumulative_lengths",
                                array_of_numbers(env, value.undecimated_cumulative_lengths));
        napi_set_named_property(env, object, "events", array_of_events(env, value.events));
        napi_set_named_property(env, object, "catch_events", array_of_events(env, value.catch_events));
        return object;
    }

    napi_value timing_point(napi_env env, const pppp::beatmaps::TimingPoint& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "time", number(env, value.time));
        napi_set_named_property(env, object, "beat_length", number(env, value.beat_length));
        napi_set_named_property(env, object, "meter", integer(env, value.meter));
        napi_set_named_property(env, object, "uninherited", boolean(env, value.uninherited));
        napi_set_named_property(env, object, "effects", unsigned_integer(env, value.effects));
        return object;
    }

    napi_value break_period(napi_env env, const pppp::beatmaps::timing::BreakPeriod& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "start_time", number(env, value.start_time));
        napi_set_named_property(env, object, "end_time", number(env, value.end_time));
        return object;
    }

    void release_beatmap(napi_env env, void* data, void* hint) {
        delete static_cast<pppp::beatmaps::Beatmap*>(data);
    }

    napi_value from_file(napi_env env, napi_callback_info info) {
        size_t argc = 1;
        napi_value argv[1];
        if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok || argc < 1) {
            napi_throw_type_error(env, NULL, "expected a path");
            return NULL;
        }
        napi_valuetype kind;
        if (napi_typeof(env, argv[0], &kind) != napi_ok || kind != napi_string) {
            napi_throw_type_error(env, NULL, "expected a path");
            return NULL;
        }
        size_t length = 0;
        if (napi_get_value_string_utf8(env, argv[0], NULL, 0, &length) != napi_ok) {
            return NULL;
        }
        char* path = new (std::nothrow) char[length + 1];
        if (!path) {
            napi_throw_range_error(env, NULL, "cannot allocate the path");
            return NULL;
        }
        if (napi_get_value_string_utf8(env, argv[0], path, length + 1, &length) != napi_ok) {
            delete[] path;
            return NULL;
        }

        pppp::beatmaps::Beatmap* map = new (std::nothrow) pppp::beatmaps::Beatmap;
        if (!map) {
            delete[] path;
            napi_throw_range_error(env, NULL, "cannot allocate the beatmap");
            return NULL;
        }
        // Both native entry points are noexcept, including allocation and I/O failures.
        const pppp::Result::Value status = pppp::beatmaps::from_file(*map, path);
        delete[] path;
        if (status != pppp::Result::OK) {
            delete map;
            if (status == pppp::Result::ALLOCATION) {
                napi_throw_range_error(env, NULL, "cannot allocate the beatmap");
                return NULL;
            }
            char message[64];
            std::snprintf(message, sizeof(message), "cannot parse the beatmap (result %d)",
                          static_cast<int>(status));
            napi_throw_error(env, NULL, message);
            return NULL;
        }

        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            delete map;
            return NULL;
        }
        if (napi_wrap(env, object, map, release_beatmap, NULL, NULL) != napi_ok) {
            delete map;
            return NULL;
        }
        napi_set_named_property(env, object, "format_version", integer(env, map->format_version));
        napi_set_named_property(env, object, "mode", integer(env, map->mode));
        napi_set_named_property(env, object, "stack_leniency", number(env, map->stack_leniency));
        napi_set_named_property(env, object, "difficulty", difficulty(env, map->difficulty));

        const size_t hit_object_count = map->hit_objects.size();
        napi_value hit_objects = NULL;
        napi_create_array_with_length(env, hit_object_count, &hit_objects);
        for (size_t i = 0; i < hit_object_count; i++) {
            napi_set_element(env, hit_objects, static_cast<uint32_t>(i),
                             hit_object(env, map->hit_objects[i]));
        }
        napi_set_named_property(env, object, "hit_objects", hit_objects);

        const size_t slider_count = map->sliders.size();
        napi_value sliders = NULL;
        napi_create_array_with_length(env, slider_count, &sliders);
        for (size_t i = 0; i < slider_count; i++) {
            napi_set_element(env, sliders, static_cast<uint32_t>(i), slider(env, map->sliders[i]));
        }
        napi_set_named_property(env, object, "sliders", sliders);

        const size_t timing_point_count = map->timing_points.size();
        napi_value timing_points = NULL;
        napi_create_array_with_length(env, timing_point_count, &timing_points);
        for (size_t i = 0; i < timing_point_count; i++) {
            napi_set_element(env, timing_points, static_cast<uint32_t>(i),
                             timing_point(env, map->timing_points[i]));
        }
        napi_set_named_property(env, object, "timing_points", timing_points);

        const size_t break_count = map->breaks.size();
        napi_value breaks = NULL;
        napi_create_array_with_length(env, break_count, &breaks);
        for (size_t i = 0; i < break_count; i++) {
            napi_set_element(env, breaks, static_cast<uint32_t>(i), break_period(env, map->breaks[i]));
        }
        napi_set_named_property(env, object, "breaks", breaks);

        return object;
    }

    napi_value osu_attributes(napi_env env, const pppp::osu::difficulty::OsuDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "star_rating", number(env, value.star_rating));
        napi_set_named_property(env, object, "max_combo", integer(env, value.max_combo));
        napi_set_named_property(env, object, "aim_difficulty", number(env, value.aim_difficulty));
        napi_set_named_property(env, object, "speed_difficulty", number(env, value.speed_difficulty));
        napi_set_named_property(env, object, "reading_difficulty", number(env, value.reading_difficulty));
        napi_set_named_property(env, object, "flashlight_difficulty",
                                number(env, value.flashlight_difficulty));
        napi_set_named_property(env, object, "slider_factor", number(env, value.slider_factor));
        napi_set_named_property(env, object, "aim_difficult_strain_count",
                                number(env, value.aim_difficult_strain_count));
        napi_set_named_property(env, object, "speed_difficult_strain_count",
                                number(env, value.speed_difficult_strain_count));
        napi_set_named_property(env, object, "reading_difficult_note_count",
                                number(env, value.reading_difficult_note_count));
        napi_set_named_property(env, object, "aim_difficult_slider_count",
                                number(env, value.aim_difficult_slider_count));
        napi_set_named_property(env, object, "aim_top_weighted_slider_factor",
                                number(env, value.aim_top_weighted_slider_factor));
        napi_set_named_property(env, object, "speed_top_weighted_slider_factor",
                                number(env, value.speed_top_weighted_slider_factor));
        napi_set_named_property(env, object, "speed_note_count", number(env, value.speed_note_count));
        napi_set_named_property(env, object, "hit_circle_count", integer(env, value.hit_circle_count));
        napi_set_named_property(env, object, "slider_count", integer(env, value.slider_count));
        napi_set_named_property(env, object, "large_tick_count", integer(env, value.large_tick_count));
        napi_set_named_property(env, object, "spinner_count", integer(env, value.spinner_count));
        napi_set_named_property(env, object, "nested_score_per_object",
                                number(env, value.nested_score_per_object));
        napi_set_named_property(env, object, "legacy_score_base_multiplier",
                                number(env, value.legacy_score_base_multiplier));
        napi_set_named_property(env, object, "maximum_legacy_combo_score",
                                number(env, value.maximum_legacy_combo_score));
        return object;
    }

    napi_value taiko_attributes(napi_env env,
                                const pppp::taiko::difficulty::TaikoDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "star_rating", number(env, value.star_rating));
        napi_set_named_property(env, object, "max_combo", integer(env, value.max_combo));
        napi_set_named_property(env, object, "mechanical_difficulty",
                                number(env, value.mechanical_difficulty));
        napi_set_named_property(env, object, "rhythm_difficulty", number(env, value.rhythm_difficulty));
        napi_set_named_property(env, object, "reading_difficulty", number(env, value.reading_difficulty));
        napi_set_named_property(env, object, "colour_difficulty", number(env, value.colour_difficulty));
        napi_set_named_property(env, object, "stamina_difficulty", number(env, value.stamina_difficulty));
        napi_set_named_property(env, object, "mono_stamina_factor", number(env, value.mono_stamina_factor));
        napi_set_named_property(env, object, "consistency_factor", number(env, value.consistency_factor));
        napi_set_named_property(env, object, "stamina_top_strains", number(env, value.stamina_top_strains));
        return object;
    }

    napi_value catch_attributes(napi_env env,
                                const pppp::fruits::difficulty::CatchDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "star_rating", number(env, value.star_rating));
        napi_set_named_property(env, object, "max_combo", integer(env, value.max_combo));
        return object;
    }

    napi_value mania_attributes(napi_env env,
                                const pppp::mania::difficulty::ManiaDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "star_rating", number(env, value.star_rating));
        napi_set_named_property(env, object, "max_combo", integer(env, value.max_combo));
        return object;
    }

    napi_value difficulty_attributes(napi_env env, const pppp::DifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, static_cast<int>(value.ruleset)));
        napi_set_named_property(env, object, "star_rating", number(env, value.star_rating()));
        napi_set_named_property(env, object, "max_combo", integer(env, value.max_combo()));
        napi_set_named_property(env, object, "osu", osu_attributes(env, value.osu));
        napi_set_named_property(env, object, "taiko", taiko_attributes(env, value.taiko));
        napi_set_named_property(env, object, "fruits", catch_attributes(env, value.fruits));
        napi_set_named_property(env, object, "mania", mania_attributes(env, value.mania));
        return object;
    }

    const int MAX_MODS = 64;

    bool is_absent(napi_env env, napi_value value, bool* absent) {
        napi_valuetype kind;
        if (napi_typeof(env, value, &kind) != napi_ok) {
            return false;
        }
        *absent = kind == napi_undefined || kind == napi_null;
        return true;
    }

    int read_mods(napi_env env, napi_value value, pppp::mods::Mod* mods, size_t* count) {
        *count = 0;
        bool absent = false;
        if (!is_absent(env, value, &absent) || absent) {
            return absent ? 0 : -1;
        }
        size_t length = 0;
        if (napi_get_value_string_utf8(env, value, NULL, 0, &length) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected a mod specification");
            return -1;
        }
        char* spec = new (std::nothrow) char[length + 1];
        if (!spec) {
            napi_throw_range_error(env, NULL, "cannot allocate the mod specification");
            return -1;
        }
        const napi_status status = napi_get_value_string_utf8(env, value, spec, length + 1, &length);
        if (status != napi_ok) {
            delete[] spec;
            return -1;
        }
        const int parsed = pppp::mods::mod_from_acronyms(mods, MAX_MODS, spec);
        delete[] spec;
        if (parsed < 0) {
            napi_throw_error(env, NULL, "invalid mod specification");
            return -1;
        }
        *count = static_cast<size_t>(parsed);
        return 0;
    }

    int read_ruleset(napi_env env, napi_value value, pppp::Difficulty& difficulty) {
        bool absent = false;
        if (!is_absent(env, value, &absent)) {
            return -1;
        }
        if (absent) {
            return 0;
        }
        int32_t ruleset = 0;
        if (napi_get_value_int32(env, value, &ruleset) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected a ruleset");
            return -1;
        }
        if (ruleset < 0 || ruleset > 3) {
            napi_throw_error(env, NULL, "ruleset must be one of 0, 1, 2, 3");
            return -1;
        }
        difficulty.ruleset(static_cast<pppp::Ruleset::Value>(ruleset));
        return 0;
    }

    int read_clock_rate(napi_env env, napi_value value, pppp::Difficulty& difficulty) {
        bool absent = false;
        if (!is_absent(env, value, &absent)) {
            return -1;
        }
        if (absent) {
            return 0;
        }
        double clock_rate = 0.0;
        if (napi_get_value_double(env, value, &clock_rate) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected a clock rate");
            return -1;
        }
        difficulty.clock_rate(clock_rate);
        return 0;
    }

    napi_value calculate_difficulty(napi_env env, napi_callback_info info) {
        size_t argc = 4;
        napi_value argv[4];
        if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok || argc < 1) {
            napi_throw_type_error(env, NULL, "expected a Beatmap");
            return NULL;
        }
        void* data = NULL;
        if (napi_unwrap(env, argv[0], &data) != napi_ok || !data) {
            napi_throw_type_error(env, NULL, "expected a Beatmap");
            return NULL;
        }
        const pppp::beatmaps::Beatmap* map = static_cast<const pppp::beatmaps::Beatmap*>(data);

        napi_value undefined = NULL;
        napi_get_undefined(env, &undefined);
        pppp::mods::Mod mods[MAX_MODS];
        size_t mod_count = 0;
        if (read_mods(env, argc > 1 ? argv[1] : undefined, mods, &mod_count) < 0) {
            return NULL;
        }
        pppp::Difficulty difficulty;
        difficulty.mods(mod_count ? mods : 0, mod_count);
        if (read_ruleset(env, argc > 2 ? argv[2] : undefined, difficulty) < 0) {
            return NULL;
        }
        if (read_clock_rate(env, argc > 3 ? argv[3] : undefined, difficulty) < 0) {
            return NULL;
        }

        const pppp::DifficultyAttributes attributes = difficulty.calculate(*map);
        return difficulty_attributes(env, attributes);
    }

    napi_value optional_number(napi_env env, const nonstd::optional<double>& value) {
        if (!value.has_value()) {
            napi_value null_value = NULL;
            napi_get_null(env, &null_value);
            return null_value;
        }
        return number(env, value.value());
    }

    napi_value osu_performance(napi_env env, const pppp::osu::difficulty::OsuPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "total", number(env, value.total));
        napi_set_named_property(env, object, "aim", number(env, value.aim));
        napi_set_named_property(env, object, "speed", number(env, value.speed));
        napi_set_named_property(env, object, "accuracy", number(env, value.accuracy));
        napi_set_named_property(env, object, "flashlight", number(env, value.flashlight));
        napi_set_named_property(env, object, "reading", number(env, value.reading));
        napi_set_named_property(env, object, "effective_miss_count", number(env, value.effective_miss_count));
        napi_set_named_property(env, object, "combo_based_estimated_miss_count",
                                number(env, value.combo_based_estimated_miss_count));
        napi_set_named_property(env, object, "score_based_estimated_miss_count",
                                optional_number(env, value.score_based_estimated_miss_count));
        napi_set_named_property(env, object, "aim_estimated_slider_breaks",
                                number(env, value.aim_estimated_slider_breaks));
        napi_set_named_property(env, object, "speed_estimated_slider_breaks",
                                number(env, value.speed_estimated_slider_breaks));
        napi_set_named_property(env, object, "speed_deviation", optional_number(env, value.speed_deviation));
        return object;
    }

    napi_value taiko_performance(napi_env env,
                                 const pppp::taiko::difficulty::TaikoPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "total", number(env, value.total));
        napi_set_named_property(env, object, "difficulty", number(env, value.difficulty));
        napi_set_named_property(env, object, "accuracy", number(env, value.accuracy));
        napi_set_named_property(env, object, "estimated_unstable_rate",
                                optional_number(env, value.estimated_unstable_rate));
        return object;
    }

    napi_value catch_performance(napi_env env,
                                 const pppp::fruits::difficulty::CatchPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "total", number(env, value.total));
        return object;
    }

    napi_value mania_performance(napi_env env,
                                 const pppp::mania::difficulty::ManiaPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "total", number(env, value.total));
        napi_set_named_property(env, object, "difficulty", number(env, value.difficulty));
        return object;
    }

    napi_value performance_attributes(napi_env env, const pppp::PerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, static_cast<int>(value.ruleset)));
        napi_set_named_property(env, object, "total", number(env, value.total()));
        napi_set_named_property(env, object, "osu", osu_performance(env, value.osu));
        napi_set_named_property(env, object, "taiko", taiko_performance(env, value.taiko));
        napi_set_named_property(env, object, "fruits", catch_performance(env, value.fruits));
        napi_set_named_property(env, object, "mania", mania_performance(env, value.mania));
        return object;
    }

    bool has_named_property(napi_env env, napi_value object, const char* name, napi_value* out) {
        bool has = false;
        if (napi_has_named_property(env, object, name, &has) != napi_ok || !has) {
            return false;
        }
        return napi_get_named_property(env, object, name, out) == napi_ok;
    }

    int read_statistics(napi_env env, napi_value value, int* out) {
        bool is_array = false;
        uint32_t length = 0;
        if (napi_is_array(env, value, &is_array) != napi_ok || !is_array ||
            napi_get_array_length(env, value, &length) != napi_ok ||
            length != pppp::common::HIT_RESULT_COUNT) {
            char message[64];
            std::snprintf(message, sizeof(message), "statistics must be a list of %d counts",
                          pppp::common::HIT_RESULT_COUNT);
            napi_throw_error(env, NULL, message);
            return -1;
        }
        for (uint32_t i = 0; i < length; i++) {
            napi_value element = NULL;
            int32_t count = 0;
            if (napi_get_element(env, value, i, &element) != napi_ok ||
                napi_get_value_int32(env, element, &count) != napi_ok) {
                napi_throw_type_error(env, NULL, "statistics must be a list of counts");
                return -1;
            }
            out[i] = count;
        }
        return 0;
    }

    int read_optional_integer(napi_env env, napi_value value, int* out, bool* present) {
        bool absent = false;
        if (!is_absent(env, value, &absent)) {
            return -1;
        }
        *present = !absent;
        if (absent) {
            return 0;
        }
        int32_t integer_value = 0;
        if (napi_get_value_int32(env, value, &integer_value) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected an integer");
            return -1;
        }
        *out = integer_value;
        return 0;
    }

    int read_score(napi_env env, napi_value value, pppp::common::ScoreInfo* score) {
        napi_value statistics = NULL;
        napi_value maximum_statistics = NULL;
        if (!has_named_property(env, value, "statistics", &statistics) ||
            !has_named_property(env, value, "maximum_statistics", &maximum_statistics)) {
            napi_throw_type_error(env, NULL, "expected a ScoreInfo");
            return -1;
        }
        if (read_statistics(env, statistics, score->statistics) < 0 ||
            read_statistics(env, maximum_statistics, score->maximum_statistics) < 0) {
            return -1;
        }
        napi_value field = NULL;
        if (!has_named_property(env, value, "max_combo", &field)) {
            napi_throw_type_error(env, NULL, "expected a ScoreInfo");
            return -1;
        }
        int32_t max_combo = 0;
        if (napi_get_value_int32(env, field, &max_combo) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected a ScoreInfo");
            return -1;
        }
        score->max_combo = max_combo;
        if (!has_named_property(env, value, "accuracy", &field) ||
            napi_get_value_double(env, field, &score->accuracy) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected a ScoreInfo");
            return -1;
        }
        if (has_named_property(env, value, "legacy_total_score", &field)) {
            bool absent = false;
            if (is_absent(env, field, &absent) && !absent) {
                double legacy = 0.0;
                if (napi_get_value_double(env, field, &legacy) != napi_ok) {
                    napi_throw_type_error(env, NULL, "expected a ScoreInfo");
                    return -1;
                }
                score->legacy_total_score = static_cast<pppp_int64>(legacy);
            }
        }
        return 0;
    }

    napi_value attribute_value(napi_env env, napi_value object, const char* name) {
        if (!object) {
            return NULL;
        }
        napi_value value = NULL;
        if (napi_get_named_property(env, object, name, &value) != napi_ok) {
            napi_value ignored = NULL;
            napi_get_and_clear_last_exception(env, &ignored);
            return NULL;
        }
        return value;
    }

    double attribute_number(napi_env env, napi_value object, const char* name) {
        napi_value value = attribute_value(env, object, name);
        double out = 0.0;
        if (!value || napi_get_value_double(env, value, &out) != napi_ok) {
            napi_value ignored = NULL;
            napi_get_and_clear_last_exception(env, &ignored);
            return 0.0;
        }
        return out;
    }

    int attribute_integer(napi_env env, napi_value object, const char* name) {
        napi_value value = attribute_value(env, object, name);
        int32_t out = 0;
        if (!value || napi_get_value_int32(env, value, &out) != napi_ok) {
            napi_value ignored = NULL;
            napi_get_and_clear_last_exception(env, &ignored);
            return 0;
        }
        return static_cast<int>(out);
    }

    int read_attributes(napi_env env, napi_value value, pppp::DifficultyAttributes& out) {
        napi_valuetype type = napi_undefined;
        if (napi_typeof(env, value, &type) != napi_ok || type != napi_object) {
            napi_throw_type_error(env, NULL, "expected a DifficultyAttributes");
            return -1;
        }

        const int ruleset = attribute_integer(env, value, "ruleset");
        if (ruleset < 0 || ruleset > 3) {
            napi_throw_range_error(env, NULL, "ruleset must be one of 0, 1, 2, 3");
            return -1;
        }
        out.ruleset = static_cast<pppp::Ruleset::Value>(ruleset);

        napi_value osu = ruleset == pppp::Ruleset::RULESET_OSU ? attribute_value(env, value, "osu") : NULL;
        if (osu) {
            out.osu.star_rating = attribute_number(env, osu, "star_rating");
            out.osu.max_combo = attribute_integer(env, osu, "max_combo");
            out.osu.aim_difficulty = attribute_number(env, osu, "aim_difficulty");
            out.osu.speed_difficulty = attribute_number(env, osu, "speed_difficulty");
            out.osu.reading_difficulty = attribute_number(env, osu, "reading_difficulty");
            out.osu.flashlight_difficulty = attribute_number(env, osu, "flashlight_difficulty");
            out.osu.slider_factor = attribute_number(env, osu, "slider_factor");
            out.osu.aim_difficult_strain_count = attribute_number(env, osu, "aim_difficult_strain_count");
            out.osu.speed_difficult_strain_count = attribute_number(env, osu, "speed_difficult_strain_count");
            out.osu.reading_difficult_note_count = attribute_number(env, osu, "reading_difficult_note_count");
            out.osu.aim_difficult_slider_count = attribute_number(env, osu, "aim_difficult_slider_count");
            out.osu.aim_top_weighted_slider_factor =
                attribute_number(env, osu, "aim_top_weighted_slider_factor");
            out.osu.speed_top_weighted_slider_factor =
                attribute_number(env, osu, "speed_top_weighted_slider_factor");
            out.osu.speed_note_count = attribute_number(env, osu, "speed_note_count");
            out.osu.hit_circle_count = attribute_integer(env, osu, "hit_circle_count");
            out.osu.slider_count = attribute_integer(env, osu, "slider_count");
            out.osu.large_tick_count = attribute_integer(env, osu, "large_tick_count");
            out.osu.spinner_count = attribute_integer(env, osu, "spinner_count");
            out.osu.nested_score_per_object = attribute_number(env, osu, "nested_score_per_object");
            out.osu.legacy_score_base_multiplier = attribute_number(env, osu, "legacy_score_base_multiplier");
            out.osu.maximum_legacy_combo_score = attribute_number(env, osu, "maximum_legacy_combo_score");
        }

        napi_value taiko =
            ruleset == pppp::Ruleset::RULESET_TAIKO ? attribute_value(env, value, "taiko") : NULL;
        if (taiko) {
            out.taiko.star_rating = attribute_number(env, taiko, "star_rating");
            out.taiko.max_combo = attribute_integer(env, taiko, "max_combo");
            out.taiko.mechanical_difficulty = attribute_number(env, taiko, "mechanical_difficulty");
            out.taiko.rhythm_difficulty = attribute_number(env, taiko, "rhythm_difficulty");
            out.taiko.reading_difficulty = attribute_number(env, taiko, "reading_difficulty");
            out.taiko.colour_difficulty = attribute_number(env, taiko, "colour_difficulty");
            out.taiko.stamina_difficulty = attribute_number(env, taiko, "stamina_difficulty");
            out.taiko.mono_stamina_factor = attribute_number(env, taiko, "mono_stamina_factor");
            out.taiko.consistency_factor = attribute_number(env, taiko, "consistency_factor");
            out.taiko.stamina_top_strains = attribute_number(env, taiko, "stamina_top_strains");
        }

        napi_value fruits =
            ruleset == pppp::Ruleset::RULESET_CATCH ? attribute_value(env, value, "fruits") : NULL;
        if (fruits) {
            out.fruits.star_rating = attribute_number(env, fruits, "star_rating");
            out.fruits.max_combo = attribute_integer(env, fruits, "max_combo");
        }

        napi_value mania =
            ruleset == pppp::Ruleset::RULESET_MANIA ? attribute_value(env, value, "mania") : NULL;
        if (mania) {
            out.mania.star_rating = attribute_number(env, mania, "star_rating");
            out.mania.max_combo = attribute_integer(env, mania, "max_combo");
        }

        return 0;
    }

    napi_value calculate_performance(napi_env env, napi_callback_info info) {
        size_t argc = 7;
        napi_value argv[7];
        if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok || argc < 1) {
            napi_throw_type_error(env, NULL, "expected a Beatmap");
            return NULL;
        }
        void* data = NULL;
        if (napi_unwrap(env, argv[0], &data) != napi_ok || !data) {
            napi_throw_type_error(env, NULL, "expected a Beatmap");
            return NULL;
        }
        const pppp::beatmaps::Beatmap* map = static_cast<const pppp::beatmaps::Beatmap*>(data);

        napi_value undefined = NULL;
        napi_get_undefined(env, &undefined);
        pppp::mods::Mod mods[MAX_MODS];
        size_t mod_count = 0;
        if (read_mods(env, argc > 1 ? argv[1] : undefined, mods, &mod_count) < 0) {
            return NULL;
        }
        pppp::common::ScoreInfo score;
        napi_value state = argc > 2 ? argv[2] : undefined;
        bool state_absent = false;
        if (!is_absent(env, state, &state_absent)) {
            return NULL;
        }
        if (!state_absent) {
            if (read_score(env, state, &score) < 0) {
                return NULL;
            }
        }
        int combo = 0;
        int misses = 0;
        bool has_combo = false;
        bool has_misses = false;
        if (read_optional_integer(env, argc > 3 ? argv[3] : undefined, &combo, &has_combo) < 0 ||
            read_optional_integer(env, argc > 5 ? argv[5] : undefined, &misses, &has_misses) < 0) {
            return NULL;
        }
        napi_value accuracy_value = argc > 4 ? argv[4] : undefined;
        bool accuracy_absent = false;
        if (!is_absent(env, accuracy_value, &accuracy_absent)) {
            return NULL;
        }
        const bool has_accuracy = !accuracy_absent;
        double accuracy = 0.0;
        if (has_accuracy && napi_get_value_double(env, accuracy_value, &accuracy) != napi_ok) {
            napi_throw_type_error(env, NULL, "expected an accuracy");
            return NULL;
        }
        napi_value attributes_value = argc > 6 ? argv[6] : undefined;
        bool attributes_absent = false;
        if (!is_absent(env, attributes_value, &attributes_absent)) {
            return NULL;
        }

        pppp::Performance performance(*map);
        if (!attributes_absent) {
            pppp::DifficultyAttributes provided;
            if (read_attributes(env, attributes_value, provided) < 0) {
                return NULL;
            }
            performance.attributes(provided);
        }
        performance.state(score);
        if (mod_count) {
            performance.mods(mods, mod_count);
        }
        if (has_combo) {
            performance.combo(combo);
        }
        if (has_accuracy) {
            performance.accuracy(accuracy);
        }
        if (has_misses) {
            performance.misses(misses);
        }

        const pppp::PerformanceAttributes attributes = performance.calculate();
        return performance_attributes(env, attributes);
    }

    napi_value read_version(napi_env env, napi_callback_info info) {
        char version[16];
        std::snprintf(version, sizeof(version), "%d.%d.%d", PPPP_VERSION_MAJOR, PPPP_VERSION_MINOR,
                      PPPP_VERSION_PATCH);
        napi_value value = NULL;
        napi_create_string_utf8(env, version, NAPI_AUTO_LENGTH, &value);
        return value;
    }

    napi_property_descriptor properties[] = {
        {"version", NULL, NULL, read_version, NULL, NULL, napi_default_jsproperty, NULL},
        {"fromFile", NULL, from_file, NULL, NULL, NULL, napi_default_jsproperty, NULL},
        {"calculateDifficulty", NULL, calculate_difficulty, NULL, NULL, NULL, napi_default_jsproperty, NULL},
        {"calculatePerformance", NULL, calculate_performance, NULL, NULL, NULL, napi_default_jsproperty,
         NULL},
    };

} // namespace

NAPI_MODULE_INIT() {
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}
