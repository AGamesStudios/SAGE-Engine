import sys
import types
import ctypes


def _stub_sdl(monkeypatch, calls):
    mod = types.SimpleNamespace()
    mod.SDL_INIT_VIDEO = 0
    mod.SDL_BLENDMODE_BLEND = 1
    mod.SDL_TEXTUREACCESS_STATIC = 0
    mod.SDL_PIXELFORMAT_RGBA8888 = 0
    mod.SDL_WINDOWPOS_CENTERED = 0
    mod.SDL_FLIP_HORIZONTAL = 1
    mod.SDL_FLIP_VERTICAL = 2
    mod.SDL_Renderer = object
    mod.SDL_Rect = lambda x, y, w, h: (x, y, w, h)
    mod.SDL_Point = type('P', (ctypes.Structure,), {'_fields_': [('x', ctypes.c_int), ('y', ctypes.c_int)]})
    mod.SDL_GetError = lambda: b""
    mod.SDL_Init = lambda flag: 0
    mod.SDL_CreateWindow = lambda *a, **k: object()
    mod.SDL_CreateRenderer = lambda *a, **k: object()
    mod.SDL_SetRenderDrawBlendMode = lambda *a, **k: None
    mod.SDL_DestroyRenderer = lambda r: None
    mod.SDL_DestroyWindow = lambda w: None
    mod.SDL_Quit = lambda: None
    mod.SDL_CreateTexture = lambda *a, **k: ctypes.c_void_p(1)
    mod.SDL_UpdateTexture = lambda *a, **k: None
    mod.SDL_SetTextureBlendMode = lambda *a, **k: None
    mod.SDL_DestroyTexture = lambda *a, **k: None
    mod.SDL_RenderPresent = lambda *a, **k: None
    def copyex(renderer, tex, src, dst, angle, center, flip):
        calls.setdefault('rects', []).append(dst)
    mod.SDL_RenderCopyEx = copyex
    monkeypatch.setitem(sys.modules, 'sdl2', mod)
    monkeypatch.setitem(
        sys.modules,
        'engine.renderers.opengl.drawing',
        types.SimpleNamespace(parse_color=lambda c: (255, 0, 0, 255)),
    )
    gl = types.ModuleType('OpenGL.GL')
    gl.GL_VERTEX_SHADER = 0
    gl.GL_FRAGMENT_SHADER = 0
    gl.glUseProgram = lambda *a, **k: None
    gl.glGetUniformLocation = lambda *a, **k: -1
    gl.glUniform1f = gl.glUniform2f = gl.glUniform3f = gl.glUniform4f = lambda *a, **k: None
    shaders_mod = types.ModuleType('OpenGL.GL.shaders')
    shaders_mod.compileProgram = lambda *a, **k: 1
    shaders_mod.compileShader = lambda *a, **k: 1
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', gl)
    monkeypatch.setitem(sys.modules, 'OpenGL.GL.shaders', shaders_mod)
    if 'engine.renderers.sdl_renderer' in sys.modules:
        del sys.modules['engine.renderers.sdl_renderer']
    pil = types.ModuleType('PIL')
    img_mod = types.ModuleType('PIL.Image')
    class DummyImage:
        width = 3
        height = 3
        size = (3, 3)
        def convert(self, mode):
            return self
        def transpose(self, m):
            return self
        def tobytes(self, *a, **k):
            return b"\xff" * 9 * 4
    img_mod.Image = DummyImage
    img_mod.Transpose = types.SimpleNamespace(FLIP_TOP_BOTTOM=0)
    img_mod.open = lambda p: img_mod.Image()
    img_mod.frombytes = lambda *a, **k: img_mod.Image()
    monkeypatch.setitem(sys.modules, 'PIL', pil)
    monkeypatch.setitem(sys.modules, 'PIL.Image', img_mod)


def test_flip_does_not_shift(monkeypatch):
    calls = {}
    _stub_sdl(monkeypatch, calls)
    from sage_engine.renderers.sdl_renderer import SDLRenderer

    class DummyObj:
        class Img:
            width = 3
            height = 3
            size = (3, 3)
            def tobytes(self, *a, **k):
                return b"\xff" * 9 * 4
            def transpose(self, t):
                return self
        def __init__(self):
            self.image = DummyObj.Img()
            self.width = 3
            self.height = 3
            self.scale_x = 1.0
            self.scale_y = 1.0
            self.x = 10
            self.y = 20
            self.pivot_x = 0.5
            self.pivot_y = 0.5
            self.angle = 0.0
            self.flip_x = False
            self.flip_y = False

    obj = DummyObj()

    r = SDLRenderer(32, 32, 't')
    r._draw_sprite(obj)
    rect_no = calls['rects'][-1]

    obj.flip_x = True
    r._draw_sprite(obj)
    rect_flip_x = calls['rects'][-1]
    assert rect_flip_x == rect_no

    obj.flip_x = False
    obj.flip_y = True
    r._draw_sprite(obj)
    rect_flip_y = calls['rects'][-1]
    assert rect_flip_y == rect_no
