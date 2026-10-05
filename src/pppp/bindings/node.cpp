// Construct detached JS values using Node-API.
#include <pppp/beatmap.h>
#include <pppp/config.h>
#include <pppp/pppp.h>

#include <cstdio>
#include <cstring>
#include <new>
#include <vector>

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
        napi_set_named_property(env, object, "drainRate", number(env, value.drain_rate));
        napi_set_named_property(env, object, "circleSize", number(env, value.circle_size));
        napi_set_named_property(env, object, "overallDifficulty", number(env, value.overall_difficulty));
        napi_set_named_property(env, object, "approachRate", number(env, value.approach_rate));
        napi_set_named_property(env, object, "sliderMultiplier", number(env, value.slider_multiplier));
        napi_set_named_property(env, object, "sliderTickRate", number(env, value.slider_tick_rate));
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
        napi_set_named_property(env, object, "startTime", number(env, value.start_time));
        napi_set_named_property(env, object, "endTime", number(env, value.end_time));
        napi_set_named_property(env, object, "newCombo", boolean(env, value.new_combo));
        napi_set_named_property(env, object, "comboOffset", integer(env, value.combo_offset));
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
        napi_set_named_property(env, object, "spanIndex", integer(env, value.span_index));
        napi_set_named_property(env, object, "spanStartTime", number(env, value.span_start_time));
        napi_set_named_property(env, object, "pathProgress", number(env, value.path_progress));
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
        napi_set_named_property(env, object, "expectedLength", number(env, value.expected_length));
        napi_set_named_property(env, object, "nodeSounds", array_of_integers(env, value.node_sounds));
        napi_set_named_property(env, object, "controlPoints", array_of_points(env, value.control_points));
        napi_set_named_property(env, object, "path", array_of_points(env, value.path));
        napi_set_named_property(env, object, "cumulativeLengths",
                                array_of_numbers(env, value.cumulative_lengths));
        napi_set_named_property(env, object, "undecimatedPath", array_of_points(env, value.undecimated_path));
        napi_set_named_property(env, object, "undecimatedCumulativeLengths",
                                array_of_numbers(env, value.undecimated_cumulative_lengths));
        napi_set_named_property(env, object, "events", array_of_events(env, value.events));
        napi_set_named_property(env, object, "catchEvents", array_of_events(env, value.catch_events));
        return object;
    }

    napi_value timing_point(napi_env env, const pppp::beatmaps::TimingPoint& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "time", number(env, value.time));
        napi_set_named_property(env, object, "beatLength", number(env, value.beat_length));
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
        napi_set_named_property(env, object, "startTime", number(env, value.start_time));
        napi_set_named_property(env, object, "endTime", number(env, value.end_time));
        return object;
    }

    napi_value build_hit_objects(napi_env env, const pppp::beatmaps::Beatmap& map) {
        const size_t count = map.hit_objects.size();
        napi_value array = NULL;
        if (napi_create_array_with_length(env, count, &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < count; i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), hit_object(env, map.hit_objects[i]));
        }
        return array;
    }

    napi_value build_sliders(napi_env env, const pppp::beatmaps::Beatmap& map) {
        const size_t count = map.sliders.size();
        napi_value array = NULL;
        if (napi_create_array_with_length(env, count, &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < count; i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), slider(env, map.sliders[i]));
        }
        return array;
    }

    napi_value build_timing_points(napi_env env, const pppp::beatmaps::Beatmap& map) {
        const size_t count = map.timing_points.size();
        napi_value array = NULL;
        if (napi_create_array_with_length(env, count, &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < count; i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), timing_point(env, map.timing_points[i]));
        }
        return array;
    }

    napi_value build_breaks(napi_env env, const pppp::beatmaps::Beatmap& map) {
        const size_t count = map.breaks.size();
        napi_value array = NULL;
        if (napi_create_array_with_length(env, count, &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < count; i++) {
            napi_set_element(env, array, static_cast<uint32_t>(i), break_period(env, map.breaks[i]));
        }
        return array;
    }

    napi_value beatmap_field(napi_env env, napi_callback_info info, const char* cache,
                             napi_value (*build)(napi_env, const pppp::beatmaps::Beatmap&)) {
        napi_value receiver = NULL;
        if (napi_get_cb_info(env, info, NULL, NULL, &receiver, NULL) != napi_ok) {
            return NULL;
        }
        bool built = false;
        if (napi_has_named_property(env, receiver, cache, &built) != napi_ok) {
            return NULL;
        }
        if (built) {
            napi_value cached = NULL;
            if (napi_get_named_property(env, receiver, cache, &cached) != napi_ok) {
                return NULL;
            }
            return cached;
        }
        void* data = NULL;
        if (napi_unwrap(env, receiver, &data) != napi_ok || !data) {
            return NULL;
        }
        napi_value array = build(env, *static_cast<const pppp::Beatmap*>(data));
        if (!array) {
            return NULL;
        }
        const napi_property_descriptor hidden = {cache, NULL, NULL, NULL, NULL, array, napi_default, NULL};
        if (napi_define_properties(env, receiver, 1, &hidden) != napi_ok) {
            return NULL;
        }
        return array;
    }

    napi_value get_hit_objects(napi_env env, napi_callback_info info) {
        return beatmap_field(env, info, "_hitObjects", build_hit_objects);
    }

    napi_value get_sliders(napi_env env, napi_callback_info info) {
        return beatmap_field(env, info, "_sliders", build_sliders);
    }

    napi_value get_timing_points(napi_env env, napi_callback_info info) {
        return beatmap_field(env, info, "_timingPoints", build_timing_points);
    }

    napi_value get_breaks(napi_env env, napi_callback_info info) {
        return beatmap_field(env, info, "_breaks", build_breaks);
    }

    void release_beatmap(napi_env env, void* data, void* hint) { delete static_cast<pppp::Beatmap*>(data); }

    napi_value from_bytes(napi_env env, napi_callback_info info) {
        size_t argc = 1;
        napi_value argv[1];
        bool is_typed_array = false;
        if (napi_get_cb_info(env, info, &argc, argv, NULL, NULL) != napi_ok || argc < 1 ||
            napi_is_typedarray(env, argv[0], &is_typed_array) != napi_ok || !is_typed_array) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected a Uint8Array");
            return NULL;
        }
        napi_typedarray_type type;
        size_t length = 0;
        void* bytes = NULL;
        if (napi_get_typedarray_info(env, argv[0], &type, &length, &bytes, NULL, NULL) != napi_ok ||
            type != napi_uint8_array) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected a Uint8Array");
            return NULL;
        }

        pppp::Beatmap* map = new (std::nothrow) pppp::Beatmap;
        if (!map) {
            napi_throw_range_error(env, NULL, "cannot allocate the beatmap");
            return NULL;
        }
        const pppp::Status status = map->load_buffer(bytes, length);
        if (!status.ok()) {
            delete map;
            if (status.code() == pppp::StatusCode::ALLOCATION) {
                napi_throw_range_error(env, NULL, "cannot allocate the beatmap");
                return NULL;
            }
            napi_throw_error(env, "ERR_PPPP_PARSE", "cannot parse the beatmap");
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
        napi_set_named_property(env, object, "formatVersion", integer(env, map->format_version));
        napi_set_named_property(env, object, "mode", integer(env, map->mode));
        napi_set_named_property(env, object, "stackLeniency", number(env, map->stack_leniency));
        napi_set_named_property(env, object, "difficulty", difficulty(env, map->difficulty));

        const napi_property_descriptor fields[] = {
            {"hitObjects", NULL, NULL, get_hit_objects, NULL, NULL, napi_enumerable, NULL},
            {"sliders", NULL, NULL, get_sliders, NULL, NULL, napi_enumerable, NULL},
            {"timingPoints", NULL, NULL, get_timing_points, NULL, NULL, napi_enumerable, NULL},
            {"breaks", NULL, NULL, get_breaks, NULL, NULL, napi_enumerable, NULL}};
        if (napi_define_properties(env, object, 4, fields) != napi_ok) {
            return NULL;
        }

        return object;
    }

    napi_value osu_attributes(napi_env env, const pppp::osu::difficulty::OsuDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::OSU));
        napi_set_named_property(env, object, "starRating", number(env, value.star_rating));
        napi_set_named_property(env, object, "maxCombo", integer(env, value.max_combo));
        napi_set_named_property(env, object, "aimDifficulty", number(env, value.aim_difficulty));
        napi_set_named_property(env, object, "speedDifficulty", number(env, value.speed_difficulty));
        napi_set_named_property(env, object, "readingDifficulty", number(env, value.reading_difficulty));
        napi_set_named_property(env, object, "flashlightDifficulty",
                                number(env, value.flashlight_difficulty));
        napi_set_named_property(env, object, "sliderFactor", number(env, value.slider_factor));
        napi_set_named_property(env, object, "aimDifficultStrainCount",
                                number(env, value.aim_difficult_strain_count));
        napi_set_named_property(env, object, "speedDifficultStrainCount",
                                number(env, value.speed_difficult_strain_count));
        napi_set_named_property(env, object, "readingDifficultNoteCount",
                                number(env, value.reading_difficult_note_count));
        napi_set_named_property(env, object, "aimDifficultSliderCount",
                                number(env, value.aim_difficult_slider_count));
        napi_set_named_property(env, object, "aimTopWeightedSliderFactor",
                                number(env, value.aim_top_weighted_slider_factor));
        napi_set_named_property(env, object, "speedTopWeightedSliderFactor",
                                number(env, value.speed_top_weighted_slider_factor));
        napi_set_named_property(env, object, "speedNoteCount", number(env, value.speed_note_count));
        napi_set_named_property(env, object, "hitCircleCount", integer(env, value.hit_circle_count));
        napi_set_named_property(env, object, "sliderCount", integer(env, value.slider_count));
        napi_set_named_property(env, object, "largeTickCount", integer(env, value.large_tick_count));
        napi_set_named_property(env, object, "spinnerCount", integer(env, value.spinner_count));
        napi_set_named_property(env, object, "nestedScorePerObject",
                                number(env, value.nested_score_per_object));
        napi_set_named_property(env, object, "legacyScoreBaseMultiplier",
                                number(env, value.legacy_score_base_multiplier));
        napi_set_named_property(env, object, "maximumLegacyComboScore",
                                number(env, value.maximum_legacy_combo_score));
        return object;
    }

    napi_value taiko_attributes(napi_env env,
                                const pppp::taiko::difficulty::TaikoDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::TAIKO));
        napi_set_named_property(env, object, "starRating", number(env, value.star_rating));
        napi_set_named_property(env, object, "maxCombo", integer(env, value.max_combo));
        napi_set_named_property(env, object, "mechanicalDifficulty",
                                number(env, value.mechanical_difficulty));
        napi_set_named_property(env, object, "rhythmDifficulty", number(env, value.rhythm_difficulty));
        napi_set_named_property(env, object, "readingDifficulty", number(env, value.reading_difficulty));
        napi_set_named_property(env, object, "colourDifficulty", number(env, value.colour_difficulty));
        napi_set_named_property(env, object, "staminaDifficulty", number(env, value.stamina_difficulty));
        napi_set_named_property(env, object, "monoStaminaFactor", number(env, value.mono_stamina_factor));
        napi_set_named_property(env, object, "consistencyFactor", number(env, value.consistency_factor));
        napi_set_named_property(env, object, "staminaTopStrains", number(env, value.stamina_top_strains));
        return object;
    }

    napi_value catch_attributes(napi_env env,
                                const pppp::fruits::difficulty::CatchDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::CATCH));
        napi_set_named_property(env, object, "starRating", number(env, value.star_rating));
        napi_set_named_property(env, object, "maxCombo", integer(env, value.max_combo));
        return object;
    }

    napi_value mania_attributes(napi_env env,
                                const pppp::mania::difficulty::ManiaDifficultyAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::MANIA));
        napi_set_named_property(env, object, "starRating", number(env, value.star_rating));
        napi_set_named_property(env, object, "maxCombo", integer(env, value.max_combo));
        return object;
    }

    napi_value difficulty_attributes(napi_env env, const pppp::DifficultyAttributes& value) {
        if (value.taiko()) {
            return taiko_attributes(env, *value.taiko());
        }
        if (value.fruits()) {
            return catch_attributes(env, *value.fruits());
        }
        if (value.mania()) {
            return mania_attributes(env, *value.mania());
        }
        return osu_attributes(env, *value.osu());
    }

    bool is_absent(napi_env env, napi_value value, bool* absent) {
        napi_valuetype kind;
        if (napi_typeof(env, value, &kind) != napi_ok) {
            return false;
        }
        *absent = kind == napi_undefined || kind == napi_null;
        return true;
    }

    int read_mods(napi_env env, napi_value value, pppp::Mods& mods) {
        mods.clear();
        bool absent = false;
        if (!is_absent(env, value, &absent) || absent) {
            return absent ? 0 : -1;
        }
        napi_valuetype kind;
        if (napi_typeof(env, value, &kind) != napi_ok) {
            return -1;
        }
        if (kind == napi_number) {
            double bits = 0.0;
            napi_get_value_double(env, value, &bits);
            if (bits < 0.0 || bits > 4294967295.0 ||
                bits != static_cast<double>(static_cast<pppp_uint64>(bits))) {
                napi_throw_error(env, "ERR_PPPP_MODS", "invalid mod specification");
                return -1;
            }
            mods = pppp::Mods::from_legacy(static_cast<unsigned>(bits));
            return 0;
        }
        size_t length = 0;
        if (kind != napi_string || napi_get_value_string_utf8(env, value, NULL, 0, &length) != napi_ok) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "mods must be a string or a number");
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
        const bool parsed = mods.parse(spec).ok();
        delete[] spec;
        if (!parsed) {
            napi_throw_error(env, "ERR_PPPP_MODS", "invalid mod specification");
            return -1;
        }
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
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected a ruleset");
            return -1;
        }
        if (ruleset < 0 || ruleset > 3) {
            napi_throw_range_error(env, "ERR_OUT_OF_RANGE", "ruleset must be one of 0, 1, 2, 3");
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
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected a clock rate");
            return -1;
        }
        difficulty.clock_rate(clock_rate);
        return 0;
    }

    int configure_difficulty(napi_env env, const napi_value* argv, pppp::Difficulty& difficulty) {
        pppp::Mods mods;
        if (read_mods(env, argv[1], mods) < 0) {
            return -1;
        }
        difficulty.mods(mods);
        if (read_ruleset(env, argv[2], difficulty) < 0) {
            return -1;
        }
        return read_clock_rate(env, argv[3], difficulty);
    }

    napi_value timed_difficulty(napi_env env, const std::vector<pppp::TimedDifficultyAttributes>& timed) {
        napi_value array = NULL;
        if (napi_create_array_with_length(env, timed.size(), &array) != napi_ok) {
            return NULL;
        }
        for (size_t i = 0; i < timed.size(); i++) {
            napi_value entry = NULL;
            if (napi_create_object(env, &entry) != napi_ok) {
                return NULL;
            }
            napi_set_named_property(env, entry, "time", number(env, timed[i].time));
            napi_set_named_property(env, entry, "attributes",
                                    difficulty_attributes(env, timed[i].attributes));
            if (napi_set_element(env, array, static_cast<uint32_t>(i), entry) != napi_ok) {
                return NULL;
            }
        }
        return array;
    }

    napi_value strains(napi_env env, const pppp::Strains& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, value.ruleset()));
        napi_set_named_property(env, object, "startTime", number(env, value.start_time()));
        napi_set_named_property(env, object, "sectionLength", number(env, value.section_length()));
        if (const pppp::OsuStrains* osu = value.osu()) {
            napi_set_named_property(env, object, "aim", array_of_numbers(env, osu->aim));
            napi_set_named_property(env, object, "aimNoSliders", array_of_numbers(env, osu->aim_no_sliders));
            napi_set_named_property(env, object, "speed", array_of_numbers(env, osu->speed));
            napi_set_named_property(env, object, "reading", array_of_numbers(env, osu->reading));
            napi_set_named_property(env, object, "flashlight", array_of_numbers(env, osu->flashlight));
        } else if (const pppp::TaikoStrains* taiko = value.taiko()) {
            napi_set_named_property(env, object, "colour", array_of_numbers(env, taiko->colour));
            napi_set_named_property(env, object, "reading", array_of_numbers(env, taiko->reading));
            napi_set_named_property(env, object, "rhythm", array_of_numbers(env, taiko->rhythm));
            napi_set_named_property(env, object, "stamina", array_of_numbers(env, taiko->stamina));
            napi_set_named_property(env, object, "singleColourStamina",
                                    array_of_numbers(env, taiko->single_colour_stamina));
        } else if (const pppp::CatchStrains* fruits = value.fruits()) {
            napi_set_named_property(env, object, "movement", array_of_numbers(env, fruits->movement));
        } else if (const pppp::ManiaStrains* mania = value.mania()) {
            napi_set_named_property(env, object, "strain", array_of_numbers(env, mania->strain));
        }
        return object;
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
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::OSU));
        napi_set_named_property(env, object, "total", number(env, value.total));
        napi_set_named_property(env, object, "aim", number(env, value.aim));
        napi_set_named_property(env, object, "speed", number(env, value.speed));
        napi_set_named_property(env, object, "accuracy", number(env, value.accuracy));
        napi_set_named_property(env, object, "flashlight", number(env, value.flashlight));
        napi_set_named_property(env, object, "reading", number(env, value.reading));
        napi_set_named_property(env, object, "effectiveMissCount", number(env, value.effective_miss_count));
        napi_set_named_property(env, object, "comboBasedEstimatedMissCount",
                                number(env, value.combo_based_estimated_miss_count));
        napi_set_named_property(env, object, "scoreBasedEstimatedMissCount",
                                optional_number(env, value.score_based_estimated_miss_count));
        napi_set_named_property(env, object, "aimEstimatedSliderBreaks",
                                number(env, value.aim_estimated_slider_breaks));
        napi_set_named_property(env, object, "speedEstimatedSliderBreaks",
                                number(env, value.speed_estimated_slider_breaks));
        napi_set_named_property(env, object, "speedDeviation", optional_number(env, value.speed_deviation));
        return object;
    }

    napi_value taiko_performance(napi_env env,
                                 const pppp::taiko::difficulty::TaikoPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::TAIKO));
        napi_set_named_property(env, object, "total", number(env, value.total));
        napi_set_named_property(env, object, "difficulty", number(env, value.difficulty));
        napi_set_named_property(env, object, "accuracy", number(env, value.accuracy));
        napi_set_named_property(env, object, "estimatedUnstableRate",
                                optional_number(env, value.estimated_unstable_rate));
        return object;
    }

    napi_value catch_performance(napi_env env,
                                 const pppp::fruits::difficulty::CatchPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::CATCH));
        napi_set_named_property(env, object, "total", number(env, value.total));
        return object;
    }

    napi_value mania_performance(napi_env env,
                                 const pppp::mania::difficulty::ManiaPerformanceAttributes& value) {
        napi_value object = NULL;
        if (napi_create_object(env, &object) != napi_ok) {
            return NULL;
        }
        napi_set_named_property(env, object, "ruleset", integer(env, pppp::Ruleset::MANIA));
        napi_set_named_property(env, object, "total", number(env, value.total));
        napi_set_named_property(env, object, "difficulty", number(env, value.difficulty));
        return object;
    }

    napi_value performance_attributes(napi_env env, const pppp::PerformanceAttributes& value) {
        if (value.taiko()) {
            return taiko_performance(env, *value.taiko());
        }
        if (value.fruits()) {
            return catch_performance(env, *value.fruits());
        }
        if (value.mania()) {
            return mania_performance(env, *value.mania());
        }
        return osu_performance(env, *value.osu());
    }

    const char* const hit_result_names[] = {"none",          "miss",
                                            "meh",           "ok",
                                            "good",          "great",
                                            "perfect",       "smallTickMiss",
                                            "smallTickHit",  "largeTickMiss",
                                            "largeTickHit",  "smallBonus",
                                            "largeBonus",    "ignoreMiss",
                                            "ignoreHit",     "comboBreak",
                                            "sliderTailHit", "legacyComboIncrease"};

    PPPP_STATIC_ASSERT(hit_result_names_match, sizeof(hit_result_names) / sizeof(hit_result_names[0]) ==
                                                   pppp::common::HIT_RESULT_COUNT);

    int read_statistics(napi_env env, napi_value value, int* out) {
        bool absent = false;
        if (!is_absent(env, value, &absent)) {
            return -1;
        }
        if (absent) {
            return 0;
        }
        napi_valuetype kind;
        if (napi_typeof(env, value, &kind) != napi_ok || kind != napi_object) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "statistics must be an object of counts");
            return -1;
        }
        napi_value keys = NULL;
        uint32_t length = 0;
        if (napi_get_property_names(env, value, &keys) != napi_ok ||
            napi_get_array_length(env, keys, &length) != napi_ok) {
            return -1;
        }
        for (uint32_t i = 0; i < length; i++) {
            napi_value key = NULL;
            char name[32];
            size_t size = 0;
            if (napi_get_element(env, keys, i, &key) != napi_ok ||
                napi_get_value_string_utf8(env, key, name, sizeof(name), &size) != napi_ok) {
                return -1;
            }
            int index = -1;
            for (int result = 0; result < pppp::common::HIT_RESULT_COUNT; result++) {
                if (std::strcmp(name, hit_result_names[result]) == 0) {
                    index = result;
                    break;
                }
            }
            if (index < 0) {
                napi_throw_range_error(env, "ERR_OUT_OF_RANGE", "statistics keys must be hit results");
                return -1;
            }
            napi_value count = NULL;
            int32_t number = 0;
            if (napi_get_property(env, value, key, &count) != napi_ok ||
                napi_get_value_int32(env, count, &number) != napi_ok) {
                napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "statistics must be an object of counts");
                return -1;
            }
            out[index] = number;
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
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected an integer");
            return -1;
        }
        *out = integer_value;
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
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected difficulty attributes");
            return -1;
        }

        const int ruleset = attribute_integer(env, value, "ruleset");
        if (ruleset < 0 || ruleset > 3) {
            napi_throw_range_error(env, "ERR_OUT_OF_RANGE", "ruleset must be one of 0, 1, 2, 3");
            return -1;
        }
        pppp::OsuDifficultyAttributes osu_attributes;
        pppp::TaikoDifficultyAttributes taiko_attributes;
        pppp::CatchDifficultyAttributes fruits_attributes;
        pppp::ManiaDifficultyAttributes mania_attributes;

        napi_value osu = ruleset == pppp::Ruleset::OSU ? value : NULL;
        if (osu) {
            osu_attributes.star_rating = attribute_number(env, osu, "starRating");
            osu_attributes.max_combo = attribute_integer(env, osu, "maxCombo");
            osu_attributes.aim_difficulty = attribute_number(env, osu, "aimDifficulty");
            osu_attributes.speed_difficulty = attribute_number(env, osu, "speedDifficulty");
            osu_attributes.reading_difficulty = attribute_number(env, osu, "readingDifficulty");
            osu_attributes.flashlight_difficulty = attribute_number(env, osu, "flashlightDifficulty");
            osu_attributes.slider_factor = attribute_number(env, osu, "sliderFactor");
            osu_attributes.aim_difficult_strain_count = attribute_number(env, osu, "aimDifficultStrainCount");
            osu_attributes.speed_difficult_strain_count =
                attribute_number(env, osu, "speedDifficultStrainCount");
            osu_attributes.reading_difficult_note_count =
                attribute_number(env, osu, "readingDifficultNoteCount");
            osu_attributes.aim_difficult_slider_count = attribute_number(env, osu, "aimDifficultSliderCount");
            osu_attributes.aim_top_weighted_slider_factor =
                attribute_number(env, osu, "aimTopWeightedSliderFactor");
            osu_attributes.speed_top_weighted_slider_factor =
                attribute_number(env, osu, "speedTopWeightedSliderFactor");
            osu_attributes.speed_note_count = attribute_number(env, osu, "speedNoteCount");
            osu_attributes.hit_circle_count = attribute_integer(env, osu, "hitCircleCount");
            osu_attributes.slider_count = attribute_integer(env, osu, "sliderCount");
            osu_attributes.large_tick_count = attribute_integer(env, osu, "largeTickCount");
            osu_attributes.spinner_count = attribute_integer(env, osu, "spinnerCount");
            osu_attributes.nested_score_per_object = attribute_number(env, osu, "nestedScorePerObject");
            osu_attributes.legacy_score_base_multiplier =
                attribute_number(env, osu, "legacyScoreBaseMultiplier");
            osu_attributes.maximum_legacy_combo_score = attribute_number(env, osu, "maximumLegacyComboScore");
        }

        napi_value taiko = ruleset == pppp::Ruleset::TAIKO ? value : NULL;
        if (taiko) {
            taiko_attributes.star_rating = attribute_number(env, taiko, "starRating");
            taiko_attributes.max_combo = attribute_integer(env, taiko, "maxCombo");
            taiko_attributes.mechanical_difficulty = attribute_number(env, taiko, "mechanicalDifficulty");
            taiko_attributes.rhythm_difficulty = attribute_number(env, taiko, "rhythmDifficulty");
            taiko_attributes.reading_difficulty = attribute_number(env, taiko, "readingDifficulty");
            taiko_attributes.colour_difficulty = attribute_number(env, taiko, "colourDifficulty");
            taiko_attributes.stamina_difficulty = attribute_number(env, taiko, "staminaDifficulty");
            taiko_attributes.mono_stamina_factor = attribute_number(env, taiko, "monoStaminaFactor");
            taiko_attributes.consistency_factor = attribute_number(env, taiko, "consistencyFactor");
            taiko_attributes.stamina_top_strains = attribute_number(env, taiko, "staminaTopStrains");
        }

        napi_value fruits = ruleset == pppp::Ruleset::CATCH ? value : NULL;
        if (fruits) {
            fruits_attributes.star_rating = attribute_number(env, fruits, "starRating");
            fruits_attributes.max_combo = attribute_integer(env, fruits, "maxCombo");
        }

        napi_value mania = ruleset == pppp::Ruleset::MANIA ? value : NULL;
        if (mania) {
            mania_attributes.star_rating = attribute_number(env, mania, "starRating");
            mania_attributes.max_combo = attribute_integer(env, mania, "maxCombo");
        }

        switch (ruleset) {
        case pppp::Ruleset::TAIKO: out = taiko_attributes; break;
        case pppp::Ruleset::CATCH: out = fruits_attributes; break;
        case pppp::Ruleset::MANIA: out = mania_attributes; break;
        default: out = osu_attributes; break;
        }
        return 0;
    }

    int configure_performance(napi_env env, const napi_value* argv, pppp::Performance& performance) {
        pppp::Mods mods;
        if (read_mods(env, argv[1], mods) < 0) {
            return -1;
        }
        int max_combo = 0;
        int misses = 0;
        bool has_max_combo = false;
        bool has_misses = false;
        if (read_optional_integer(env, argv[2], &max_combo, &has_max_combo) < 0 ||
            read_optional_integer(env, argv[4], &misses, &has_misses) < 0) {
            return -1;
        }
        napi_value accuracy_value = argv[3];
        bool accuracy_absent = false;
        if (!is_absent(env, accuracy_value, &accuracy_absent)) {
            return -1;
        }
        const bool has_accuracy = !accuracy_absent;
        double accuracy = 0.0;
        if (has_accuracy && napi_get_value_double(env, accuracy_value, &accuracy) != napi_ok) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected an accuracy");
            return -1;
        }
        pppp::ScoreInfo score;
        if (read_statistics(env, argv[5], score.statistics) < 0) {
            return -1;
        }
        napi_value legacy_value = argv[6];
        bool legacy_absent = false;
        if (!is_absent(env, legacy_value, &legacy_absent)) {
            return -1;
        }
        if (!legacy_absent) {
            int64_t legacy = 0;
            if (napi_get_value_int64(env, legacy_value, &legacy) != napi_ok) {
                napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected an integer");
                return -1;
            }
            score.legacy_total_score = static_cast<pppp_int64>(legacy);
        }
        napi_value attributes_value = argv[7];
        bool attributes_absent = false;
        if (!is_absent(env, attributes_value, &attributes_absent)) {
            return -1;
        }

        if (!attributes_absent) {
            pppp::DifficultyAttributes provided;
            if (read_attributes(env, attributes_value, provided) < 0) {
                return -1;
            }
            performance.attributes(provided);
        }
        performance.score(score);
        if (!mods.empty()) {
            performance.mods(mods);
        }
        if (has_max_combo) {
            performance.combo(max_combo);
        }
        if (has_accuracy) {
            performance.accuracy(accuracy);
        }
        if (has_misses) {
            performance.misses(misses);
        }

        return 0;
    }

    struct Job {
        enum Kind { DIFFICULTY, TIMED, STRAINS, PERFORMANCE };

        Job(Kind job_kind, const pppp::Beatmap& job_map)
            : kind(job_kind),
              map(&job_map),
              beatmap(NULL),
              deferred(NULL),
              work(NULL),
              performance(job_map) {}

        Kind kind;
        const pppp::Beatmap* map;
        napi_ref beatmap;
        napi_deferred deferred;
        napi_async_work work;
        pppp::Difficulty difficulty;
        pppp::Performance performance;
        pppp::Status status;
        pppp::DifficultyAttributes difficulty_result;
        std::vector<pppp::TimedDifficultyAttributes> timed_result;
        pppp::Strains strains_result;
        pppp::PerformanceAttributes performance_result;
    };

    struct Call {
        Job::Kind kind;
        bool async;
    };

    void run(Job& job) {
        switch (job.kind) {
        case Job::DIFFICULTY: job.status = job.difficulty.calculate(*job.map, job.difficulty_result); break;
        case Job::TIMED: job.status = job.difficulty.calculate_timed(*job.map, job.timed_result); break;
        case Job::STRAINS: job.status = job.difficulty.strains(*job.map, job.strains_result); break;
        case Job::PERFORMANCE: job.status = job.performance.calculate(job.performance_result); break;
        }
    }

    napi_value result(napi_env env, const Job& job) {
        switch (job.kind) {
        case Job::DIFFICULTY: return difficulty_attributes(env, job.difficulty_result);
        case Job::TIMED: return timed_difficulty(env, job.timed_result);
        case Job::STRAINS: return strains(env, job.strains_result);
        case Job::PERFORMANCE: return performance_attributes(env, job.performance_result);
        }
        return NULL;
    }

    void release(napi_env env, Job* job) {
        if (job->work) {
            napi_delete_async_work(env, job->work);
        }
        if (job->beatmap) {
            napi_delete_reference(env, job->beatmap);
        }
        delete job;
    }

    void throw_last_error(napi_env env) {
        const napi_extended_error_info* error_info = NULL;
        napi_get_last_error_info(env, &error_info);
        bool is_pending = false;
        const char* err_message = error_info->error_message;
        napi_is_exception_pending(env, &is_pending);
        // If an exception is already pending, don't rethrow it
        if (!is_pending) {
            const char* error_message = err_message != NULL ? err_message : "empty error message";
            napi_throw_error(env, NULL, error_message);
        }
    }

    void reject_with_last_error(napi_env env, napi_deferred deferred) {
        throw_last_error(env);
        napi_value error = NULL;
        napi_get_and_clear_last_exception(env, &error);
        napi_reject_deferred(env, deferred, error);
    }

    napi_value status_error(napi_env env, const pppp::Status& status) {
        napi_value message = NULL;
        napi_value error = NULL;
        napi_create_string_utf8(env, status.message(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, NULL, message, &error);
        return error;
    }

    void execute(napi_env, void* data) { run(*static_cast<Job*>(data)); }

    void complete(napi_env env, napi_status status, void* data) {
        Job* job = static_cast<Job*>(data);
        if (status != napi_ok) {
            release(env, job);
            return;
        }
        if (!job->status.ok()) {
            napi_reject_deferred(env, job->deferred, status_error(env, job->status));
            release(env, job);
            return;
        }
        napi_value value = result(env, *job);
        if (value) {
            napi_resolve_deferred(env, job->deferred, value);
        } else {
            reject_with_last_error(env, job->deferred);
        }
        release(env, job);
    }

    napi_value queue(napi_env env, Job* job, napi_value beatmap) {
        napi_value promise = NULL;
        napi_value name = NULL;
        if (napi_create_reference(env, beatmap, 1, &job->beatmap) != napi_ok ||
            napi_create_string_utf8(env, "pppp", NAPI_AUTO_LENGTH, &name) != napi_ok ||
            napi_create_async_work(env, NULL, name, execute, complete, job, &job->work) != napi_ok ||
            napi_create_promise(env, &job->deferred, &promise) != napi_ok) {
            throw_last_error(env);
            release(env, job);
            return NULL;
        }
        if (napi_queue_async_work(env, job->work) != napi_ok) {
            reject_with_last_error(env, job->deferred);
            release(env, job);
        }
        return promise;
    }

    napi_value calculate(napi_env env, napi_callback_info info) {
        size_t argc = 8;
        napi_value argv[8];
        void* data = NULL;
        if (napi_get_cb_info(env, info, &argc, argv, NULL, &data) != napi_ok || argc < 1) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected a Beatmap");
            return NULL;
        }
        void* wrapped = NULL;
        if (napi_unwrap(env, argv[0], &wrapped) != napi_ok || !wrapped) {
            napi_throw_type_error(env, "ERR_INVALID_ARG_TYPE", "expected a Beatmap");
            return NULL;
        }
        const Call* call = static_cast<const Call*>(data);
        const pppp::Beatmap& map = *static_cast<const pppp::Beatmap*>(wrapped);

        Job local(call->kind, map);
        Job* job = call->async ? new (std::nothrow) Job(call->kind, map) : &local;
        if (!job) {
            napi_throw_error(env, NULL, "out of memory");
            return NULL;
        }
        const int configured = call->kind == Job::PERFORMANCE
                                   ? configure_performance(env, argv, job->performance)
                                   : configure_difficulty(env, argv, job->difficulty);
        if (configured < 0) {
            if (call->async) {
                delete job;
            }
            return NULL;
        }
        if (!call->async) {
            run(*job);
            if (!job->status.ok()) {
                napi_throw(env, status_error(env, job->status));
                return NULL;
            }
            return result(env, *job);
        }
        return queue(env, job, argv[0]);
    }

    const Call calls[] = {{Job::DIFFICULTY, false},  {Job::DIFFICULTY, true}, {Job::TIMED, false},
                          {Job::TIMED, true},        {Job::STRAINS, false},   {Job::STRAINS, true},
                          {Job::PERFORMANCE, false}, {Job::PERFORMANCE, true}};

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
        {"fromBytes", NULL, from_bytes, NULL, NULL, NULL, napi_default_jsproperty, NULL},
        {"calculateDifficulty", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[0])},
        {"calculateDifficultyAsync", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[1])},
        {"calculateTimedDifficulty", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[2])},
        {"calculateTimedDifficultyAsync", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[3])},
        {"calculateStrains", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[4])},
        {"calculateStrainsAsync", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[5])},
        {"calculatePerformance", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[6])},
        {"calculatePerformanceAsync", NULL, calculate, NULL, NULL, NULL, napi_default_jsproperty,
         const_cast<Call*>(&calls[7])},
    };

} // namespace

NAPI_MODULE_INIT() {
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}
