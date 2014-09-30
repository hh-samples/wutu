#include "font.h"

static FT_Error error;

static FT_Library freetype;

int
Font_Init() {
    error = FT_Init_FreeType(&freetype);
    return error;
}

unsigned char*
unpack_bitmap_mono(FT_Bitmap bitmap) {
    unsigned char* buffer = (unsigned char*)malloc(bitmap.width * bitmap.rows);
    unsigned char* dist = buffer;
    for (int row = 0; row < bitmap.rows; row++)
    {
        for (int p = 0; p < bitmap.pitch; p++)
        {
            unsigned char c = bitmap.buffer[p + row * bitmap.pitch];
            for( int b = 0; b < 8; b++)
            {
                if (b + p * 8 < bitmap.width)
                {
                    int v = (c&0x80) >> 7;
                    *(dist++) = (unsigned char)v * 255;
                    c <<= 1;
                }
            }
        }
    }
    return buffer;
}

void
unsafe_blit_alpha(unsigned char* pixels, int px, int py, int image_width, unsigned char* alpha, int a_w, int a_h) {
    int components = 4; // RGBA
    for (int y = 0; y < a_h; y++)
    {
        unsigned char* cur = pixels + (y + py) * image_width * components;
        for (int x = 0; x < a_w; x++)
        {
            // A index == 3
            *(cur + 3 + (x + px) * components) = *(alpha + x + y * a_w);
        }
    }
}

FontGlyphPixmap*
load_pixmap(Font* self, int unicode) {
    FontGlyphPixmap* pixmap;
    PyObject* capsule;

    FT_GlyphSlot slot = self->face->glyph;

    PyObject* key = PyLong_FromLong(unicode);
    capsule = PyDict_GetItem(self->glyph_cache, key);
    if (capsule != NULL)
    {
        return (FontGlyphPixmap*)PyCapsule_GetPointer(capsule, NULL);
    }
    pixmap = (FontGlyphPixmap*)malloc(sizeof(FontGlyphPixmap));
    if(FT_Load_Char(self->face, unicode, FT_LOAD_RENDER))
    {
        return NULL;
    }
    pixmap->width = slot->bitmap.width;
    pixmap->height = slot->bitmap.rows;
    pixmap->advance_x = (slot->advance.x >> 6);
    pixmap->y_offset = (self->face->size->metrics.ascender >> 6) - slot->bitmap_top;
    pixmap->x_offset = (slot->metrics.horiBearingX >> 6);

    if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
    {
        pixmap->buffer = unpack_bitmap_mono(slot->bitmap);
    }
    else
    {
        pixmap->buffer = (unsigned char*)malloc(pixmap->width * pixmap->height);
        for(int i = 0; i < pixmap->width * pixmap->height; i++)
        {
            pixmap->buffer[i] = slot->bitmap.buffer[i];
        }
    }
    capsule = PyCapsule_New(pixmap, NULL, NULL);
    PyDict_SetItem(self->glyph_cache, key, capsule);
    Py_DECREF(key);
    return pixmap;
}

FT_Error
Font_text_size(Font* self, PyObject* text, int* width, int* height) {
    FontGlyphPixmap* pixmap;
    int unicode;
    int text_length;
    int line_width = 0;
    int lines = 1;
    text_length = PyUnicode_GetLength(text);
    *height = 0;
    *width = 0;
    for (int index = 0; index < text_length; index++)
    {
        unicode = PyUnicode_ReadChar(text, index);
        if (unicode == UNICODE_BOM_NATIVE || unicode == UNICODE_BOM_SWAPPED)
        {
            continue;
        }
        if (unicode == UNICODE_NEW_LINE)
        {
            lines++;
            if (line_width > *width)
            {
                *width = line_width;
            }
            line_width = 0;
            continue;
        }
        pixmap = load_pixmap(self, unicode);
        if (pixmap == NULL)
        {
            return -1;
        }
        line_width += pixmap->advance_x;
    }
    if (line_width > *width)
    {
        *width = line_width;
    }
    *height = lines * (self->face->size->metrics.height >> 6);
    return FT_Err_Ok;
}

