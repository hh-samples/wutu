#ifndef WINDOW_H
#define WINDOW_H

#include <Python.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

typedef struct {
    PyObject_HEAD
    SDL_Window* instance;
    int width;
    int height;
} Window;

extern PyTypeObject WindowType;

#endif /* WINDOW_H */
