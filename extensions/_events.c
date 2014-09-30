#include <Python.h>
#include <SDL2/SDL.h>

#include "event.h"

static PyObject*
poll_event(PyObject* self, PyObject* args) {
    PyObject* instance;
    SDL_Event e;
    if(SDL_PollEvent(&e)) {
        instance = PyObject_CallObject((PyObject *)&EventType, args);
        ((Event*)instance)->e = e;
        return instance;
    }
    Py_RETURN_NONE;
}

static PyMethodDef _events_methods[] = {
    {
        "poll_event",
        poll_event,
        METH_VARARGS,
        "Polls for currently pending events."
    },
    {NULL, NULL, 0, NULL}
};

static PyModuleDef _events_module = {
    PyModuleDef_HEAD_INIT,
    "wutu._events",
    "...",
    -1,
    _events_methods,
    NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit__events(void)
{
    PyObject* module;

    if (PyType_Ready(&EventType) < 0) {
        return NULL;
    }

    module = PyModule_Create(&_events_module);
    if (module == NULL) {
        return NULL;
    }

    Py_INCREF(&EventType);
    PyModule_AddObject(module, "Event", (PyObject *)&EventType);

    return module;
}
