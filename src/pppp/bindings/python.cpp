// Construct detached Python values using the CPython stable ABI.
#include <pppp/beatmap.h>
#include <pppp/config.h>
#include <pppp/pppp.h>

#include "pppp/bindings/records.h"

#include <Python.h>
#include <cstring>
#include <new>
#include <structmember.h>

namespace {

    class PyRef {
    public:
        explicit PyRef(PyObject* value)
            : object(value) {}
        ~PyRef() { Py_XDECREF(object); }

        PyObject* get() const { return object; }

        PyObject* release() {
            PyObject* released = object;
            object = 0;
            return released;
        }

        void reset(PyObject* value) {
            Py_XDECREF(object);
            object = value;
        }

    private:
        PyRef(const PyRef&);
        PyRef& operator=(const PyRef&);

        PyObject* object;
    };

    class PauseGC {
    public:
        PauseGC()
            : was_enabled(PyGC_Disable()) {}
        ~PauseGC() {
            if (was_enabled) {
                PyGC_Enable();
            }
        }

    private:
        PauseGC(const PauseGC&);
        PauseGC& operator=(const PauseGC&);

        int was_enabled;
    };

    PyObject* py_integer(long long value) { return PyLong_FromLongLong(value); }

    PyObject* py_number(double value) { return PyFloat_FromDouble(value); }

    PyObject* py_boolean(bool value) { return PyBool_FromLong(value ? 1 : 0); }

    PyObject* py_optional_number(const nonstd::optional<double>& value) {
        if (!value.has_value()) {
            return Py_NewRef(Py_None);
        }
        return py_number(value.value());
    }

    // NOLINTNEXTLINE(bugprone-narrowing-conversions)
    Py_ssize_t py_count(size_t count) { return static_cast<Py_ssize_t>(count); }

    // Cast a function to `void*` to use it in a `PyType_Slot`.
    //
    // This is needed to prevent compiler warnings: the standard does not allow a conversion
    // between function and object pointers, and `-Wpedantic` reports it.
    template <class T>
    void* slot(T* function) {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
        return reinterpret_cast<void*>(function);
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    }

    // Per-module ownership of the model keeps this compatible with subinterpreters.
    enum Field {
        f_x,
        f_y,
        f_drain_rate,
        f_circle_size,
        f_overall_difficulty,
        f_approach_rate,
        f_slider_multiplier,
        f_slider_tick_rate,
        f_position,
        f_type,
        f_hitsound,
        f_start_time,
        f_end_time,
        f_new_combo,
        f_combo_offset,
        f_slider,
        f_slides,
        f_expected_length,
        f_node_sounds,
        f_control_points,
        f_path,
        f_cumulative_lengths,
        f_undecimated_path,
        f_undecimated_cumulative_lengths,
        f_events,
        f_catch_events,
        f_time,
        f_beat_length,
        f_meter,
        f_uninherited,
        f_effects,
        f_span_index,
        f_span_start_time,
        f_path_progress,
        f_format_version,
        f_mode,
        f_stack_leniency,
        f_difficulty,
        f_hit_objects,
        f_sliders,
        f_timing_points,
        f_breaks,
        f_max_combo,
        f_accuracy,
        f_star_rating,
        f_total,
        f_aim_difficulty,
        f_speed_difficulty,
        f_reading_difficulty,
        f_flashlight_difficulty,
        f_slider_factor,
        f_aim_difficult_strain_count,
        f_speed_difficult_strain_count,
        f_reading_difficult_note_count,
        f_aim_difficult_slider_count,
        f_aim_top_weighted_slider_factor,
        f_speed_top_weighted_slider_factor,
        f_speed_note_count,
        f_hit_circle_count,
        f_slider_count,
        f_large_tick_count,
        f_spinner_count,
        f_nested_score_per_object,
        f_legacy_score_base_multiplier,
        f_maximum_legacy_combo_score,
        f_mechanical_difficulty,
        f_rhythm_difficulty,
        f_colour_difficulty,
        f_stamina_difficulty,
        f_mono_stamina_factor,
        f_consistency_factor,
        f_stamina_top_strains,
        f_aim,
        f_speed,
        f_flashlight,
        f_reading,
        f_effective_miss_count,
        f_combo_based_estimated_miss_count,
        f_score_based_estimated_miss_count,
        f_aim_estimated_slider_breaks,
        f_speed_estimated_slider_breaks,
        f_speed_deviation,
        f_estimated_unstable_rate,
        f_attributes,
        f_section_length,
        f_aim_no_sliders,
        f_colour,
        f_rhythm,
        f_stamina,
        f_single_colour_stamina,
        f_movement,
        f_strain,
        field_count
    };

    const char* field_names[] = {"x",
                                 "y",
                                 "drain_rate",
                                 "circle_size",
                                 "overall_difficulty",
                                 "approach_rate",
                                 "slider_multiplier",
                                 "slider_tick_rate",
                                 "position",
                                 "type",
                                 "hitsound",
                                 "start_time",
                                 "end_time",
                                 "new_combo",
                                 "combo_offset",
                                 "slider",
                                 "slides",
                                 "expected_length",
                                 "node_sounds",
                                 "control_points",
                                 "path",
                                 "cumulative_lengths",
                                 "undecimated_path",
                                 "undecimated_cumulative_lengths",
                                 "events",
                                 "catch_events",
                                 "time",
                                 "beat_length",
                                 "meter",
                                 "uninherited",
                                 "effects",
                                 "span_index",
                                 "span_start_time",
                                 "path_progress",
                                 "format_version",
                                 "mode",
                                 "stack_leniency",
                                 "difficulty",
                                 "hit_objects",
                                 "sliders",
                                 "timing_points",
                                 "breaks",
                                 "max_combo",
                                 "accuracy",
                                 "star_rating",
                                 "total",
                                 "aim_difficulty",
                                 "speed_difficulty",
                                 "reading_difficulty",
                                 "flashlight_difficulty",
                                 "slider_factor",
                                 "aim_difficult_strain_count",
                                 "speed_difficult_strain_count",
                                 "reading_difficult_note_count",
                                 "aim_difficult_slider_count",
                                 "aim_top_weighted_slider_factor",
                                 "speed_top_weighted_slider_factor",
                                 "speed_note_count",
                                 "hit_circle_count",
                                 "slider_count",
                                 "large_tick_count",
                                 "spinner_count",
                                 "nested_score_per_object",
                                 "legacy_score_base_multiplier",
                                 "maximum_legacy_combo_score",
                                 "mechanical_difficulty",
                                 "rhythm_difficulty",
                                 "colour_difficulty",
                                 "stamina_difficulty",
                                 "mono_stamina_factor",
                                 "consistency_factor",
                                 "stamina_top_strains",
                                 "aim",
                                 "speed",
                                 "flashlight",
                                 "reading",
                                 "effective_miss_count",
                                 "combo_based_estimated_miss_count",
                                 "score_based_estimated_miss_count",
                                 "aim_estimated_slider_breaks",
                                 "speed_estimated_slider_breaks",
                                 "speed_deviation",
                                 "estimated_unstable_rate",
                                 "attributes",
                                 "section_length",
                                 "aim_no_sliders",
                                 "colour",
                                 "rhythm",
                                 "stamina",
                                 "single_colour_stamina",
                                 "movement",
                                 "strain"};

