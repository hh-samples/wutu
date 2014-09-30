#include "renderer.h"

static int
Renderer_init(Renderer* self, PyObject* args, PyObject* kwargs) {
    PyObject* target;
    if (! PyArg_ParseTuple(args, "O", &target)) {
        return NULL;
    }
    if (! PyObject_IsInstance(target, (PyObject*)&WindowType)) {
        PyErr_SetString(PyExc_TypeError, "Renderer window must be instance of wutu.graphics.Window");
        return -1;
    }
    self->window = (Window*)target;
    Py_INCREF(self->window);
    self->context = SDL_GL_CreateContext(self->window->instance);
    if (self->context == NULL) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }
    glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glViewport(0, 0, self->window->width, self->window->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, self->window->width, self->window->height, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return 0;
}

static void
Renderer_dealloc(Renderer* self) {
    Py_DECREF(self->window);
    SDL_free(self->context);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject*
Renderer_restore(Renderer* self) {
    glPopMatrix();
    Py_RETURN_NONE;
}

static PyObject*
Renderer_rotate(Renderer* self, PyObject* args) {
    float angle;
    if (! PyArg_ParseTuple(args, "f", &angle))
    {
        return NULL;
    }
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    Py_RETURN_NONE;
}

static PyObject*
Renderer_save(Renderer* self) {
    glPushMatrix();
    Py_RETURN_NONE;
}

static PyObject*
Renderer_translate(Renderer* self, PyObject* args) {
    float x, y;
    if (! PyArg_ParseTuple(args, "ff", &x, &y)) {
        return NULL;
    }
    glTranslatef(x, y, 0.0f);
    Py_RETURN_NONE;
}

static PyObject*
Renderer__clear(Renderer* self, PyObject* args) {
    float r, g, b, a;
    if (! PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a)) {
        return NULL;
    }
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    Py_RETURN_NONE;
}

static PyObject*
Renderer__draw_line_strip(Renderer* self, PyObject* args) {
    PyObject* coordinates;
    PyObject* smooth;
    double* data;
    float width;
    int length;
    if (! PyArg_ParseTuple(args, "OfO", &coordinates, &width, &smooth)) {
        return NULL;
    }
    length = PySequence_Length(coordinates);
    data = (double*)malloc(length * sizeof(double));
    for(int i = 0; i < length; i++) {
        data[i] = PyFloat_AsDouble(PySequence_GetItem(coordinates, i));
    }

    if (smooth && PyBool_Check(smooth) && PyObject_IsTrue(smooth)) {
        glEnable(GL_LINE_SMOOTH);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_DOUBLE, 0, data);
    glLineWidth(width);
    glDrawArrays(GL_LINE_STRIP, 0, length / 2);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LINE_SMOOTH);
    free(data);
    Py_RETURN_NONE;
}

static PyObject*
Renderer__draw_polygon(Renderer* self, PyObject* args) {
    PyObject* coordinates;
    double* data;
    int length;
    if (! PyArg_ParseTuple(args, "O", &coordinates)) {
        return NULL;
    }

    length = PySequence_Length(coordinates);
    data = (double*)malloc(length * sizeof(double));
    for(int i = 0; i < length; i++) {
        data[i] = PyFloat_AsDouble(PySequence_GetItem(coordinates, i));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_DOUBLE, 0, data);
    glDrawArrays(GL_POLYGON, 0, length / 2);
    glDisableClientState(GL_VERTEX_ARRAY);
    free(data);
    Py_RETURN_NONE;
}

