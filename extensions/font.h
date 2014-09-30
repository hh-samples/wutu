#ifndef FONT_H
#define FONT_H

#include <Python.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_IDS_H

#define UNICODE_BOM_NATIVE  0xFEFF
#define UNICODE_BOM_SWAPPED 0xFFFE
#define UNICODE_NEW_LINE    0x000A

int
Font_Init();

char*
Font_GetError();

typedef struct {
    unsigned char* buffer;
    int height;
    int width;
    int y_offset;
    int x_offset;
    int advance_x;
} FontGlyphPixmap;

typedef struct {
    PyObject_HEAD
    FT_Face face;
    PyObject* glyph_cache;
} Font;

extern PyTypeObject FontType;

#endif /* FONT_H */
