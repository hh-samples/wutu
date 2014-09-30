#ifndef RENDERER_H
#define RENDERER_H

#include <Python.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "window.h"

typedef struct {
    PyObject_HEAD
    Window* window;
    SDL_GLContext context;
} Renderer;

extern PyTypeObject RendererType;

#endif /* RENDERER_H */
