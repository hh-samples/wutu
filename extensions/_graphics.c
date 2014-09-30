#include <Python.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "window.h"
#include "renderer.h"
#include "font.h"

static PyObject*
load_image_attributes_from_file(PyObject* self, PyObject* args) {
    char* path;
    unsigned char* data;
    int width, height, components;
    PyObject* pixels;
    PyObject* attributes;
    if (! PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }
    data = stbi_load(path, &width, &height, &components, 0);
    if (data == NULL) {
        PyErr_SetString(PyExc_RuntimeError, stbi_failure_reason());
        return NULL;
    }
    pixels = PyBytes_FromStringAndSize((const char*)data, width * height * components);
    free(data);
    if (pixels == NULL) {
        return NULL;
    }
    attributes = Py_BuildValue("Oiii", pixels, width, height, components);
    Py_DECREF(pixels);
    return attributes;
}

static PyObject*
save_image_pixels_to_png_file(PyObject* self, PyObject* args) {
    char* path;
    char* data;
    PyObject* pixels;
    int width, height, components;
    if (! PyArg_ParseTuple(args, "Oiiis", &pixels, &width, &height, &components, &path)) {
        return NULL;
    }
    data = PyBytes_AsString(pixels);
    if (! stbi_write_png(path, width, height, components, data, 0)) {
        PyErr_SetString(PyExc_RuntimeError, stbi_failure_reason());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef _graphics_methods[] = {
    {
        "load_image_attributes_from_file",
        load_image_attributes_from_file,
        METH_VARARGS,
        "Loads image data from file."
    },
    {
        "save_image_pixels_to_png_file",
        save_image_pixels_to_png_file,
        METH_VARARGS,
        "Saves image pixels to PNG file."
    },
    {NULL, NULL, 0, NULL}
};

static PyModuleDef _graphics_module = {
    PyModuleDef_HEAD_INIT,
    "wutu._graphics",
    "...",
    -1,
    _graphics_methods,
    NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit__graphics(void)
{
    PyObject* module;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return NULL;
    }

    if (Font_Init() != 0) {
        PyErr_SetString(PyExc_RuntimeError, Font_GetError());
        return NULL;
    }

    if (PyType_Ready(&WindowType) < 0) {
        return NULL;
    }

    if (PyType_Ready(&FontType) < 0) {
        return NULL;
    }

    if (PyType_Ready(&RendererType) < 0) {
        return NULL;
    }

    module = PyModule_Create(&_graphics_module);
    if (module == NULL) {
        return NULL;
    }

    Py_INCREF(&WindowType);
    PyModule_AddObject(module, "Window", (PyObject *)&WindowType);

    Py_INCREF(&FontType);
    PyModule_AddObject(module, "Font", (PyObject *)&FontType);

    Py_INCREF(&RendererType);
    PyModule_AddObject(module, "Renderer", (PyObject *)&RendererType);

    return module;
}
