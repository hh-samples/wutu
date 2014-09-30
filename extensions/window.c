#include "window.h"

static int
Window_init(Window* self, PyObject* args, PyObject* kwargs) {
    PyObject* visible;
    char* title;
    Uint32 flags = SDL_WINDOW_OPENGL;
    if (! PyArg_ParseTuple(args, "iisO", &self->width, &self->height, &title, &visible)) {
        return NULL;
    }
    if (!PyObject_IsTrue(visible)) {
        flags |= SDL_WINDOW_HIDDEN;
    }
    self->instance = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, self->width, self->height, flags);
    if (self->instance == NULL) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }
    return 0;
}

static void
Window_dealloc(Window* self) {
    SDL_DestroyWindow(self->instance);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject*
Window_update(Window* self) {
    SDL_GL_SwapWindow(self->instance);
    Py_RETURN_NONE;
}

static PyGetSetDef Window_properties[] = {
    {NULL}
};

static PyMethodDef Window_methods[] = {
    {
        "update",
        (PyCFunction)Window_update,
        METH_NOARGS,
        "..."
    },
    {NULL}
};

PyTypeObject WindowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "wutu._graphics.Window",
    sizeof(Window),
    0,                         /* tp_itemsize */
    (destructor)Window_dealloc,
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    "Represents a program window in the windowing system.",
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Window_methods,
    0,                         /* tp_members */
    Window_properties,
    0,
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Window_init,
    0,                         /* tp_alloc */
    (newfunc)PyType_GenericNew
};