    PPPP_STATIC_ASSERT(field_names_match_field_count,
                       sizeof(field_names) / sizeof(field_names[0]) == field_count);

    // The record types the extension itself builds. `_model.py` declares the same names as classes.
    enum PythonType {
        t_vector2,
        t_beatmap_difficulty,
        t_hit_object,
        t_slider,
        t_timing_point,
        t_break_period,
        t_slider_event,
        t_osu_difficulty_attributes,
        t_taiko_difficulty_attributes,
        t_catch_difficulty_attributes,
        t_mania_difficulty_attributes,
        t_osu_performance_attributes,
        t_taiko_performance_attributes,
        t_catch_performance_attributes,
        t_mania_performance_attributes,
        t_timed_difficulty_attributes,
        t_osu_strains,
        t_taiko_strains,
        t_catch_strains,
        t_mania_strains,
        type_count
    };

    const char* type_names[] = {"Vector2",
                                "BeatmapDifficulty",
                                "HitObject",
                                "Slider",
                                "TimingPoint",
                                "BreakPeriod",
                                "SliderEvent",
                                "OsuDifficultyAttributes",
                                "TaikoDifficultyAttributes",
                                "CatchDifficultyAttributes",
                                "ManiaDifficultyAttributes",
                                "OsuPerformanceAttributes",
                                "TaikoPerformanceAttributes",
                                "CatchPerformanceAttributes",
                                "ManiaPerformanceAttributes",
                                "TimedDifficultyAttributes",
                                "OsuStrains",
                                "TaikoStrains",
                                "CatchStrains",
                                "ManiaStrains"};

    PPPP_STATIC_ASSERT(type_names_match_type_count, sizeof(type_names) / sizeof(type_names[0]) == type_count);

    const char* qualified_names[] = {"pppp._model.Vector2",
                                     "pppp._model.BeatmapDifficulty",
                                     "pppp._model.HitObject",
                                     "pppp._model.Slider",
                                     "pppp._model.TimingPoint",
                                     "pppp._model.BreakPeriod",
                                     "pppp._model.SliderEvent",
                                     "pppp._model.OsuDifficultyAttributes",
                                     "pppp._model.TaikoDifficultyAttributes",
                                     "pppp._model.CatchDifficultyAttributes",
                                     "pppp._model.ManiaDifficultyAttributes",
                                     "pppp._model.OsuPerformanceAttributes",
                                     "pppp._model.TaikoPerformanceAttributes",
                                     "pppp._model.CatchPerformanceAttributes",
                                     "pppp._model.ManiaPerformanceAttributes",
                                     "pppp._model.TimedDifficultyAttributes",
                                     "pppp._model.OsuStrains",
                                     "pppp._model.TaikoStrains",
                                     "pppp._model.CatchStrains",
                                     "pppp._model.ManiaStrains"};

    struct State {
        PyObject* types[type_count];
        Py_ssize_t slots[type_count][field_count];
        Py_ssize_t sizes[type_count];
        PyObject* beatmap_type;
        PyObject* error;
        PyObject* mods_error;
        PyObject* parse_error;
    };

    struct BeatmapObject {
        PyObject_HEAD pppp::Beatmap* map;
        State* state;
        PyObject* difficulty;
        PyObject* hit_objects;
        PyObject* sliders;
        PyObject* timing_points;
        PyObject* breaks;
    };

    int index_of_field(const char* name) {
        for (int field = 0; field < field_count; field++) {
            if (std::strcmp(name, field_names[field]) == 0) {
                return field;
            }
        }
        return -1;
    }

    int index_of_type(const char* name) {
        for (int kind = 0; kind < type_count; kind++) {
            if (std::strcmp(name, type_names[kind]) == 0) {
                return kind;
            }
        }
        return -1;
    }

    struct FieldValue {
        int field;
        PyObject* value;
    };

    class Builder {
    public:
        explicit Builder(State* module_state)
            : state(module_state) {}

        PyObject* point(float x, float y) const {
            FieldValue values[] = {{f_x, py_number(x)}, {f_y, py_number(y)}};
            return build(t_vector2, values, 2);
        }

        PyObject* difficulty(const pppp::beatmaps::BeatmapDifficulty& value) const {
            FieldValue values[] = {{f_drain_rate, py_number(value.drain_rate)},
                                   {f_circle_size, py_number(value.circle_size)},
                                   {f_overall_difficulty, py_number(value.overall_difficulty)},
                                   {f_approach_rate, py_number(value.approach_rate)},
                                   {f_slider_multiplier, py_number(value.slider_multiplier)},
                                   {f_slider_tick_rate, py_number(value.slider_tick_rate)}};
            return build(t_beatmap_difficulty, values, 6);
        }

        PyObject* hit_object(const pppp::beatmaps::HitObject& value) const {
            FieldValue values[] = {{f_position, point(value.position.x, value.position.y)},
                                   {f_type, py_integer(value.type)},
                                   {f_hitsound, py_integer(value.hitsound)},
                                   {f_start_time, py_number(value.start_time)},
                                   {f_end_time, py_number(value.end_time)},
                                   {f_new_combo, py_boolean(value.new_combo)},
                                   {f_combo_offset, py_integer(value.combo_offset)},
                                   {f_slider, py_integer(value.slider)}};
            return build(t_hit_object, values, 8);
        }

        PyObject* slider_event(const pppp::beatmaps::SliderEventDescriptor& value) const {
            FieldValue values[] = {{f_type, py_integer(value.type)},
                                   {f_time, py_number(value.time)},
                                   {f_span_index, py_integer(value.span_index)},
                                   {f_span_start_time, py_number(value.span_start_time)},
                                   {f_path_progress, py_number(value.path_progress)},
                                   {f_position, point(value.position.x, value.position.y)}};
            return build(t_slider_event, values, 6);
        }

        PyObject* slider(const pppp::beatmaps::Slider& value) const {
            FieldValue values[] = {
                {f_slides, py_integer(value.slides)},
                {f_expected_length, py_number(value.expected_length)},
                {f_node_sounds, integers(value.node_sounds)},
                {f_control_points, points(value.control_points)},
                {f_path, points(value.path)},
                {f_cumulative_lengths, numbers(value.cumulative_lengths)},
                {f_undecimated_path, points(value.undecimated_path)},
                {f_undecimated_cumulative_lengths, numbers(value.undecimated_cumulative_lengths)},
                {f_events, events(value.events)},
                {f_catch_events, events(value.catch_events)}};
            return build(t_slider, values, 10);
        }

        PyObject* timing_point(const pppp::beatmaps::TimingPoint& value) const {
            FieldValue values[] = {{f_time, py_number(value.time)},
                                   {f_beat_length, py_number(value.beat_length)},
                                   {f_meter, py_integer(value.meter)},
                                   {f_uninherited, py_boolean(value.uninherited)},
                                   {f_effects, py_integer(value.effects)}};
            return build(t_timing_point, values, 5);
        }

        PyObject* break_period(const pppp::beatmaps::timing::BreakPeriod& value) const {
            FieldValue values[] = {{f_start_time, py_number(value.start_time)},
                                   {f_end_time, py_number(value.end_time)}};
            return build(t_break_period, values, 2);
        }

