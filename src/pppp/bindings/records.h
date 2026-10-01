#ifndef PPPP_BINDINGS_RECORDS_H
#define PPPP_BINDINGS_RECORDS_H

#include <Python.h>
#include <structmember.h>

namespace {

    // Object-valued slots hold eager Python values, not C numbers boxed on read.
    struct RecordObject {
        PyObject_HEAD Py_ssize_t field_count;
    };

    PyObject** record_fields(PyObject* object) {
        return reinterpret_cast<PyObject**>(reinterpret_cast<char*>(object) + sizeof(RecordObject));
    }

    const PyMemberDef* record_members(PyTypeObject* type) {
        return static_cast<const PyMemberDef*>(PyType_GetSlot(type, Py_tp_members));
    }

    Py_ssize_t record_member_count(const PyMemberDef* members) {
        Py_ssize_t count = 0;
        while (members[count].name) {
            count++;
        }
        return count;
    }

    PyObject* allocate_record(PyTypeObject* type, Py_ssize_t count) {
        PyObject* object = PyType_GenericAlloc(type, 0);
        if (object) {
            reinterpret_cast<RecordObject*>(object)->field_count = count;
        }
        return object;
    }

    int record_traverse(PyObject* object, visitproc visit, void* arg) {
        Py_VISIT(Py_TYPE(object));
        PyObject** fields = record_fields(object);
        const Py_ssize_t count = reinterpret_cast<RecordObject*>(object)->field_count;
        for (Py_ssize_t i = 0; i < count; i++) {
            Py_VISIT(fields[i]);
        }
        return 0;
    }

    int record_clear(PyObject* object) {
        PyObject** fields = record_fields(object);
        const Py_ssize_t count = reinterpret_cast<RecordObject*>(object)->field_count;
        for (Py_ssize_t i = 0; i < count; i++) {
            Py_CLEAR(fields[i]);
        }
        return 0;
    }

    void record_dealloc(PyObject* object) {
        PyObject_GC_UnTrack(object);
        record_clear(object);
        PyTypeObject* type = Py_TYPE(object);
        PyObject_GC_Del(object);
        Py_DECREF(type);
    }

