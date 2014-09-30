#ifndef EVENT_H
#define EVENT_H

#include <Python.h>
#include <structmember.h>
#include <SDL2/SDL.h>

typedef struct {
    PyObject_HEAD
    SDL_Event e;
} Event;

extern PyTypeObject EventType;

#endif /* EVENT_H */
