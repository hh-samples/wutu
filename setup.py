import sys
import setuptools

if sys.platform == 'win32':
    from distutils import msvccompiler
    # compile C files as C++ code (/TP)
    msvccompiler.MSVCCompiler._c_extensions = []
    msvccompiler.MSVCCompiler._cpp_extensions.append('.c')

INCLUDE_DIRECTORIES = [
    '/usr/local/include'
]

LIB_DIRECTORIES = [
    '/usr/local/lib'
]

_events = setuptools.Extension(
    '_events',
    include_dirs=INCLUDE_DIRECTORIES,
    library_dirs=LIB_DIRECTORIES,
    libraries=[
        'SDL2'
    ],
    sources=[
        'extensions/event.c',
        'extensions/_events.c'
    ],
)

_graphics = setuptools.Extension(
    '_graphics',
    include_dirs=INCLUDE_DIRECTORIES,
    library_dirs=LIB_DIRECTORIES,
    libraries=[
        'opengl32',
        'glu32',
        'SDL2',
        'freetype2412'
    ],
    sources=[
        'extensions/window.c',
        'extensions/renderer.c',
        'extensions/font.c',
        'extensions/_graphics.c'
    ],
)

setuptools.setup(
    name='wutu',
    version='0.1.1',
    license='MIT',
    packages=['wutu'],
    ext_package='wutu',
    ext_modules=[
        _events,
        _graphics
    ]
)