        PyObject*
        osu_difficulty_attributes(const pppp::osu::difficulty::OsuDifficultyAttributes& value) const {
            FieldValue values[] = {
                {f_star_rating, py_number(value.star_rating)},
                {f_max_combo, py_integer(value.max_combo)},
                {f_aim_difficulty, py_number(value.aim_difficulty)},
                {f_speed_difficulty, py_number(value.speed_difficulty)},
                {f_reading_difficulty, py_number(value.reading_difficulty)},
                {f_flashlight_difficulty, py_number(value.flashlight_difficulty)},
                {f_slider_factor, py_number(value.slider_factor)},
                {f_aim_difficult_strain_count, py_number(value.aim_difficult_strain_count)},
                {f_speed_difficult_strain_count, py_number(value.speed_difficult_strain_count)},
                {f_reading_difficult_note_count, py_number(value.reading_difficult_note_count)},
                {f_aim_difficult_slider_count, py_number(value.aim_difficult_slider_count)},
                {f_aim_top_weighted_slider_factor, py_number(value.aim_top_weighted_slider_factor)},
                {f_speed_top_weighted_slider_factor, py_number(value.speed_top_weighted_slider_factor)},
                {f_speed_note_count, py_number(value.speed_note_count)},
                {f_hit_circle_count, py_integer(value.hit_circle_count)},
                {f_slider_count, py_integer(value.slider_count)},
                {f_large_tick_count, py_integer(value.large_tick_count)},
                {f_spinner_count, py_integer(value.spinner_count)},
                {f_nested_score_per_object, py_number(value.nested_score_per_object)},
                {f_legacy_score_base_multiplier, py_number(value.legacy_score_base_multiplier)},
                {f_maximum_legacy_combo_score, py_number(value.maximum_legacy_combo_score)}};
            return build(t_osu_difficulty_attributes, values, 21);
        }

        PyObject*
        taiko_difficulty_attributes(const pppp::taiko::difficulty::TaikoDifficultyAttributes& value) const {
            FieldValue values[] = {{f_star_rating, py_number(value.star_rating)},
                                   {f_max_combo, py_integer(value.max_combo)},
                                   {f_mechanical_difficulty, py_number(value.mechanical_difficulty)},
                                   {f_rhythm_difficulty, py_number(value.rhythm_difficulty)},
                                   {f_reading_difficulty, py_number(value.reading_difficulty)},
                                   {f_colour_difficulty, py_number(value.colour_difficulty)},
                                   {f_stamina_difficulty, py_number(value.stamina_difficulty)},
                                   {f_mono_stamina_factor, py_number(value.mono_stamina_factor)},
                                   {f_consistency_factor, py_number(value.consistency_factor)},
                                   {f_stamina_top_strains, py_number(value.stamina_top_strains)}};
            return build(t_taiko_difficulty_attributes, values, 10);
        }

        PyObject*
        catch_difficulty_attributes(const pppp::fruits::difficulty::CatchDifficultyAttributes& value) const {
            FieldValue values[] = {{f_star_rating, py_number(value.star_rating)},
                                   {f_max_combo, py_integer(value.max_combo)}};
            return build(t_catch_difficulty_attributes, values, 2);
        }

        PyObject*
        mania_difficulty_attributes(const pppp::mania::difficulty::ManiaDifficultyAttributes& value) const {
            FieldValue values[] = {{f_star_rating, py_number(value.star_rating)},
                                   {f_max_combo, py_integer(value.max_combo)}};
            return build(t_mania_difficulty_attributes, values, 2);
        }

        PyObject* difficulty_attributes(const pppp::DifficultyAttributes& value) const {
            if (value.taiko()) {
                return taiko_difficulty_attributes(*value.taiko());
            }
            if (value.fruits()) {
                return catch_difficulty_attributes(*value.fruits());
            }
            if (value.mania()) {
                return mania_difficulty_attributes(*value.mania());
            }
            return osu_difficulty_attributes(*value.osu());
        }

        PyObject*
        osu_performance_attributes(const pppp::osu::difficulty::OsuPerformanceAttributes& value) const {
            FieldValue values[] = {
                {f_total, py_number(value.total)},
                {f_aim, py_number(value.aim)},
                {f_speed, py_number(value.speed)},
                {f_accuracy, py_number(value.accuracy)},
                {f_flashlight, py_number(value.flashlight)},
                {f_reading, py_number(value.reading)},
                {f_effective_miss_count, py_number(value.effective_miss_count)},
                {f_combo_based_estimated_miss_count, py_number(value.combo_based_estimated_miss_count)},
                {f_score_based_estimated_miss_count,
                 py_optional_number(value.score_based_estimated_miss_count)},
                {f_aim_estimated_slider_breaks, py_number(value.aim_estimated_slider_breaks)},
                {f_speed_estimated_slider_breaks, py_number(value.speed_estimated_slider_breaks)},
                {f_speed_deviation, py_optional_number(value.speed_deviation)}};
            return build(t_osu_performance_attributes, values, 12);
        }

        PyObject*
        taiko_performance_attributes(const pppp::taiko::difficulty::TaikoPerformanceAttributes& value) const {
            FieldValue values[] = {
                {f_total, py_number(value.total)},
                {f_difficulty, py_number(value.difficulty)},
                {f_accuracy, py_number(value.accuracy)},
                {f_estimated_unstable_rate, py_optional_number(value.estimated_unstable_rate)}};
            return build(t_taiko_performance_attributes, values, 4);
        }

        PyObject* catch_performance_attributes(
            const pppp::fruits::difficulty::CatchPerformanceAttributes& value) const {
            FieldValue values[] = {{f_total, py_number(value.total)}};
            return build(t_catch_performance_attributes, values, 1);
        }

        PyObject*
        mania_performance_attributes(const pppp::mania::difficulty::ManiaPerformanceAttributes& value) const {
            FieldValue values[] = {{f_total, py_number(value.total)},
                                   {f_difficulty, py_number(value.difficulty)}};
            return build(t_mania_performance_attributes, values, 2);
        }

        PyObject* timed_difficulty_attributes(const pppp::TimedDifficultyAttributes& value) const {
            FieldValue values[] = {{f_time, py_number(value.time)},
                                   {f_attributes, difficulty_attributes(value.attributes)}};
            return build(t_timed_difficulty_attributes, values, 2);
        }

        PyObject* timed_difficulty(const std::vector<pppp::TimedDifficultyAttributes>& values) const {
            return list_of(values, &Builder::timed_difficulty_attributes);
        }