static PyObject*
Font_load(Font* self, PyObject* args) {
    char* path;
    int size;
    if (! PyArg_ParseTuple(args, "si", &path, &size))
    {
        return NULL;
    }
    error = FT_New_Face(freetype, path, 0, &self->face);
    if (error)
    {
        PyErr_SetString(PyExc_RuntimeError, Font_GetError());
        return NULL;
    }
    if (FT_IS_SCALABLE(self->face))
    {
        error = FT_Set_Char_Size(self->face, 0, size * 64, 0, 0);
        if (error)
        {
            PyErr_SetString(PyExc_RuntimeError, Font_GetError());
            return NULL;
        }
    }
    else
    {
        if (size >= self->face->num_fixed_sizes)
        {
            size = self->face->num_fixed_sizes - 1;
        }
        error = FT_Set_Pixel_Sizes(
            self->face,
            self->face->available_sizes[size].width,
            self->face->available_sizes[size].height
        );
        if (error)
        {
            PyErr_SetString(PyExc_RuntimeError, Font_GetError());
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

static PyObject*
Font_render_text(Font* self, PyObject* args, PyObject* kwargs) {
    PyObject* text;
    PyObject* attributes;
    PyObject* pixels;
    int text_length, image_width = 1, image_height = 1, image_components = 4;
    int unicode;
    unsigned char* data;
    FT_GlyphSlot slot = self->face->glyph;
    FontGlyphPixmap* pixmap;
    if (! PyArg_ParseTuple(args, "O", &text)) {
        return NULL;
    }
    int text_w = 0, text_h = 0;
    error = Font_text_size(self, text, &text_w, &text_h);
    if (error != FT_Err_Ok) {
        PyErr_SetString(PyExc_RuntimeError, "Font_text_size");
        return NULL;
    }
    while (image_width < text_w) image_width *= 2;
    while (image_height < text_h) image_height *= 2;
    data = (unsigned char*)malloc(image_width * image_height * image_components);
    for(int i = 0; i < image_width * image_height; i++) {
        data[i * 4 + 0] = 255;
        data[i * 4 + 1] = 255;
        data[i * 4 + 2] = 255;
        data[i * 4 + 3] = 0;
    }
    int pen_x = 0, pen_y = 0;
    text_length = PyUnicode_GetLength(text);
    for (int index = 0; index < text_length; index++) {
        unicode = PyUnicode_ReadChar(text, index);
        if (unicode == UNICODE_BOM_NATIVE || unicode == UNICODE_BOM_SWAPPED) {
            continue;
        }
        if (unicode == UNICODE_NEW_LINE) {
            pen_y += (self->face->size->metrics.height >> 6);
            pen_x = 0;
            continue;
        }
        pixmap = load_pixmap(self, unicode);
        if (error) {
            PyErr_SetString(PyExc_RuntimeError, Font_GetError());
            return NULL;
        }
        int y = pen_y + pixmap->y_offset;
        int x = pen_x + pixmap->x_offset;
        unsafe_blit_alpha(data, x, y, image_width, pixmap->buffer, pixmap->width, pixmap->height);
        pen_x += pixmap->advance_x;
    }
    pixels = PyBytes_FromStringAndSize((const char*)data, image_width * image_height * image_components);
    free(data);
    if (pixels == NULL) {
        return NULL;
    }
    attributes = Py_BuildValue("Oiii", pixels, image_width, image_height, image_components);
    Py_DECREF(pixels);
    return attributes;
}

static PyObject*
Font_measure_text(Font* self, PyObject* args) {
    PyObject* text;
    int width, height;
    if (! PyArg_ParseTuple(args, "O", &text)) {
        return NULL;
    }
    error = Font_text_size(self, text, &width, &height);
    if (error) {
        PyErr_SetString(PyExc_RuntimeError, Font_GetError());
        return NULL;
    }
    return Py_BuildValue("ii", width, height);
}

static int
Font_init(Font* self, PyObject* args, PyObject* kwargs) {
    self->glyph_cache = PyDict_New();
    return 0;
}

static void
Font_dealloc(Font* self) {
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyGetSetDef Font_getsetters[] = {
    {NULL}
};

static PyMethodDef Font_methods[] = {
    {
        "load",
        (PyCFunction)Font_load,
        METH_VARARGS,
        "Loads an font from the file with the given file name."
    },
    {
        "measure_text",
        (PyCFunction)Font_measure_text,
        METH_VARARGS,
        "Returns an tuple object that contains the width and height of the specified text, in pixels."
    },
    {
        "render_text",
        (PyCFunction)Font_render_text,
        METH_VARARGS | METH_KEYWORDS,
        "..."
    },
    {NULL}
};

PyTypeObject FontType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_graphics.Font",
    sizeof(Font),
    0,                         /* tp_itemsize */
    (destructor)Font_dealloc,
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
    "The Font class specifies a font used for drawing text.",
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Font_methods,
    0,                         /* tp_members */
    Font_getsetters,
    0,
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Font_init,
    0,                         /* tp_alloc */
    (newfunc)PyType_GenericNew
};

char*
Font_GetError() {
    switch(error)
    {
        case FT_Err_Ok: return "no error";
        case FT_Err_Cannot_Open_Resource: return "cannot open resource";
        case FT_Err_Unknown_File_Format: return "unknown file format";
        case FT_Err_Invalid_File_Format: return "broken file";
        case FT_Err_Invalid_Version: return "invalid FreeType version";
        case FT_Err_Lower_Module_Version: return "module version is too low";
        case FT_Err_Invalid_Argument: return "invalid argument";
        case FT_Err_Unimplemented_Feature: return "unimplemented feature";
        case FT_Err_Invalid_Table: return "broken table";
        case FT_Err_Invalid_Offset: return "broken offset within table";
        case FT_Err_Array_Too_Large: return "array allocation size too large";
        case FT_Err_Missing_Module: return "missing module";
        case FT_Err_Missing_Property: return "missing property";
        case FT_Err_Invalid_Glyph_Index: return "invalid glyph index";
        case FT_Err_Invalid_Character_Code: return "invalid character code";
        case FT_Err_Invalid_Glyph_Format: return "unsupported glyph image format";
        case FT_Err_Cannot_Render_Glyph: return "cannot render this glyph format";
        case FT_Err_Invalid_Outline: return "invalid outline";
        case FT_Err_Invalid_Composite: return "invalid composite glyph";
        case FT_Err_Too_Many_Hints: return "too many hints";
        case FT_Err_Invalid_Pixel_Size: return "invalid pixel size";
        case FT_Err_Invalid_Handle: return "invalid object handle";
        case FT_Err_Invalid_Library_Handle: return "invalid library handle";
        case FT_Err_Invalid_Driver_Handle: return "invalid module handle";
        case FT_Err_Invalid_Face_Handle: return "invalid face handle";
        case FT_Err_Invalid_Size_Handle: return "invalid size handle";
        case FT_Err_Invalid_Slot_Handle: return "invalid glyph slot handle";
        case FT_Err_Invalid_CharMap_Handle: return "invalid charmap handle";
        case FT_Err_Invalid_Cache_Handle: return "invalid cache manager handle";
        case FT_Err_Invalid_Stream_Handle: return "invalid stream handle";
        case FT_Err_Too_Many_Drivers: return "too many modules";
        case FT_Err_Too_Many_Extensions: return "too many extensions";
        case FT_Err_Out_Of_Memory: return "out of memory";
        case FT_Err_Unlisted_Object: return "unlisted object";
        case FT_Err_Cannot_Open_Stream: return "cannot open stream";
        case FT_Err_Invalid_Stream_Seek: return "invalid stream seek";
        case FT_Err_Invalid_Stream_Skip: return "invalid stream skip";
        case FT_Err_Invalid_Stream_Read: return "invalid stream read";
        case FT_Err_Invalid_Stream_Operation: return "invalid stream operation";
        case FT_Err_Invalid_Frame_Operation: return "invalid frame operation";
        case FT_Err_Nested_Frame_Access: return "nested frame access";
        case FT_Err_Invalid_Frame_Read: return "invalid frame read";
        case FT_Err_Raster_Uninitialized: return "raster uninitialized";
        case FT_Err_Raster_Corrupted: return "raster corrupted";
        case FT_Err_Raster_Overflow: return "raster overflow";
        case FT_Err_Raster_Negative_Height: return "negative height while rastering";
        case FT_Err_Too_Many_Caches: return "too many registered caches";
        case FT_Err_Invalid_Opcode: return "invalid opcode";
        case FT_Err_Too_Few_Arguments: return "too few arguments";
        case FT_Err_Stack_Overflow: return "stack overflow";
        case FT_Err_Code_Overflow: return "code overflow";
        case FT_Err_Bad_Argument: return "bad argument";
        case FT_Err_Divide_By_Zero: return "division by zero";
        case FT_Err_Invalid_Reference: return "invalid reference";
        case FT_Err_Debug_OpCode: return "found debug opcode";
        case FT_Err_ENDF_In_Exec_Stream: return "found ENDF opcode in execution stream";
        case FT_Err_Nested_DEFS: return "nested DEFS";
        case FT_Err_Invalid_CodeRange: return "invalid code range";
        case FT_Err_Execution_Too_Long: return "execution context too long";
        case FT_Err_Too_Many_Function_Defs: return "too many function definitions";
        case FT_Err_Too_Many_Instruction_Defs: return "too many instruction definitions";
        case FT_Err_Table_Missing: return "SFNT font table missing";
        case FT_Err_Horiz_Header_Missing: return "horizontal header (hhea) table missing";
        case FT_Err_Locations_Missing: return "locations (loca) table missing";
        case FT_Err_Name_Table_Missing: return "name table missing";
        case FT_Err_CMap_Table_Missing: return "character map (cmap) table missing";
        case FT_Err_Hmtx_Table_Missing: return "horizontal metrics (hmtx) table missing";
        case FT_Err_Post_Table_Missing: return "PostScript (post) table missing";
        case FT_Err_Invalid_Horiz_Metrics: return "invalid horizontal metrics";
        case FT_Err_Invalid_CharMap_Format: return "invalid character map (cmap) format";
        case FT_Err_Invalid_PPem: return "invalid ppem value";
        case FT_Err_Invalid_Vert_Metrics: return "invalid vertical metrics";
        case FT_Err_Could_Not_Find_Context: return "could not find context";
        case FT_Err_Invalid_Post_Table_Format: return "invalid PostScript (post) table format";
        case FT_Err_Invalid_Post_Table: return "invalid PostScript (post) table";
        case FT_Err_Syntax_Error: return "opcode syntax error";
        case FT_Err_Stack_Underflow: return "argument stack underflow";
        case FT_Err_Ignore: return "ignore";
        case FT_Err_No_Unicode_Glyph_Name: return "no Unicode glyph name found";
        case FT_Err_Glyph_Too_Big: return "glyph to big for hinting";
        case FT_Err_Missing_Startfont_Field: return "`STARTFONT` field missing";
        case FT_Err_Missing_Font_Field: return "`FONT` field missing";
        case FT_Err_Missing_Size_Field: return "`SIZE` field missing";
        case FT_Err_Missing_Fontboundingbox_Field: return "`FONTBOUNDINGBOX` field missing";
        case FT_Err_Missing_Chars_Field: return "`CHARS` field missing";
        case FT_Err_Missing_Startchar_Field: return "`STARTCHAR` field missing";
        case FT_Err_Missing_Encoding_Field: return "`ENCODING` field missing";
        case FT_Err_Missing_Bbx_Field: return "`BBX` field missing";
        case FT_Err_Bbx_Too_Big: return "`BBX` too big";
        case FT_Err_Corrupted_Font_Header: return "font header corrupted or missing fields";
        case FT_Err_Corrupted_Font_Glyphs: return "font glyphs corrupted or missing fields";
    }
    return "unknown error";
}
