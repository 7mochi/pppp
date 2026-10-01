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
        f_statistics,
        f_maximum_statistics,
        f_max_combo,
        f_accuracy,
        f_legacy_total_score,
        f_ruleset,
        f_star_rating,
        f_osu,
        f_taiko,
        f_fruits,
        f_mania,
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
                                 "statistics",
                                 "maximum_statistics",
                                 "max_combo",
                                 "accuracy",
                                 "legacy_total_score",
                                 "ruleset",
                                 "star_rating",
                                 "osu",
                                 "taiko",
                                 "fruits",
                                 "mania",
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
                                 "estimated_unstable_rate"};

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
        t_score_info,
        type_count
    };

    const char* type_names[] = {"Vector2",     "BeatmapDifficulty", "HitObject",   "Slider",
                                "TimingPoint", "BreakPeriod",       "SliderEvent", "ScoreInfo"};

    PPPP_STATIC_ASSERT(type_names_match_type_count, sizeof(type_names) / sizeof(type_names[0]) == type_count);

    const char* qualified_names[] = {"pppp._model.Vector2",     "pppp._model.BeatmapDifficulty",
                                     "pppp._model.HitObject",   "pppp._model.Slider",
                                     "pppp._model.TimingPoint", "pppp._model.BreakPeriod",
                                     "pppp._model.SliderEvent", "pppp._model.ScoreInfo"};

    struct State {
        PyObject* types[type_count];
        Py_ssize_t slots[type_count][field_count];
        Py_ssize_t sizes[type_count];
        PyObject* beatmap_type;
    };

    struct BeatmapObject {
        PyObject_HEAD pppp::beatmaps::Beatmap* map;
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

    PyObject* beatmap_field(PyObject* object, PyObject* BeatmapObject::* member) {
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        PyObject* value = beatmap->*member;
        if (!value) {
            PyErr_SetString(PyExc_AttributeError, "beatmap is not loaded");
            return NULL;
        }
        return Py_NewRef(value);
    }

    PyObject* beatmap_get_difficulty(PyObject* object, void*) {
        return beatmap_field(object, &BeatmapObject::difficulty);
    }

    PyObject* beatmap_get_hit_objects(PyObject* object, void*) {
        return beatmap_field(object, &BeatmapObject::hit_objects);
    }

    PyObject* beatmap_get_sliders(PyObject* object, void*) {
        return beatmap_field(object, &BeatmapObject::sliders);
    }

    PyObject* beatmap_get_timing_points(PyObject* object, void*) {
        return beatmap_field(object, &BeatmapObject::timing_points);
    }

    PyObject* beatmap_get_breaks(PyObject* object, void*) {
        return beatmap_field(object, &BeatmapObject::breaks);
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

    PyObject* from_file(PyObject* module, PyObject* argument) {
        State* state = static_cast<State*>(PyModule_GetState(module));
        PyRef path(PyOS_FSPath(argument));
        if (!path.get()) {
            return NULL;
        }
        if (PyUnicode_Check(path.get())) {
            path.reset(PyUnicode_EncodeFSDefault(path.get()));
            if (!path.get()) {
                return NULL;
            }
        }
        char* bytes = NULL;
        Py_ssize_t size = 0;
        if (PyBytes_AsStringAndSize(path.get(), &bytes, &size) < 0) {
            return NULL;
        }
        if (std::memchr(bytes, 0, static_cast<size_t>(size))) {
            PyErr_SetString(PyExc_ValueError, "embedded null byte");
            return NULL;
        }

        pppp::beatmaps::Beatmap* map = new (std::nothrow) pppp::beatmaps::Beatmap;
        if (!map) {
            return PyErr_NoMemory();
        }
        pppp::Result::Value status;
        // Both native entry points are noexcept, including allocation and I/O failures.
        PyThreadState* thread = PyEval_SaveThread();
        status = pppp::beatmaps::from_file(*map, bytes);
        PyEval_RestoreThread(thread);
        if (status != pppp::Result::OK) {
            delete map;
            if (status == pppp::Result::ALLOCATION) {
                return PyErr_NoMemory();
            }
            PyErr_Format(PyExc_ValueError, "cannot parse the beatmap (result %d)", static_cast<int>(status));
            return NULL;
        }

        PyObject* object = PyType_GenericAlloc(reinterpret_cast<PyTypeObject*>(state->beatmap_type), 0);
        if (!object) {
            delete map;
            return NULL;
        }
        BeatmapObject* beatmap = reinterpret_cast<BeatmapObject*>(object);
        beatmap->map = map;
        {
            const PauseGC pause;
            const Builder builder(state);
            beatmap->difficulty = builder.difficulty(map->difficulty);
            beatmap->hit_objects = builder.hit_objects(map->hit_objects);
            beatmap->sliders = builder.sliders(map->sliders);
            beatmap->timing_points = builder.timing_points(map->timing_points);
            beatmap->breaks = builder.breaks(map->breaks);
        }
        if (!beatmap->difficulty || !beatmap->hit_objects || !beatmap->sliders || !beatmap->timing_points ||
            !beatmap->breaks) {
            Py_DECREF(object);
            return NULL;
        }
        return object;
    }

    void dict_number(PyObject* dict, const char* name, double value) {
        PyObject* number = py_number(value);
        if (number) {
            PyDict_SetItemString(dict, name, number);
        }
        Py_XDECREF(number);
    }

    void dict_integer(PyObject* dict, const char* name, int value) {
        PyObject* number = py_integer(value);
        if (number) {
            PyDict_SetItemString(dict, name, number);
        }
        Py_XDECREF(number);
    }

    PyObject* finish_dict(PyObject* dict) {
        if (PyErr_Occurred()) {
            Py_DECREF(dict);
            return NULL;
        }
        return dict;
    }

    PyObject* osu_attributes(const pppp::osu::difficulty::OsuDifficultyAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "star_rating", attributes.star_rating);
        dict_integer(dict, "max_combo", attributes.max_combo);
        dict_number(dict, "aim_difficulty", attributes.aim_difficulty);
        dict_number(dict, "speed_difficulty", attributes.speed_difficulty);
        dict_number(dict, "reading_difficulty", attributes.reading_difficulty);
        dict_number(dict, "flashlight_difficulty", attributes.flashlight_difficulty);
        dict_number(dict, "slider_factor", attributes.slider_factor);
        dict_number(dict, "aim_difficult_strain_count", attributes.aim_difficult_strain_count);
        dict_number(dict, "speed_difficult_strain_count", attributes.speed_difficult_strain_count);
        dict_number(dict, "reading_difficult_note_count", attributes.reading_difficult_note_count);
        dict_number(dict, "aim_difficult_slider_count", attributes.aim_difficult_slider_count);
        dict_number(dict, "aim_top_weighted_slider_factor", attributes.aim_top_weighted_slider_factor);
        dict_number(dict, "speed_top_weighted_slider_factor", attributes.speed_top_weighted_slider_factor);
        dict_number(dict, "speed_note_count", attributes.speed_note_count);
        dict_integer(dict, "hit_circle_count", attributes.hit_circle_count);
        dict_integer(dict, "slider_count", attributes.slider_count);
        dict_integer(dict, "large_tick_count", attributes.large_tick_count);
        dict_integer(dict, "spinner_count", attributes.spinner_count);
        dict_number(dict, "nested_score_per_object", attributes.nested_score_per_object);
        dict_number(dict, "legacy_score_base_multiplier", attributes.legacy_score_base_multiplier);
        dict_number(dict, "maximum_legacy_combo_score", attributes.maximum_legacy_combo_score);
        return finish_dict(dict);
    }

    PyObject* taiko_attributes(const pppp::taiko::difficulty::TaikoDifficultyAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "star_rating", attributes.star_rating);
        dict_integer(dict, "max_combo", attributes.max_combo);
        dict_number(dict, "mechanical_difficulty", attributes.mechanical_difficulty);
        dict_number(dict, "rhythm_difficulty", attributes.rhythm_difficulty);
        dict_number(dict, "reading_difficulty", attributes.reading_difficulty);
        dict_number(dict, "colour_difficulty", attributes.colour_difficulty);
        dict_number(dict, "stamina_difficulty", attributes.stamina_difficulty);
        dict_number(dict, "mono_stamina_factor", attributes.mono_stamina_factor);
        dict_number(dict, "consistency_factor", attributes.consistency_factor);
        dict_number(dict, "stamina_top_strains", attributes.stamina_top_strains);
        return finish_dict(dict);
    }

    PyObject* catch_attributes(const pppp::fruits::difficulty::CatchDifficultyAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "star_rating", attributes.star_rating);
        dict_integer(dict, "max_combo", attributes.max_combo);
        return finish_dict(dict);
    }

    PyObject* mania_attributes(const pppp::mania::difficulty::ManiaDifficultyAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "star_rating", attributes.star_rating);
        dict_integer(dict, "max_combo", attributes.max_combo);
        return finish_dict(dict);
    }

    PyObject* difficulty_attributes(const pppp::DifficultyAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_integer(dict, "ruleset", static_cast<int>(attributes.ruleset));
        dict_number(dict, "star_rating", attributes.star_rating());
        dict_integer(dict, "max_combo", attributes.max_combo());

        const char* names[] = {"osu", "taiko", "fruits", "mania"};
        PyObject* modes[] = {osu_attributes(attributes.osu), taiko_attributes(attributes.taiko),
                             catch_attributes(attributes.fruits), mania_attributes(attributes.mania)};
        for (int i = 0; i < 4; i++) {
            if (!modes[i] || PyDict_SetItemString(dict, names[i], modes[i]) < 0) {
                for (int j = i; j < 4; j++) {
                    Py_XDECREF(modes[j]);
                }
                Py_DECREF(dict);
                return NULL;
            }
        }
        for (int i = 0; i < 4; i++) {
            Py_DECREF(modes[i]);
        }
        return finish_dict(dict);
    }

    const int MAX_MODS = 64;

    int read_mods(PyObject* object, pppp::mods::Mod* mods, size_t* count) {
        *count = 0;
        if (object == Py_None) {
            return 0;
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
        const int parsed = pppp::mods::mod_from_acronyms(mods, MAX_MODS, spec);
        if (parsed < 0) {
            PyErr_SetString(PyExc_ValueError, "invalid mod specification");
            return -1;
        }
        *count = static_cast<size_t>(parsed);
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

    PyObject* calculate_difficulty(PyObject* module, PyObject* args) {
        PyObject* beatmap_object;
        PyObject* mods_object;
        PyObject* ruleset_object;
        PyObject* clock_rate_object;
        if (!PyArg_ParseTuple(args, "OOOO:calculate_difficulty", &beatmap_object, &mods_object,
                              &ruleset_object, &clock_rate_object)) {
            return NULL;
        }
        State* state = static_cast<State*>(PyModule_GetState(module));
        if (!PyObject_TypeCheck(beatmap_object, reinterpret_cast<PyTypeObject*>(state->beatmap_type))) {
            PyErr_SetString(PyExc_TypeError, "expected a Beatmap");
            return NULL;
        }

        pppp::mods::Mod mods[MAX_MODS];
        size_t mod_count = 0;
        if (read_mods(mods_object, mods, &mod_count) < 0) {
            return NULL;
        }
        pppp::Difficulty difficulty;
        difficulty.mods(mod_count ? mods : 0, mod_count);
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

        const BeatmapObject* beatmap = reinterpret_cast<const BeatmapObject*>(beatmap_object);
        pppp::DifficultyAttributes attributes;
        PyThreadState* thread = PyEval_SaveThread();
        attributes = difficulty.calculate(*beatmap->map);
        PyEval_RestoreThread(thread);

        const PauseGC pause;
        return difficulty_attributes(attributes);
    }

    PyObject* py_none() {
        Py_INCREF(Py_None);
        return Py_None;
    }

    void dict_optional_number(PyObject* dict, const char* name, const nonstd::optional<double>& value) {
        PyObject* number = value.has_value() ? py_number(value.value()) : py_none();
        if (number) {
            PyDict_SetItemString(dict, name, number);
        }
        Py_XDECREF(number);
    }

    PyObject* osu_performance(const pppp::osu::difficulty::OsuPerformanceAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "total", attributes.total);
        dict_number(dict, "aim", attributes.aim);
        dict_number(dict, "speed", attributes.speed);
        dict_number(dict, "accuracy", attributes.accuracy);
        dict_number(dict, "flashlight", attributes.flashlight);
        dict_number(dict, "reading", attributes.reading);
        dict_number(dict, "effective_miss_count", attributes.effective_miss_count);
        dict_number(dict, "combo_based_estimated_miss_count", attributes.combo_based_estimated_miss_count);
        dict_optional_number(dict, "score_based_estimated_miss_count",
                             attributes.score_based_estimated_miss_count);
        dict_number(dict, "aim_estimated_slider_breaks", attributes.aim_estimated_slider_breaks);
        dict_number(dict, "speed_estimated_slider_breaks", attributes.speed_estimated_slider_breaks);
        dict_optional_number(dict, "speed_deviation", attributes.speed_deviation);
        return finish_dict(dict);
    }

    PyObject* taiko_performance(const pppp::taiko::difficulty::TaikoPerformanceAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "total", attributes.total);
        dict_number(dict, "difficulty", attributes.difficulty);
        dict_number(dict, "accuracy", attributes.accuracy);
        dict_optional_number(dict, "estimated_unstable_rate", attributes.estimated_unstable_rate);
        return finish_dict(dict);
    }

    PyObject* catch_performance(const pppp::fruits::difficulty::CatchPerformanceAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "total", attributes.total);
        return finish_dict(dict);
    }

    PyObject* mania_performance(const pppp::mania::difficulty::ManiaPerformanceAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_number(dict, "total", attributes.total);
        dict_number(dict, "difficulty", attributes.difficulty);
        return finish_dict(dict);
    }

    PyObject* performance_attributes(const pppp::PerformanceAttributes& attributes) {
        PyObject* dict = PyDict_New();
        if (!dict) {
            return NULL;
        }
        dict_integer(dict, "ruleset", static_cast<int>(attributes.ruleset));
        dict_number(dict, "total", attributes.total());

        const char* names[] = {"osu", "taiko", "fruits", "mania"};
        PyObject* modes[] = {osu_performance(attributes.osu), taiko_performance(attributes.taiko),
                             catch_performance(attributes.fruits), mania_performance(attributes.mania)};
        for (int i = 0; i < 4; i++) {
            if (!modes[i] || PyDict_SetItemString(dict, names[i], modes[i]) < 0) {
                for (int j = i; j < 4; j++) {
                    Py_XDECREF(modes[j]);
                }
                Py_DECREF(dict);
                return NULL;
            }
        }
        for (int i = 0; i < 4; i++) {
            Py_DECREF(modes[i]);
        }
        return finish_dict(dict);
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
        if (!PyList_Check(object) || PyList_Size(object) != pppp::common::HIT_RESULT_COUNT) {
            PyErr_Format(PyExc_ValueError, "statistics must be a list of %d counts",
                         pppp::common::HIT_RESULT_COUNT);
            return -1;
        }
        for (int i = 0; i < pppp::common::HIT_RESULT_COUNT; i++) {
            const long value = PyLong_AsLong(PyList_GetItem(object, i));
            if (value == -1 && PyErr_Occurred()) {
                return -1;
            }
            out[i] = static_cast<int>(value);
        }
        return 0;
    }

    int read_score(State* state, PyObject* object, pppp::common::ScoreInfo* score) {
        PyObject** fields = record_fields(object);
        const Py_ssize_t* slots = state->slots[t_score_info];
        if (read_statistics(fields[slots[f_statistics]], score->statistics) < 0 ||
            read_statistics(fields[slots[f_maximum_statistics]], score->maximum_statistics) < 0) {
            return -1;
        }
        score->max_combo = static_cast<int>(PyLong_AsLong(fields[slots[f_max_combo]]));
        score->accuracy = PyFloat_AsDouble(fields[slots[f_accuracy]]);
        if (PyErr_Occurred()) {
            return -1;
        }
        PyObject* legacy = fields[slots[f_legacy_total_score]];
        if (legacy != Py_None) {
            const long long value = PyLong_AsLongLong(legacy);
            if (value == -1 && PyErr_Occurred()) {
                return -1;
            }
            score->legacy_total_score = static_cast<pppp_int64>(value);
        }
        return 0;
    }

    PyObject* calculate_performance(PyObject* module, PyObject* args) {
        PyObject* beatmap_object;
        PyObject* mods_object;
        PyObject* score_object;
        PyObject* combo_object;
        PyObject* accuracy_object;
        PyObject* misses_object;
        if (!PyArg_ParseTuple(args, "OOOOOO:calculate_performance", &beatmap_object, &mods_object,
                              &score_object, &combo_object, &accuracy_object, &misses_object)) {
            return NULL;
        }
        State* state = static_cast<State*>(PyModule_GetState(module));
        if (!PyObject_TypeCheck(beatmap_object, reinterpret_cast<PyTypeObject*>(state->beatmap_type))) {
            PyErr_SetString(PyExc_TypeError, "expected a Beatmap");
            return NULL;
        }

        pppp::mods::Mod mods[MAX_MODS];
        size_t mod_count = 0;
        if (read_mods(mods_object, mods, &mod_count) < 0) {
            return NULL;
        }
        long combo = 0;
        long misses = 0;
        bool has_combo = false;
        bool has_misses = false;
        if (read_optional_long(combo_object, &combo, &has_combo) < 0 ||
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

        pppp::common::ScoreInfo score;
        if (score_object != Py_None) {
            if (!PyObject_TypeCheck(score_object,
                                    reinterpret_cast<PyTypeObject*>(state->types[t_score_info]))) {
                PyErr_SetString(PyExc_TypeError, "expected a ScoreInfo");
                return NULL;
            }
            if (read_score(state, score_object, &score) < 0) {
                return NULL;
            }
        }

        const BeatmapObject* beatmap = reinterpret_cast<const BeatmapObject*>(beatmap_object);
        pppp::Performance performance(*beatmap->map);
        performance.state(score);
        if (mod_count) {
            performance.mods(mods, mod_count);
        }
        if (has_combo) {
            performance.combo(static_cast<int>(combo));
        }
        if (has_accuracy) {
            performance.accuracy(accuracy);
        }
        if (has_misses) {
            performance.misses(static_cast<int>(misses));
        }

        pppp::PerformanceAttributes attributes;
        PyThreadState* thread = PyEval_SaveThread();
        attributes = performance.calculate();
        PyEval_RestoreThread(thread);

        const PauseGC pause;
        return performance_attributes(attributes);
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

    int add_beatmap_type(PyObject* module) {
        PyType_Slot beatmap_slots[] = {{Py_tp_dealloc, slot(beatmap_dealloc)},
                                       {Py_tp_getset, beatmap_getset},
                                       {Py_tp_traverse, slot(beatmap_traverse)},
                                       {Py_tp_clear, slot(beatmap_clear)},
                                       {0, NULL}};
        PyType_Spec spec = {"pppp.Beatmap", static_cast<int>(sizeof(BeatmapObject)), 0,
                            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, beatmap_slots};
        State* state = static_cast<State*>(PyModule_GetState(module));
        state->beatmap_type = PyType_FromModuleAndSpec(module, &spec, NULL);
        if (!state->beatmap_type) {
            return -1;
        }
        return PyModule_AddType(module, reinterpret_cast<PyTypeObject*>(state->beatmap_type));
    }

    PyMethodDef methods[] = {{"_record", make_record, METH_VARARGS, NULL},
                             {"_restore_record", restore_record, METH_O, NULL},
                             {"from_file", from_file, METH_O, "Read a beatmap from a file path."},
                             {"calculate_difficulty", calculate_difficulty, METH_VARARGS,
                              "Calculate the difficulty attributes of a beatmap."},
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
        return 0;
    }

    // Allow exec_module to omit calling clear on error.
    void free_module(void* module) { clear(static_cast<PyObject*>(module)); }

    int exec_module(PyObject* module) {
        if (add_version(module) < 0) {
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