    PyObject* record_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
        const PyMemberDef* members = record_members(type);
        const Py_ssize_t count = record_member_count(members);
        const Py_ssize_t positional = PyTuple_Size(args);
        if (positional > count) {
            PyErr_SetString(PyExc_TypeError, "too many positional arguments");
            return NULL;
        }
        PyObject* object = allocate_record(type, count);
        if (!object) {
            return NULL;
        }
        Py_ssize_t keywords = 0;
        for (Py_ssize_t i = 0; i < count; i++) {
            PyObject* keyword = kwargs ? PyDict_GetItemString(kwargs, members[i].name) : NULL;
            if (i < positional && keyword) {
                PyErr_Format(PyExc_TypeError, "multiple values for %s", members[i].name);
                Py_DECREF(object);
                return NULL;
            }
            PyObject* value = i < positional ? PyTuple_GetItem(args, i) : keyword;
            if (!value) {
                PyErr_Format(PyExc_TypeError, "missing required argument: %s", members[i].name);
                Py_DECREF(object);
                return NULL;
            }
            keywords += keyword != NULL;
            record_fields(object)[i] = Py_NewRef(value);
        }
        if (kwargs && keywords != PyDict_Size(kwargs)) {
            PyErr_SetString(PyExc_TypeError, "unexpected keyword argument");
            Py_DECREF(object);
            return NULL;
        }
        return object;
    }

    PyObject* record_values(PyObject* object, const PyMemberDef* members) {
        const Py_ssize_t count = record_member_count(members);
        PyObject* values = PyTuple_New(count);
        if (!values) {
            return NULL;
        }
        for (Py_ssize_t i = 0; i < count; i++) {
            PyObject* value =
                *reinterpret_cast<PyObject**>(reinterpret_cast<char*>(object) + members[i].offset);
            Py_XINCREF(value);
            if (!value || PyTuple_SetItem(values, i, value) < 0) {
                Py_DECREF(values);
                if (!value) {
                    PyErr_SetString(PyExc_AttributeError, "record field is unset");
                }
                return NULL;
            }
        }
        return values;
    }

    PyObject* record_equal(PyObject* left, PyObject* right, int op) {
        if (Py_TYPE(left) != Py_TYPE(right) || (op != Py_EQ && op != Py_NE)) {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        const PyMemberDef* members = record_members(Py_TYPE(left));
        PyObject* a = record_values(left, members);
        PyObject* b = a ? record_values(right, members) : NULL;
        PyObject* result = b ? PyObject_RichCompare(a, b, op) : NULL;
        Py_XDECREF(b);
        Py_XDECREF(a);
        return result;
    }

    PyObject* record_reduce(PyObject* object, PyObject*) {
        const PyMemberDef* members = record_members(Py_TYPE(object));
        PyObject* values = record_values(object, members);
        if (!values) {
            return NULL;
        }
        PyObject* type = reinterpret_cast<PyObject*>(Py_TYPE(object));
        PyObject* module_name = PyObject_GetAttrString(type, "__module__");
        PyObject* module = module_name ? PyImport_Import(module_name) : NULL;
        PyObject* restore = module ? PyObject_GetAttrString(module, "_restore") : NULL;
        PyObject* args = restore ? PyTuple_Pack(1, type) : NULL;
        PyObject* result = args ? PyTuple_Pack(3, restore, args, values) : NULL;
        Py_XDECREF(args);
        Py_XDECREF(restore);
        Py_XDECREF(module);
        Py_XDECREF(module_name);
        Py_DECREF(values);
        return result;
    }

    PyObject* record_setstate(PyObject* object, PyObject* values) {
        const Py_ssize_t count = reinterpret_cast<RecordObject*>(object)->field_count;
        if (!PyTuple_Check(values) || PyTuple_Size(values) != count) {
            PyErr_SetString(PyExc_TypeError, "record state must contain every field");
            return NULL;
        }
        PyObject** fields = record_fields(object);
        for (Py_ssize_t i = 0; i < count; i++) {
            if (fields[i]) {
                PyErr_SetString(PyExc_AttributeError, "record is already initialized");
                return NULL;
            }
        }
        for (Py_ssize_t i = 0; i < count; i++) {
            fields[i] = Py_NewRef(PyTuple_GetItem(values, i));
        }
        Py_RETURN_NONE;
    }

    PyObject* record_repr(PyObject* object) {
        const PyMemberDef* members = record_members(Py_TYPE(object));
        const int entered = Py_ReprEnter(object);
        if (entered < 0) {
            return NULL;
        }
        if (entered) {
            return PyUnicode_FromString("...");
        }
        PyObject* fields = PyList_New(0);
        PyObject* separator = NULL;
        PyObject* joined = NULL;
        PyObject* result = NULL;
        if (!fields) {
            goto done;
        }
        for (const PyMemberDef* member = members; member->name; member++) {
            PyObject* value = *reinterpret_cast<PyObject**>(reinterpret_cast<char*>(object) + member->offset);
            PyObject* field = PyUnicode_FromFormat("%s=%R", member->name, value ? value : Py_None);
            if (!field) {
                goto done;
            }
            const int added = PyList_Append(fields, field);
            Py_DECREF(field);
            if (added < 0) {
                goto done;
            }
        }
        separator = PyUnicode_FromString(", ");
        if (!separator) {
            goto done;
        }
        joined = PyUnicode_Join(separator, fields);
        if (joined) {
            PyObject* name = PyObject_GetAttrString(reinterpret_cast<PyObject*>(Py_TYPE(object)), "__name__");
            if (name) {
                result = PyUnicode_FromFormat("%U(%U)", name, joined);
                Py_DECREF(name);
            }
        }
    done:
        Py_XDECREF(joined);
        Py_XDECREF(separator);
        Py_XDECREF(fields);
        Py_ReprLeave(object);
        return result;
    }

    PyMethodDef record_methods[] = {{"__reduce__", record_reduce, METH_NOARGS, NULL},
                                    {"__setstate__", record_setstate, METH_O, NULL},
                                    {NULL, NULL, 0, NULL}};

} // namespace

#endif
