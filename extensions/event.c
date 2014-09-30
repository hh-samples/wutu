#include "event.h"

static PyMemberDef Event_members[] = {
    {
        "type", T_INT,
        offsetof(Event, e.type), 0,
        "..."
    },
    // Fields shared by every event
    {
        "common_type", T_INT,
        offsetof(Event, e.common.type), 0,
        "..."
    },
    {
        "common_timestamp", T_INT,
        offsetof(Event, e.common.timestamp), 0,
        "..."
    },
    // Window state change event data
    {
        "window_type", T_INT,
        offsetof(Event, e.window.type), 0,
        "WE_*"
    },
    {
        "window_timestamp", T_INT,
        offsetof(Event, e.window.timestamp), 0,
        "..."
    },
    {
        "window_id", T_INT,
        offsetof(Event, e.window.windowID), 0,
        "The associated window."
    },
    {
        "window_event", T_INT,
        offsetof(Event, e.window.event), 0,
        "..."
    },
    // Keyboard button event structure
    {
        "key_type", T_INT,
        offsetof(Event, e.key.type), 0,
        "KEYDOWN or KEYUP."
    },
    {
        "key_timestamp", T_INT,
        offsetof(Event, e.key.timestamp), 0,
        "..."
    },
    {
        "key_window_id", T_INT,
        offsetof(Event, e.key.windowID), 0,
        "The window with keyboard focus, if any."
    },
    {
        "key_state", T_INT,
        offsetof(Event, e.key.state), 0,
        "PRESSED or RELEASED."
    },
    {
        "key_repeat", T_INT,
        offsetof(Event, e.key.repeat), 0,
        "Non-zero if this is a key repeat."
    },
    {
        "key_code", T_INT,
        offsetof(Event, e.key.keysym.scancode), 0,
        "SDL physical key code."
    },
    {
        "key_sym", T_INT,
        offsetof(Event, e.key.keysym.sym), 0,
        "SDL virtual key code."
    },
    {
        "key_mod", T_INT,
        offsetof(Event, e.key.keysym.mod), 0,
        "Current key modifiers."
    },
    // Keyboard text editing event structure
    {
        "edit_type", T_INT,
        offsetof(Event, e.edit.type), 0,
        "..."
    },
    {
        "edit_timestamp", T_INT,
        offsetof(Event, e.edit.timestamp), 0,
        "..."
    },
    {
        "edit_window_id", T_INT,
        offsetof(Event, e.edit.windowID), 0,
        "..."
    },
    {
        "edit_text", T_STRING,
        offsetof(Event, e.edit.text), 0,
        "The editing text."
    },
    {
        "edit_start", T_INT,
        offsetof(Event, e.edit.start), 0,
        "The start cursor of selected editing text."
    },
    {
        "edit_length", T_INT,
        offsetof(Event, e.edit.length), 0,
        "The length of selected editing text."
    },
    // Keyboard text input event structure
    {
        "text_type", T_INT,
        offsetof(Event, e.text.type), 0,
        "..."
    },
    {
        "text_timestamp", T_INT,
        offsetof(Event, e.text.timestamp), 0,
        "..."
    },
    {
        "text_window_id", T_INT,
        offsetof(Event, e.text.windowID), 0,
        "..."
    },
    {
        "text", T_STRING,
        offsetof(Event, e.text.text), 0,
        "The input text."
    },
    // Mouse motion event structure
    {
        "motion_type", T_INT,
        offsetof(Event, e.motion.type), 0,
        "..."
    },
    {
        "motion_timestamp", T_INT,
        offsetof(Event, e.motion.timestamp), 0,
        "..."
    },
    {
        "motion_window_id", T_INT,
        offsetof(Event, e.motion.windowID), 0,
        "..."
    },
    {
        "motion_which", T_INT,
        offsetof(Event, e.motion.which), 0,
        "The mouse instance id."
    },
    {
        "motion_state", T_INT,
        offsetof(Event, e.motion.state), 0,
        "The current button state."
    },
    {
        "motion_x", T_INT,
        offsetof(Event, e.motion.x), 0,
        "X coordinate, relative to window."
    },
    {
        "motion_y", T_INT,
        offsetof(Event, e.motion.y), 0,
        "Y coordinate, relative to window."
    },
    {
        "motion_relative_x", T_INT,
        offsetof(Event, e.motion.xrel), 0,
        "The relative motion in the X direction."
    },
    {
        "motion_relative_y", T_INT,
        offsetof(Event, e.motion.yrel), 0,
        "The relative motion in the Y direction."
    },
    //  Mouse button event structure
    {
        "button_type", T_INT,
        offsetof(Event, e.button.type), 0,
        "..."
    },
    {
        "button_timestamp", T_INT,
        offsetof(Event, e.button.timestamp), 0,
        "..."
    },
    {
        "button_window_id", T_INT,
        offsetof(Event, e.button.windowID), 0,
        "The window with mouse focus."
    },
    {
        "button_which", T_INT,
        offsetof(Event, e.button.which), 0,
        "The mouse instance id."
    },
    {
        "button", T_INT,
        offsetof(Event, e.button.button), 0,
        "The mouse button index."
    },
    {
        "button_state", T_INT,
        offsetof(Event, e.button.state), 0,
        "PRESSED or RELEASED."
    },
    {
        "button_x", T_INT,
        offsetof(Event, e.button.x), 0,
        "X coordinate, relative to window"
    },
    {
        "button_y", T_INT,
        offsetof(Event, e.button.y), 0,
        "Y coordinate, relative to window"
    },
    // Mouse wheel event structure
    {
        "wheel_type", T_INT,
        offsetof(Event, e.wheel.type), 0,
        "..."
    },
    {
        "wheel_timestamp", T_INT,
        offsetof(Event, e.wheel.timestamp), 0,
        "..."
    },
    {
        "wheel_window_id", T_INT,
        offsetof(Event, e.wheel.windowID), 0,
        "..."
    },
    {
        "wheel_which", T_INT,
        offsetof(Event, e.wheel.which), 0,
        "..."
    },
    {
        "wheel_x", T_INT,
        offsetof(Event, e.wheel.x), 0,
        "..."
    },
    {
        "wheel_y", T_INT,
        offsetof(Event, e.wheel.y), 0,
        "..."
    },
    // Joystick axis motion event structure
    {
        "jaxis_type", T_INT,
        offsetof(Event, e.jaxis.type), 0,
        "..."
    },
    // Joystick trackball motion event structure
    {
        "jball_type", T_INT,
        offsetof(Event, e.jball.type), 0,
        "..."
    },
    // Joystick hat position change event structure
    {
        "jhat_type", T_INT,
        offsetof(Event, e.jhat.type), 0,
        "..."
    },
    // Joystick button event structure
    {
        "jbutton_type", T_INT,
        offsetof(Event, e.jbutton.type), 0,
        "..."
    },
    //  Joystick device event structure
    {
        "jdevice_type", T_INT,
        offsetof(Event, e.jdevice.type), 0,
        "..."
    },
    // Game controller axis motion event structure
    {
        "caxis_type", T_INT,
        offsetof(Event, e.caxis.type), 0,
        "..."
    },
    // Game controller button event structure
    {
        "cbutton_type", T_INT,
        offsetof(Event, e.cbutton.type), 0,
        "..."
    },
    //  Controller device event structure
    {
        "cdevice_type", T_INT,
        offsetof(Event, e.cdevice.type), 0,
        "..."
    },
    // The "quit requested" event
    {
        "quit_type", T_INT,
        offsetof(Event, e.quit.type), 0,
        "..."
    },
    // A user-defined event type
    {
        "user_type", T_INT,
        offsetof(Event, e.user.type), 0,
        "..."
    },
    // A video driver dependent system event
    {
        "syswm_type", T_INT,
        offsetof(Event, e.syswm.type), 0,
        "..."
    },
    // Touch finger event structure
    {
        "tfinger_type", T_INT,
        offsetof(Event, e.tfinger.type), 0,
        "..."
    },
    //  Multiple Finger Gesture Event
    {
        "mgesture_type", T_INT,
        offsetof(Event, e.mgesture.type), 0,
        "..."
    },
    //
    {
        "dgesture_type", T_INT,
        offsetof(Event, e.dgesture.type), 0,
        "..."
    },
    // An event used to request a file open by the system
    {
        "drop_type", T_INT,
        offsetof(Event, e.drop.type), 0,
        "..."
    },
    {
        "drop_timestamp", T_INT,
        offsetof(Event, e.drop.timestamp), 0,
        "..."
    },
    {
        "drop_file", T_STRING,
        offsetof(Event, e.drop.file), 0,
        "..."
    },
    {NULL}
};

static int
Event_init(Event* self, PyObject* args, PyObject* kwargs) {
    return 0;
}

static void
Event_dealloc(Event* self) {
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyGetSetDef Event_getsetters[] = {
    {NULL}
};

static PyMethodDef Event_methods[] = {
    {NULL}
};

PyTypeObject EventType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "wutu._events.Event",
    sizeof(Event),
    0,                         /* tp_itemsize */
    (destructor)Event_dealloc,
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
    "...",
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Event_methods,
    Event_members,
    Event_getsetters,
    0,
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Event_init,
    0,                         /* tp_alloc */
    (newfunc)PyType_GenericNew
};
