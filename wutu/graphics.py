import os
from . import _graphics


def color_to_float_values(color):
    if isinstance(color, str):
        color = int(color.strip('#'), 16)
        r = (255 & (color >> 16)) / 255
        g = (255 & (color >> 8)) / 255
        b = (255 & (color)) / 255
        return r, g, b, 1.0
    return color


class Window(_graphics.Window):
    """Represents a program window in the windowing system."""

    def __init__(self, width, height, title='wutu', visible=True):
        super().__init__(width, height, title, visible)
        self.width = width
        self.height = height
        self._rendering_context = None

    @property
    def context(self):
        """Returns rendering context for a window."""
        if self._rendering_context is None:
            self._rendering_context = Renderer(self)
        return self._rendering_context


class Renderer(_graphics.Renderer):
    """Represents OpenGL 2D rendering context for a window."""

    def __init__(self, window):
        super().__init__(window)
        self.window = window

    def clear(self, color):
        """Clears the screen to specified color."""
        self._clear(*color_to_float_values(color))

    def create_texture(self, image):
        """Generates a texture from image."""
        texture = Texture()
        texture.id = self._generate_texture(image.pixels, image.width, image.height, image.components)
        texture.width = image.width
        texture.height = image.height
        return texture

    def draw_line_loop(self, coordinates, width=1.0, smooth=False):
        coordinates = coordinates + coordinates[:2]
        self.draw_line_strip(coordinates, width, smooth)

    def draw_line_strip(self, coordinates, width=1.0, smooth=False):
        if len(coordinates) % 2 != 0:
            raise ValueError('coordinates must be a sequence of float values (x0, y0, x1, y1 ...)')
        self._draw_line_strip(coordinates, width, smooth)

    def draw_rectangle(self, top, left, width, height, fill=True):
        coordinates = (
            left, top,
            left, top + height,
            left + width, top + height,
            left + width, top
        )
        self.draw_polygon(coordinates)

    def draw_polygon(self, coordinates):
        if len(coordinates) % 2 != 0:
            raise ValueError('coordinates must be a sequence of float values (x0, y0, x1, y1 ...)')
        self._draw_polygon(coordinates)

    def draw_texture(self, texture):
        coordinates = (
            0, 0,
            0 + texture.width, 0,
            0 + texture.width, 0 + texture.height,
            0, 0 + texture.height
        )
        texture_coordinates = (
            0, 0,
            1, 0,
            1, 1,
            0, 1
        )
        self._draw_textured_polygon(coordinates, texture_coordinates, texture.id)

    def set_color(self, color):
        self._set_color(*color_to_float_values(color))

    def translate(self, x_or_tuple, y=None):
        """Translates the coordinate system by the given offset."""
        if y is None:
            super().translate(*x_or_tuple)
        else:
            super().translate(x_or_tuple, y)

    def present(self):
        """Returns result of drawing operations as Image."""
        pixels = self._read_pixels(self.window.width, self.window.height)
        return Image(pixels, self.window.width, self.window.height, 3)


class Font(_graphics.Font):
    """The Font class specifies a font used for drawing text."""

    def render_text(self, text):
        image_attributes = super().render_text(text)
        return Image(*image_attributes)


class Texture:
    """Represents OpenGL texture generated from context."""

    def __init__(self):
        self.id = 0
        self.width = 0
        self.height = 0
        self.image_width = 0
        self.image_height = 0

    @property
    def factor(self):
        """Specifies values for the x and y ..."""
        return self.image_width / self.width, self.image_height / self.height


class Image:
    """Represents context-independent image object that allows direct access to the pixel data."""

    def __init__(self, pixels, width, height, components, source=''):
        self._pixels = pixels
        self._width = width
        self._height = height
        self._components = components
        self.source = source
        self.texture = 0

    @property
    def pixels(self):
        return self._pixels

    @property
    def width(self):
        return self._width

    @property
    def height(self):
        return self._height

    @property
    def components(self):
        return self._components

    @staticmethod
    def load(path):
        if not os.path.exists(path):
            raise ValueError("image file '{}' not found".format(path))
        attributes = _graphics.load_image_attributes_from_file(path)
        return Image(*attributes, source=path)

    def save(self, path, file_format='png'):
        """Saves image to file with specified format."""
        if file_format.lower() not in ['png']:
            raise ValueError("save to '{}' file format not supported".format(file_format))
        directory = os.path.dirname(path)
        if not os.path.exists(directory):
            os.makedirs(directory)
        _graphics.save_image_pixels_to_png_file(self._pixels, self._width, self._height, self._components, path)