        PyObject* strains(const pppp::Strains& value) const {
            if (const pppp::TaikoStrains* taiko = value.taiko()) {
                FieldValue values[] = {{f_start_time, py_number(taiko->start_time)},
                                       {f_section_length, py_number(taiko->section_length)},
                                       {f_colour, numbers(taiko->colour)},
                                       {f_reading, numbers(taiko->reading)},
                                       {f_rhythm, numbers(taiko->rhythm)},
                                       {f_stamina, numbers(taiko->stamina)},
                                       {f_single_colour_stamina, numbers(taiko->single_colour_stamina)}};
                return build(t_taiko_strains, values, 7);
            }
            if (const pppp::CatchStrains* fruits = value.fruits()) {
                FieldValue values[] = {{f_start_time, py_number(fruits->start_time)},
                                       {f_section_length, py_number(fruits->section_length)},
                                       {f_movement, numbers(fruits->movement)}};
                return build(t_catch_strains, values, 3);
            }
            if (const pppp::ManiaStrains* mania = value.mania()) {
                FieldValue values[] = {{f_start_time, py_number(mania->start_time)},
                                       {f_section_length, py_number(mania->section_length)},
                                       {f_strain, numbers(mania->strain)}};
                return build(t_mania_strains, values, 3);
            }
            const pppp::OsuStrains& osu = *value.osu();
            FieldValue values[] = {{f_start_time, py_number(osu.start_time)},
                                   {f_section_length, py_number(osu.section_length)},
                                   {f_aim, numbers(osu.aim)},
                                   {f_aim_no_sliders, numbers(osu.aim_no_sliders)},
                                   {f_speed, numbers(osu.speed)},
                                   {f_reading, numbers(osu.reading)},
                                   {f_flashlight, numbers(osu.flashlight)}};
            return build(t_osu_strains, values, 7);
        }

        PyObject* performance_attributes(const pppp::PerformanceAttributes& value) const {
            if (value.taiko()) {
                return taiko_performance_attributes(*value.taiko());
            }
            if (value.fruits()) {
                return catch_performance_attributes(*value.fruits());
            }
            if (value.mania()) {
                return mania_performance_attributes(*value.mania());
            }
            return osu_performance_attributes(*value.osu());
        }

        template <class T>
        PyObject* list_of(const std::vector<T>& values, PyObject* (Builder::*item)(const T&) const) const {
            PyObject* list = PyList_New(py_count(values.size()));
            if (!list) {
                return NULL;
            }
            for (size_t i = 0; i < values.size(); i++) {
                PyObject* element = (this->*item)(values[i]);
                if (!element || PyList_SetItem(list, py_count(i), element) < 0) {
                    Py_DECREF(list);
                    return NULL;
                }
            }
            return list;
        }

        PyObject* hit_objects(const std::vector<pppp::beatmaps::HitObject>& values) const {
            return list_of(values, &Builder::hit_object);
        }

        PyObject* sliders(const std::vector<pppp::beatmaps::Slider>& values) const {
            return list_of(values, &Builder::slider);
        }

        PyObject* timing_points(const std::vector<pppp::beatmaps::TimingPoint>& values) const {
            return list_of(values, &Builder::timing_point);
        }

        PyObject* breaks(const std::vector<pppp::beatmaps::timing::BreakPeriod>& values) const {
            return list_of(values, &Builder::break_period);
        }

    private:
        PyObject* build(int kind, FieldValue* values, size_t count) const {
            PyObject* object =
                allocate_record(reinterpret_cast<PyTypeObject*>(state->types[kind]), state->sizes[kind]);
            if (!object) {
                release(values, count, 0);
                return NULL;
            }
            for (size_t i = 0; i < count; i++) {
                const Py_ssize_t index = state->slots[kind][values[i].field];
                if (!values[i].value || index < 0) {
                    if (index < 0 && !PyErr_Occurred()) {
                        PyErr_Format(PyExc_TypeError, "%s has no %s field", type_names[kind],
                                     field_names[values[i].field]);
                    }
                    release(values, count, i);
                    Py_DECREF(object);
                    return NULL;
                }
                record_fields(object)[index] = values[i].value;
            }
            return object;
        }

        static void release(FieldValue* values, size_t count, size_t first) {
            for (size_t i = first; i < count; i++) {
                Py_XDECREF(values[i].value);
            }
        }

        PyObject* points(const std::vector<pppp::utils::Vector2>& values) const {
            PyObject* list = PyList_New(py_count(values.size()));
            if (!list) {
                return NULL;
            }
            for (size_t i = 0; i < values.size(); i++) {
                PyObject* element = point(values[i].x, values[i].y);
                if (!element || PyList_SetItem(list, py_count(i), element) < 0) {
                    Py_DECREF(list);
                    return NULL;
                }
            }
            return list;
        }

        PyObject* events(const std::vector<pppp::beatmaps::SliderEventDescriptor>& values) const {
            return list_of(values, &Builder::slider_event);
        }

        PyObject* integers(const std::vector<unsigned>& values) const {
            PyObject* list = PyList_New(py_count(values.size()));
            if (!list) {
                return NULL;
            }
            for (size_t i = 0; i < values.size(); i++) {
                PyObject* element = py_integer(values[i]);
                if (!element || PyList_SetItem(list, py_count(i), element) < 0) {
                    Py_DECREF(list);
                    return NULL;
                }
            }
            return list;
        }

        PyObject* numbers(const std::vector<double>& values) const {
            PyObject* list = PyList_New(py_count(values.size()));
            if (!list) {
                return NULL;
            }
            for (size_t i = 0; i < values.size(); i++) {
                PyObject* element = py_number(values[i]);
                if (!element || PyList_SetItem(list, py_count(i), element) < 0) {
                    Py_DECREF(list);
                    return NULL;
                }
            }
            return list;
        }

        State* state;
    };

    PyObject* make_record(PyObject* module, PyObject* args) {
        const char* name;
        PyObject* declared;
        if (!PyArg_ParseTuple(args, "sO:_record", &name, &declared)) {
            return NULL;
        }
        if (!PyTuple_Check(declared)) {
            PyErr_SetString(PyExc_TypeError, "fields must be a tuple");
            return NULL;
        }
        const int kind = index_of_type(name);
        char qualified[128];
        if (kind < 0) {
            const int written = PyOS_snprintf(qualified, sizeof(qualified), "pppp._model.%s", name);
            if (written < 0 || static_cast<size_t>(written) >= sizeof(qualified)) {
                PyErr_SetString(PyExc_TypeError, "record name is too long");
                return NULL;
            }
        }
        State* state = static_cast<State*>(PyModule_GetState(module));
        if (kind >= 0 && state->types[kind]) {
            PyErr_SetString(PyExc_TypeError, "record already defined");
            return NULL;
        }
        const Py_ssize_t count = PyTuple_Size(declared);
        if (count > field_count) {
            PyErr_SetString(PyExc_TypeError, "too many record fields");
            return NULL;
        }

        PyMemberDef members[field_count + 1];
        for (Py_ssize_t i = 0; i < count; i++) {
            PyObject* field = PyTuple_GetItem(declared, i);
            if (!PyUnicode_Check(field)) {
                PyErr_SetString(PyExc_TypeError, "field names must be strings");
                return NULL;
            }
            int index = -1;
            for (int candidate = 0; candidate < field_count; candidate++) {
                const int equal = PyUnicode_CompareWithASCIIString(field, field_names[candidate]);
                if (equal == -1 && PyErr_Occurred()) {
                    return NULL;
                }
                if (equal == 0) {
                    index = candidate;
                    break;
                }
            }
            if (index < 0) {
                PyErr_SetString(PyExc_TypeError, "unknown record field");
                return NULL;
            }
            for (Py_ssize_t j = 0; j < i; j++) {
                if (std::strcmp(members[j].name, field_names[index]) == 0) {
                    PyErr_SetString(PyExc_TypeError, "duplicate record field");
                    return NULL;
                }
            }
            members[i].name = field_names[index];
            members[i].type = T_OBJECT_EX;
            members[i].offset = static_cast<Py_ssize_t>(sizeof(RecordObject) + i * sizeof(PyObject*));
            members[i].flags = READONLY;
            members[i].doc = NULL;
        }
        members[count].name = NULL;
        members[count].type = 0;
        members[count].offset = 0;
        members[count].flags = 0;
        members[count].doc = NULL;

        PyType_Slot type_slots[] = {{Py_tp_new, slot(record_new)},
                                    {Py_tp_repr, slot(record_repr)},
                                    {Py_tp_richcompare, slot(record_equal)},
                                    {Py_tp_hash, slot(PyObject_HashNotImplemented)},
                                    {Py_tp_methods, record_methods},
                                    {Py_tp_dealloc, slot(record_dealloc)},
                                    {Py_tp_traverse, slot(record_traverse)},
                                    {Py_tp_clear, slot(record_clear)},
                                    {Py_tp_members, members},
                                    {0, NULL}};
        // CPython copies the definitions. Their names refer to static strings.
        const char* spec_name = kind < 0 ? qualified : qualified_names[kind];
        PyType_Spec spec = {spec_name, static_cast<int>(sizeof(RecordObject) + count * sizeof(PyObject*)), 0,
                            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, type_slots};
        PyObject* type = PyType_FromModuleAndSpec(module, &spec, NULL);
        if (!type) {
            return NULL;
        }

        if (kind >= 0) {
            state->types[kind] = type;
            state->sizes[kind] = count;
            for (int field = 0; field < field_count; field++) {
                state->slots[kind][field] = -1;
            }
            for (Py_ssize_t i = 0; i < count; i++) {
                state->slots[kind][index_of_field(members[i].name)] = i;
            }
        }
        return Py_NewRef(type);
    }

