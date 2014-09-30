import os
import unittest

import wutu.graphics


def provide_image(relative_path):
    def decorator(test):
        def function(self, *args):
            try:
                path = os.path.join(os.path.dirname(__file__), relative_path)
                args = self, wutu.graphics.Image.load(path)
            except Exception as exception:
                self.skipTest(exception)
            return test(*args)
        return function
    return decorator


class GraphicsTestCase(unittest.TestCase):

    def assertImageEqual(self, expected, actual):
        if expected.pixels != actual.pixels:
            actual.source = expected.source.replace('expected', 'actual')
            actual.save(actual.source)
            raise AssertionError('{} != {}'.format(expected.source, actual.source))


class TestFont(GraphicsTestCase):

    def test_measure_text(self):
        fira = wutu.graphics.Font()
        fira.load('data/assets/fonts/fira/FiraSans-Regular.ttf', 16)
        width, height = fira.measure_text('Beautiful is better than ugly.')
        self.assertEqual(208, width)
        self.assertEqual(22, height)

    def test_measure_text_monospaced(self):
        terminus = wutu.graphics.Font()
        terminus.load('data/assets/fonts/terminus/ter-u12n.pcf.gz', 12)
        width, height = terminus.measure_text('Explicit is better than implicit.')
        self.assertEqual(198, width)
        self.assertEqual(12, height)

    def test_measure_multiline_text(self):
        fira = wutu.graphics.Font()
        fira.load('data/assets/fonts/fira/FiraSans-Regular.ttf', 16)
        lines = [
            'Simple is better than complex.',
            'Complex is better than complicated.'
        ]
        width, height = fira.measure_text('\n'.join(lines))
        self.assertEqual(268, width)
        self.assertEqual(44, height)

    def test_measure_multiline_text_monospaced(self):
        terminus = wutu.graphics.Font()
        terminus.load('data/assets/fonts/terminus/ter-u12n.pcf.gz', 12)
        lines = [
            'Now is better than never.',
            'Although never is often better than *right* now.'
        ]
        width, height = terminus.measure_text('\n'.join(lines))
        self.assertEqual(288, width)
        self.assertEqual(24, height)

    @provide_image('data/expected/test_render_multiline_text.png')
    def test_render_multiline_text(self, expected_image):
        fira = wutu.graphics.Font()
        fira.load('data/assets/fonts/fira/FiraSans-Regular.ttf', 16)
        lines = [
            'hello',
            'привет',
            'γεια σας',
            'halló',
            'cześć'
        ]
        self.assertImageEqual(expected_image, fira.render_text('\n'.join(lines)))

    @provide_image('data/expected/test_render_multiline_text_monospaced.png')
    def test_render_multiline_text_monospaced(self, expected_image):
        terminus = wutu.graphics.Font()
        terminus.load('data/assets/fonts/terminus/ter-u12n.pcf.gz', 12)
        lines = [
            'hello',
            'привет',
            'γεια σας',
            'halló',
            'cześć'
        ]
        self.assertImageEqual(expected_image, terminus.render_text('\n'.join(lines)))


class TestRenderer(GraphicsTestCase):

    def setUp(self):
        self.window = wutu.graphics.Window(128, 128, visible=False)

    @provide_image('data/expected/test_clear.png')
    def test_clear(self, expected):
        renderer = wutu.graphics.Renderer(self.window)
        renderer.clear('#7bc0fd')
        self.assertImageEqual(expected, renderer.present())

    @provide_image('data/expected/test_draw_polygon.png')
    def test_draw_polygon(self, expected):
        renderer = wutu.graphics.Renderer(self.window)
        renderer.clear('#777777')
        renderer.set_color('#f0ad4e')
        coordinates = (
            32, 32,
            32, 96,
            64, 112,
            96, 96,
            96, 32,
        )
        renderer.draw_polygon(coordinates)
        self.assertImageEqual(expected, renderer.present())

    @provide_image('data/expected/test_draw_rectangle.png')
    def test_draw_rectangle(self, expected):
        renderer = wutu.graphics.Renderer(self.window)
        renderer.clear('#f7f7ef')
        renderer.set_color('#d9534f')
        renderer.draw_rectangle(10, 10, 80, 24)
        self.assertImageEqual(expected, renderer.present())

    @provide_image('data/expected/test_draw_line_loop.png')
    def test_draw_line_loop(self, expected):
        renderer = wutu.graphics.Renderer(self.window)
        renderer.clear('#777777')
        renderer.set_color('#f0ad4e')
        coordinates = (
            32, 32,
            32, 96,
            64, 112,
            96, 96,
            96, 32,
        )
        renderer.draw_line_loop(coordinates)
        self.assertImageEqual(expected, renderer.present())

    @provide_image('data/expected/test_draw_texture.png')
    def test_draw_texture(self, expected):
        renderer = wutu.graphics.Renderer(self.window)
        image = wutu.graphics.Image.load('data/assets/images/grid.png')
        texture = renderer.create_texture(image)
        renderer.draw_texture(texture)
        self.assertImageEqual(expected, renderer.present())

if __name__ == '__main__':
    unittest.main()