static PyObject*
Renderer__draw_textured_polygon(Renderer* self, PyObject* args) {
    PyObject* coordinates;
    PyObject* texture_coordinates;
    double* data;
    double* texture_data;
    int texture, length;
    if (! PyArg_ParseTuple(args, "OOi", &coordinates, &texture_coordinates, &texture)) {
        return NULL;
    }

    length = PySequence_Length(coordinates);

    data = (double*)malloc(length * sizeof(double));
    for(int i = 0; i < length; i++) {
        data[i] = PyFloat_AsDouble(PySequence_GetItem(coordinates, i));
    }

    texture_data = (double*)malloc(length * sizeof(double));
    for(int i = 0; i < length; i++) {
        texture_data[i] = PyFloat_AsDouble(PySequence_GetItem(texture_coordinates, i));
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_DOUBLE, 0, (const GLvoid *)data);
    glTexCoordPointer(2, GL_DOUBLE, 0, (const GLvoid *)texture_data);
    glDrawArrays(GL_POLYGON, 0, length / 2);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    free(data);
    free(texture_data);
    Py_RETURN_NONE;
}

static PyObject*
Renderer__generate_texture(Renderer* self, PyObject* args) {
    GLuint id;
    Py_buffer data;
    int width, height, components, format = GL_RGB;
    if (! PyArg_ParseTuple(args, "y*iii", &data, &width, &height, &components)) {
        return NULL;
    }
    // TODO: support GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA
    if (components == 4) {
        format = GL_RGBA;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data.buf);

    return Py_BuildValue("i", id);
}

static PyObject*
Renderer__read_pixels(Renderer* self, PyObject* args) {
    int width, height, size, row_size, components = 3;
    GLubyte* buffer;
    GLubyte* data;
    PyObject* pixels;
    if (! PyArg_ParseTuple(args, "ii", &width, &height)) {
        return NULL;
    }
    row_size = width * components;
    size = height * row_size;
    buffer = (GLubyte*)malloc(size * sizeof(GLubyte));
    data = (GLubyte*)malloc(size * sizeof(GLubyte));
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    // flip Y
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < row_size; x++) {
            data[y * row_size + x] = buffer[(height - y - 1) * row_size + x];
        }
    }
    pixels = PyBytes_FromStringAndSize((const char*)data, size);
    free(data);
    free(buffer);
    return pixels;
}

static PyObject*
Renderer__set_color(Renderer* self, PyObject* args) {
    float r, g, b, a;
    if (! PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a)) {
        return NULL;
    }
    glColor4f(r, g, b, a);
    Py_RETURN_NONE;
}

static PyGetSetDef Renderer_getsetters[] = {
    {NULL}
};

static PyMethodDef Renderer_methods[] = {
    {
        "restore",
        (PyCFunction)Renderer_restore,
        METH_NOARGS,
        "..."
    },
    {
        "rotate",
        (PyCFunction)Renderer_rotate,
        METH_VARARGS,
        "..."
    },
    {
        "save",
        (PyCFunction)Renderer_save,
        METH_NOARGS,
        "..."
    },
    {
        "translate",
        (PyCFunction)Renderer_translate,
        METH_VARARGS,
        "..."
    },
    {
        "_clear",
        (PyCFunction)Renderer__clear,
        METH_VARARGS,
        "..."
    },
    {
        "_draw_line_strip",
        (PyCFunction)Renderer__draw_line_strip,
        METH_VARARGS,
        "..."
    },
    {
        "_draw_polygon",
        (PyCFunction)Renderer__draw_polygon,
        METH_VARARGS,
        "..."
    },
    {
        "_draw_textured_polygon",
        (PyCFunction)Renderer__draw_textured_polygon,
        METH_VARARGS,
        "..."
    },
    {
        "_generate_texture",
        (PyCFunction)Renderer__generate_texture,
        METH_VARARGS,
        "..."
    },
    {
        "_read_pixels",
        (PyCFunction)Renderer__read_pixels,
        METH_VARARGS,
        "..."
    },
    {
        "_set_color",
        (PyCFunction)Renderer__set_color,
        METH_VARARGS,
        "..."
    },
    {NULL}
};

PyTypeObject RendererType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "wutu._graphics.Renderer",
    sizeof(Renderer),
    0,                         /* tp_itemsize */
    (destructor)Renderer_dealloc,
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
    "Represents OpenGL 2D rendering context for a window.",
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Renderer_methods,
    0,                         /* tp_members */
    Renderer_getsetters,
    0,
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Renderer_init,
    0,                         /* tp_alloc */
    (newfunc)PyType_GenericNew
};