    PyObject* restore_record(PyObject* module, PyObject* type) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        for (int kind = 0; kind < type_count; kind++) {
            if (state->types[kind] == type) {
                return allocate_record(reinterpret_cast<PyTypeObject*>(type), state->sizes[kind]);
            }
        }
        const PyMemberDef* members = record_members(reinterpret_cast<PyTypeObject*>(type));
        if (members) {
            return allocate_record(reinterpret_cast<PyTypeObject*>(type), record_member_count(members));
        }
        PyErr_SetString(PyExc_TypeError, "expected a pppp record type");
        return NULL;
    }

    void beatmap_dealloc(PyObject* object) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        PyObject_GC_UnTrack(object);
        Py_CLEAR(beatmap->difficulty);
        Py_CLEAR(beatmap->hit_objects);
        Py_CLEAR(beatmap->sliders);
        Py_CLEAR(beatmap->timing_points);
        Py_CLEAR(beatmap->breaks);
        delete beatmap->map;
        const PyTypeObject* type = Py_TYPE(object);
        PyObject_GC_Del(object);
        Py_DECREF(type);
    }

    int beatmap_traverse(PyObject* object, visitproc visit, void* arg) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        Py_VISIT(Py_TYPE(object));
        Py_VISIT(beatmap->difficulty);
        Py_VISIT(beatmap->hit_objects);
        Py_VISIT(beatmap->sliders);
        Py_VISIT(beatmap->timing_points);
        Py_VISIT(beatmap->breaks);
        return 0;
    }

    int beatmap_clear(PyObject* object) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        Py_CLEAR(beatmap->difficulty);
        Py_CLEAR(beatmap->hit_objects);
        Py_CLEAR(beatmap->sliders);
        Py_CLEAR(beatmap->timing_points);
        Py_CLEAR(beatmap->breaks);
        return 0;
    }

    PyObject* beatmap_get_difficulty(PyObject* object, void*) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        if (!beatmap->map) {
            PyErr_SetString(PyExc_AttributeError, "beatmap is not loaded");
            return NULL;
        }
        if (!beatmap->difficulty) {
            const PauseGC pause;
            const Builder builder(beatmap->state);
            beatmap->difficulty = builder.difficulty(beatmap->map->difficulty);
            if (!beatmap->difficulty) {
                return NULL;
            }
        }
        return Py_NewRef(beatmap->difficulty);
    }

    PyObject* beatmap_get_hit_objects(PyObject* object, void*) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        if (!beatmap->map) {
            PyErr_SetString(PyExc_AttributeError, "beatmap is not loaded");
            return NULL;
        }
        if (!beatmap->hit_objects) {
            const PauseGC pause;
            const Builder builder(beatmap->state);
            beatmap->hit_objects = builder.hit_objects(beatmap->map->hit_objects);
            if (!beatmap->hit_objects) {
                return NULL;
            }
        }
        return Py_NewRef(beatmap->hit_objects);
    }

    PyObject* beatmap_get_sliders(PyObject* object, void*) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        if (!beatmap->map) {
            PyErr_SetString(PyExc_AttributeError, "beatmap is not loaded");
            return NULL;
        }
        if (!beatmap->sliders) {
            const PauseGC pause;
            const Builder builder(beatmap->state);
            beatmap->sliders = builder.sliders(beatmap->map->sliders);
            if (!beatmap->sliders) {
                return NULL;
            }
        }
        return Py_NewRef(beatmap->sliders);
    }

    PyObject* beatmap_get_timing_points(PyObject* object, void*) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        if (!beatmap->map) {
            PyErr_SetString(PyExc_AttributeError, "beatmap is not loaded");
            return NULL;
        }
        if (!beatmap->timing_points) {
            const PauseGC pause;
            const Builder builder(beatmap->state);
            beatmap->timing_points = builder.timing_points(beatmap->map->timing_points);
            if (!beatmap->timing_points) {
                return NULL;
            }
        }
        return Py_NewRef(beatmap->timing_points);
    }

    PyObject* beatmap_get_breaks(PyObject* object, void*) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        if (!beatmap->map) {
            PyErr_SetString(PyExc_AttributeError, "beatmap is not loaded");
            return NULL;
        }
        if (!beatmap->breaks) {
            const PauseGC pause;
            const Builder builder(beatmap->state);
            beatmap->breaks = builder.breaks(beatmap->map->breaks);
            if (!beatmap->breaks) {
                return NULL;
            }
        }
        return Py_NewRef(beatmap->breaks);
    }

    PyObject* beatmap_get_format_version(PyObject* object, void*) {
        return py_integer(reinterpret_cast<BeatmapObject*>(object)->map->format_version);
    }

    PyObject* beatmap_get_mode(PyObject* object, void*) {
        return py_integer(reinterpret_cast<BeatmapObject*>(object)->map->mode);
    }

    PyObject* beatmap_get_stack_leniency(PyObject* object, void*) {
        return py_number(reinterpret_cast<BeatmapObject*>(object)->map->stack_leniency);
    }

    PyGetSetDef beatmap_getset[] = {{"format_version", beatmap_get_format_version, NULL, NULL, NULL},
                                    {"mode", beatmap_get_mode, NULL, NULL, NULL},
                                    {"stack_leniency", beatmap_get_stack_leniency, NULL, NULL, NULL},
                                    {"difficulty", beatmap_get_difficulty, NULL, NULL, NULL},
                                    {"hit_objects", beatmap_get_hit_objects, NULL, NULL, NULL},
                                    {"sliders", beatmap_get_sliders, NULL, NULL, NULL},
                                    {"timing_points", beatmap_get_timing_points, NULL, NULL, NULL},
                                    {"breaks", beatmap_get_breaks, NULL, NULL, NULL},
                                    {NULL, NULL, NULL, NULL, NULL}};

    PyObject* beatmap_from_bytes(PyObject* type, PyObject* argument) {
        State* state = static_cast<State*>(PyType_GetModuleState(reinterpret_cast<PyTypeObject*>(type)));
        if (!state) {
            return NULL;
        }
        PyRef data(PyObject_Bytes(argument));
        if (!data.get()) {
            return NULL;
        }
        char* bytes = NULL;
        Py_ssize_t size = 0;
        if (PyBytes_AsStringAndSize(data.get(), &bytes, &size) < 0) {
            return NULL;
        }

        pppp::Beatmap* map = new (std::nothrow) pppp::Beatmap;
        if (!map) {
            return PyErr_NoMemory();
        }
        pppp::Status status;
        PyThreadState* thread = PyEval_SaveThread();
        status = map->load_buffer(bytes, static_cast<size_t>(size));
        PyEval_RestoreThread(thread);
        if (!status.ok()) {
            delete map;
            if (status.code() == pppp::StatusCode::ALLOCATION) {
                return PyErr_NoMemory();
            }
            PyErr_SetString(state->parse_error, "cannot parse the beatmap");
            return NULL;
        }

        PyObject* object = PyType_GenericAlloc(reinterpret_cast<PyTypeObject*>(state->beatmap_type), 0);
        if (!object) {
            delete map;
            return NULL;
        }
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        beatmap->map = map;
        beatmap->state = state;
        return object;
    }

    PyMethodDef beatmap_methods[] = {
        {"from_bytes", beatmap_from_bytes, METH_CLASS | METH_O,
         "Parse a `Beatmap` by providing the content of a `.osu` file as a slice of bytes."},
        {NULL, NULL, 0, NULL}};

    int read_mods(State* state, PyObject* object, pppp::Mods& mods) {
        mods.clear();
        if (object == Py_None) {
            return 0;
        }
        if (PyLong_Check(object) && !PyBool_Check(object)) {
            const unsigned long bits = PyLong_AsUnsignedLong(object);
            if (bits == static_cast<unsigned long>(-1) && PyErr_Occurred()) {
                PyErr_Clear();
                PyErr_SetString(state->mods_error, "invalid mod specification");
                return -1;
            }
            if (bits > 0xffffffffUL) {
                PyErr_SetString(state->mods_error, "invalid mod specification");
                return -1;
            }
            mods = pppp::Mods::from_legacy(static_cast<unsigned>(bits));
            return 0;
        }
        if (!PyUnicode_Check(object)) {
            PyErr_SetString(PyExc_TypeError, "mods must be a str or an int");
            return -1;
        }
        PyRef encoded(PyUnicode_AsUTF8String(object));
        if (!encoded.get()) {
            return -1;
        }
        char* spec = NULL;
        Py_ssize_t size = 0;
        if (PyBytes_AsStringAndSize(encoded.get(), &spec, &size) < 0) {
            return -1;
        }
        if (!mods.parse(spec).ok()) {
            PyErr_SetString(state->mods_error, "invalid mod specification");
            return -1;
        }
        return 0;
    }

    int read_ruleset(PyObject* object, pppp::Difficulty& difficulty) {
        if (object == Py_None) {
            return 0;
        }
        const long value = PyLong_AsLong(object);
        if (value == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (value < 0 || value > 3) {
            PyErr_SetString(PyExc_ValueError, "ruleset must be one of 0, 1, 2, 3");
            return -1;
        }
        difficulty.ruleset(static_cast<pppp::Ruleset::Value>(value));
        return 0;
    }

    const BeatmapObject* read_beatmap(State* state, PyObject* object) {
        if (!PyObject_TypeCheck(object, reinterpret_cast<PyTypeObject*>(state->beatmap_type))) {
            PyErr_SetString(PyExc_TypeError, "expected a Beatmap");
            return NULL;
        }
        return reinterpret_cast<const BeatmapObject*>(object);
    }

    int read_classic(PyObject* object, pppp::Mods& mods) {
        const int truth = PyObject_IsTrue(object);
        if (truth < 0) {
            return -1;
        }
        if (truth) {
            mods.add_classic();
        }
        return 0;
    }

    const BeatmapObject* read_difficulty(State* state, PyObject* args, const char* format,
                                         pppp::Difficulty& difficulty) {
        PyObject* beatmap_object;
        PyObject* mods_object;
        PyObject* ruleset_object;
        PyObject* clock_rate_object;
        PyObject* classic_object;
        if (!PyArg_ParseTuple(args, format, &beatmap_object, &mods_object, &ruleset_object,
                              &clock_rate_object, &classic_object)) {
            return NULL;
        }
        const BeatmapObject* beatmap = read_beatmap(state, beatmap_object);
        if (!beatmap) {
            return NULL;
        }

        pppp::Mods mods;
        if (read_mods(state, mods_object, mods) < 0) {
            return NULL;
        }
        if (read_classic(classic_object, mods) < 0) {
            return NULL;
        }
        difficulty.mods(mods);
        if (read_ruleset(ruleset_object, difficulty) < 0) {
            return NULL;
        }
        if (clock_rate_object != Py_None) {
            const double value = PyFloat_AsDouble(clock_rate_object);
            if (value == -1.0 && PyErr_Occurred()) {
                return NULL;
            }
            difficulty.clock_rate(value);
        }
        return beatmap;
    }

    PyObject* raise_status(State* state, const pppp::Status& status) {
        if (status.code() == pppp::StatusCode::ALLOCATION) {
            return PyErr_NoMemory();
        }
        PyErr_SetString(state->error, status.message());
        return NULL;
    }

    PyObject* calculate_difficulty(PyObject* module, PyObject* args) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        pppp::Difficulty difficulty;
        const BeatmapObject* beatmap = read_difficulty(state, args, "OOOOO:calculate_difficulty", difficulty);
        if (!beatmap) {
            return NULL;
        }

        pppp::DifficultyAttributes attributes;
        PyThreadState* thread = PyEval_SaveThread();
        const pppp::Status status = difficulty.calculate(*beatmap->map, attributes);
        PyEval_RestoreThread(thread);
        if (!status.ok()) {
            return raise_status(state, status);
        }

        const PauseGC pause;
        const Builder builder(state);
        return builder.difficulty_attributes(attributes);
    }

    PyObject* calculate_timed_difficulty(PyObject* module, PyObject* args) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        pppp::Difficulty difficulty;
        const BeatmapObject* beatmap =
            read_difficulty(state, args, "OOOOO:calculate_timed_difficulty", difficulty);
        if (!beatmap) {
            return NULL;
        }

        std::vector<pppp::TimedDifficultyAttributes> timed;
        PyThreadState* thread = PyEval_SaveThread();
        const pppp::Status status = difficulty.calculate_timed(*beatmap->map, timed);
        PyEval_RestoreThread(thread);
        if (!status.ok()) {
            return raise_status(state, status);
        }

        const PauseGC pause;
        const Builder builder(state);
        return builder.timed_difficulty(timed);
    }

    PyObject* calculate_strains(PyObject* module, PyObject* args) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        pppp::Difficulty difficulty;
        const BeatmapObject* beatmap = read_difficulty(state, args, "OOOOO:calculate_strains", difficulty);
        if (!beatmap) {
            return NULL;
        }

        pppp::Strains strains;
        PyThreadState* thread = PyEval_SaveThread();
        const pppp::Status status = difficulty.strains(*beatmap->map, strains);
        PyEval_RestoreThread(thread);
        if (!status.ok()) {
            return raise_status(state, status);
        }

        const PauseGC pause;
        const Builder builder(state);
        return builder.strains(strains);
    }

    int read_optional_long(PyObject* object, long* out, bool* present) {
        *present = object != Py_None;
        if (!*present) {
            return 0;
        }
        *out = PyLong_AsLong(object);
        return *out == -1 && PyErr_Occurred() ? -1 : 0;
    }

    int read_statistics(PyObject* object, int* out) {
        if (object == Py_None) {
            return 0;
        }
        PyRef items(PyMapping_Items(object));
        if (!items.get()) {
            return -1;
        }
        const Py_ssize_t count = PyList_Size(items.get());
        for (Py_ssize_t i = 0; i < count; i++) {
            PyObject* item = PyList_GetItem(items.get(), i);
            const long key = PyLong_AsLong(PyTuple_GetItem(item, 0));
            if (key == -1 && PyErr_Occurred()) {
                return -1;
            }
            if (key < 0 || key >= pppp::common::HIT_RESULT_COUNT) {
                PyErr_SetString(PyExc_ValueError, "statistics keys must be hit results");
                return -1;
            }
            const long value = PyLong_AsLong(PyTuple_GetItem(item, 1));
            if (value == -1 && PyErr_Occurred()) {
                return -1;
            }
            out[key] = static_cast<int>(value);
        }
        return 0;
    }

    struct AttributeSource {
        PyObject** fields;
        const Py_ssize_t* slots;

        double number(int field) const {
            const double out = PyFloat_AsDouble(fields[slots[field]]);
            if (PyErr_Occurred()) {
                PyErr_Clear();
                return 0.0;
            }
            return out;
        }

        int integer(int field) const {
            const long out = PyLong_AsLong(fields[slots[field]]);
            if (PyErr_Occurred()) {
                PyErr_Clear();
                return 0;
            }
            return static_cast<int>(out);
        }
    };

    bool is_record(State* state, PyObject* object, int kind, AttributeSource* source) {
        if (!PyObject_TypeCheck(object, reinterpret_cast<PyTypeObject*>(state->types[kind]))) {
            return false;
        }
        source->fields = record_fields(object);
        source->slots = state->slots[kind];
        return true;
    }

    int read_attributes(State* state, PyObject* object, pppp::DifficultyAttributes& out) {
        AttributeSource source;
        if (is_record(state, object, t_osu_difficulty_attributes, &source)) {
            pppp::OsuDifficultyAttributes attributes;
            attributes.star_rating = source.number(f_star_rating);
            attributes.max_combo = source.integer(f_max_combo);
            attributes.aim_difficulty = source.number(f_aim_difficulty);
            attributes.speed_difficulty = source.number(f_speed_difficulty);
            attributes.reading_difficulty = source.number(f_reading_difficulty);
            attributes.flashlight_difficulty = source.number(f_flashlight_difficulty);
            attributes.slider_factor = source.number(f_slider_factor);
            attributes.aim_difficult_strain_count = source.number(f_aim_difficult_strain_count);
            attributes.speed_difficult_strain_count = source.number(f_speed_difficult_strain_count);
            attributes.reading_difficult_note_count = source.number(f_reading_difficult_note_count);
            attributes.aim_difficult_slider_count = source.number(f_aim_difficult_slider_count);
            attributes.aim_top_weighted_slider_factor = source.number(f_aim_top_weighted_slider_factor);
            attributes.speed_top_weighted_slider_factor = source.number(f_speed_top_weighted_slider_factor);
            attributes.speed_note_count = source.number(f_speed_note_count);
            attributes.hit_circle_count = source.integer(f_hit_circle_count);
            attributes.slider_count = source.integer(f_slider_count);
            attributes.large_tick_count = source.integer(f_large_tick_count);
            attributes.spinner_count = source.integer(f_spinner_count);
            attributes.nested_score_per_object = source.number(f_nested_score_per_object);
            attributes.legacy_score_base_multiplier = source.number(f_legacy_score_base_multiplier);
            attributes.maximum_legacy_combo_score = source.number(f_maximum_legacy_combo_score);
            out = attributes;
            return 0;
        }
        if (is_record(state, object, t_taiko_difficulty_attributes, &source)) {
            pppp::TaikoDifficultyAttributes attributes;
            attributes.star_rating = source.number(f_star_rating);
            attributes.max_combo = source.integer(f_max_combo);
            attributes.mechanical_difficulty = source.number(f_mechanical_difficulty);
            attributes.rhythm_difficulty = source.number(f_rhythm_difficulty);
            attributes.reading_difficulty = source.number(f_reading_difficulty);
            attributes.colour_difficulty = source.number(f_colour_difficulty);
            attributes.stamina_difficulty = source.number(f_stamina_difficulty);
            attributes.mono_stamina_factor = source.number(f_mono_stamina_factor);
            attributes.consistency_factor = source.number(f_consistency_factor);
            attributes.stamina_top_strains = source.number(f_stamina_top_strains);
            out = attributes;
            return 0;
        }
        if (is_record(state, object, t_catch_difficulty_attributes, &source)) {
            pppp::CatchDifficultyAttributes attributes;
            attributes.star_rating = source.number(f_star_rating);
            attributes.max_combo = source.integer(f_max_combo);
            out = attributes;
            return 0;
        }
        if (is_record(state, object, t_mania_difficulty_attributes, &source)) {
            pppp::ManiaDifficultyAttributes attributes;
            attributes.star_rating = source.number(f_star_rating);
            attributes.max_combo = source.integer(f_max_combo);
            out = attributes;
            return 0;
        }
        PyErr_SetString(PyExc_TypeError, "expected difficulty attributes");
        return -1;
    }

    PyObject* calculate_performance(PyObject* module, PyObject* args) {
        PyObject* beatmap_object;
        PyObject* mods_object;
        PyObject* max_combo_object;
        PyObject* accuracy_object;
        PyObject* misses_object;
        PyObject* statistics_object;
        PyObject* legacy_total_score_object;
        PyObject* attributes_object;
        PyObject* classic_object;
        if (!PyArg_ParseTuple(args, "OOOOOOOOO:calculate_performance", &beatmap_object, &mods_object,
                              &max_combo_object, &accuracy_object, &misses_object, &statistics_object,
                              &legacy_total_score_object, &attributes_object, &classic_object)) {
            return NULL;
        }
        State* state = static_cast<State*>(PyModule_GetState(module));
        const BeatmapObject* beatmap = read_beatmap(state, beatmap_object);
        if (!beatmap) {
            return NULL;
        }

        pppp::Mods mods;
        if (read_mods(state, mods_object, mods) < 0) {
            return NULL;
        }
        if (read_classic(classic_object, mods) < 0) {
            return NULL;
        }
        long max_combo = 0;
        long misses = 0;
        bool has_max_combo = false;
        bool has_misses = false;
        if (read_optional_long(max_combo_object, &max_combo, &has_max_combo) < 0 ||
            read_optional_long(misses_object, &misses, &has_misses) < 0) {
            return NULL;
        }
        const bool has_accuracy = accuracy_object != Py_None;
        double accuracy = 0.0;
        if (has_accuracy) {
            accuracy = PyFloat_AsDouble(accuracy_object);
            if (accuracy == -1.0 && PyErr_Occurred()) {
                return NULL;
            }
        }

        pppp::ScoreInfo score;
        if (read_statistics(statistics_object, score.statistics) < 0) {
            return NULL;
        }
        if (legacy_total_score_object != Py_None) {
            const long long value = PyLong_AsLongLong(legacy_total_score_object);
            if (value == -1 && PyErr_Occurred()) {
                return NULL;
            }
            score.legacy_total_score = static_cast<pppp_int64>(value);
        }

        pppp::Performance performance(*beatmap->map);
        if (attributes_object != Py_None) {
            pppp::DifficultyAttributes provided;
            if (read_attributes(state, attributes_object, provided) < 0) {
                return NULL;
            }
            performance.attributes(provided);
        }
        performance.score(score);
        if (!mods.empty()) {
            performance.mods(mods);
        }
        if (has_max_combo) {
            performance.combo(static_cast<int>(max_combo));
        }
        if (has_accuracy) {
            performance.accuracy(accuracy);
        }
        if (has_misses) {
            performance.misses(static_cast<int>(misses));
        }

        pppp::PerformanceAttributes attributes;
        PyThreadState* thread = PyEval_SaveThread();
        const pppp::Status status = performance.calculate(attributes);
        PyEval_RestoreThread(thread);
        if (!status.ok()) {
            return raise_status(state, status);
        }

        const PauseGC pause;
        const Builder builder(state);
        return builder.performance_attributes(attributes);
    }

    int add_version(PyObject* module) {
        PyRef version(
            PyUnicode_FromFormat("%d.%d.%d", PPPP_VERSION_MAJOR, PPPP_VERSION_MINOR, PPPP_VERSION_PATCH));
        if (!version.get()) {
            return -1;
        }
        return PyModule_AddObjectRef(module, "version", version.get());
    }

    int bind_types(PyObject* module) {
        PyRef package(PyObject_GetAttrString(module, "__package__"));
        if (!package.get()) {
            return -1;
        }
        PyRef name(PyUnicode_FromFormat("%U._model", package.get()));
        if (!name.get()) {
            return -1;
        }
        PyRef model(PyImport_Import(name.get()));
        if (!model.get()) {
            return -1;
        }
        State* state = static_cast<State*>(PyModule_GetState(module));
        for (int kind = 0; kind < type_count; kind++) {
            if (!state->types[kind]) {
                PyErr_Format(PyExc_ImportError, "pppp._model does not define %s", type_names[kind]);
                return -1;
            }
        }
        return 0;
    }

    PyObject* add_error(PyObject* module, const char* name, const char* attribute, PyObject* base) {
        PyRef bases(base ? PyTuple_Pack(2, base, PyExc_ValueError) : NULL);
        if (base && !bases.get()) {
            return NULL;
        }
        PyObject* error = PyErr_NewException(name, base ? bases.get() : NULL, NULL);
        if (!error || PyModule_AddObjectRef(module, attribute, error) < 0) {
            Py_XDECREF(error);
            return NULL;
        }
        return error;
    }

    int add_errors(PyObject* module) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        state->error = add_error(module, "pppp.Error", "Error", NULL);
        if (!state->error) {
            return -1;
        }
        state->mods_error = add_error(module, "pppp.ModsError", "ModsError", state->error);
        if (!state->mods_error) {
            return -1;
        }
        state->parse_error = add_error(module, "pppp.ParseError", "ParseError", state->error);
        return state->parse_error ? 0 : -1;
    }

    int add_beatmap_type(PyObject* module) {
        PyType_Slot beatmap_slots[] = {
            {Py_tp_dealloc, slot(beatmap_dealloc)}, {Py_tp_getset, beatmap_getset},
            {Py_tp_methods, beatmap_methods},       {Py_tp_traverse, slot(beatmap_traverse)},
            {Py_tp_clear, slot(beatmap_clear)},     {0, NULL}};
        PyType_Spec spec = {"pppp.Beatmap", static_cast<int>(sizeof(BeatmapObject)), 0,
                            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, beatmap_slots};
        State* state = static_cast<State*>(PyModule_GetState(module));
        state->beatmap_type = PyType_FromModuleAndSpec(module, &spec, NULL);
        if (!state->beatmap_type) {
            return -1;
        }
        return PyModule_AddType(module, reinterpret_cast<PyTypeObject*>(state->beatmap_type));
    }

    PyMethodDef methods[] = {
        {"_record", make_record, METH_VARARGS, NULL},
        {"_restore_record", restore_record, METH_O, NULL},
        {"calculate_difficulty", calculate_difficulty, METH_VARARGS,
         "Calculate the difficulty attributes of a beatmap."},
        {"calculate_timed_difficulty", calculate_timed_difficulty, METH_VARARGS,
         "Calculates the difficulty of the beatmap using a specific mod combination and returns a set of "
         "TimedDifficultyAttributes representing the difficulty at every relevant time value in the "
         "beatmap."},
        {"calculate_strains", calculate_strains, METH_VARARGS,
         "Perform the difficulty calculation but instead of evaluating the skill strains, return them as "
         "is."},
        {"calculate_performance", calculate_performance, METH_VARARGS,
         "Calculate the performance attributes of a play on a beatmap."},
        {NULL, NULL, 0, NULL}};

    int traverse(PyObject* module, visitproc visit, void* arg) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        if (!state) {
            return 0;
        }
        for (int kind = 0; kind < type_count; kind++) {
            Py_VISIT(state->types[kind]);
        }
        Py_VISIT(state->beatmap_type);
        Py_VISIT(state->error);
        Py_VISIT(state->mods_error);
        Py_VISIT(state->parse_error);
        return 0;
    }

    int clear(PyObject* module) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        if (!state) {
            return 0;
        }
        for (int kind = 0; kind < type_count; kind++) {
            Py_CLEAR(state->types[kind]);
        }
        Py_CLEAR(state->beatmap_type);
        Py_CLEAR(state->error);
        Py_CLEAR(state->mods_error);
        Py_CLEAR(state->parse_error);
        return 0;
    }

    // Allow exec_module to omit calling clear on error.
    void free_module(void* module) { clear(static_cast<PyObject*>(module)); }

    int exec_module(PyObject* module) {
        if (add_version(module) < 0 || add_errors(module) < 0) {
            return -1;
        }
        if (add_beatmap_type(module) < 0) {
            return -1;
        }
        return bind_types(module);
    }

    PyModuleDef_Slot slots[] = {{Py_mod_exec, slot(exec_module)}, {0, NULL}};

    PyModuleDef definition = {
        PyModuleDef_HEAD_INIT, "_core", 0, sizeof(State), methods, slots, traverse, clear, free_module};

} // namespace

PyMODINIT_FUNC PyInit__core() { // NOLINT(readability-identifier-naming)
    return PyModuleDef_Init(&definition);
}
