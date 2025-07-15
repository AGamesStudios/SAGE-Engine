import sys
import types
import ctypes

from sage_engine.entities.tile_map import TileMap


def _stub_sdl(monkeypatch):
    mod = types.SimpleNamespace()
    mod.SDL_INIT_VIDEO = 0
    mod.SDL_BLENDMODE_BLEND = 1
    mod.SDL_TEXTUREACCESS_TARGET = 1
    mod.SDL_TEXTUREACCESS_STATIC = 0
    mod.SDL_PIXELFORMAT_RGBA8888 = 0
    mod.SDL_WINDOWPOS_CENTERED = 0
    mod.SDL_Rect = lambda x, y, w, h: (x, y, w, h)
    mod.SDL_GetError = lambda: b""
    calls = {"texture": 0}
    mod.SDL_Init = lambda flag: 0
    mod.SDL_CreateWindow = lambda *a, **k: object()
    mod.SDL_CreateRenderer = lambda *a, **k: object()
    mod.SDL_SetRenderDrawBlendMode = lambda *a, **k: None
    mod.SDL_SetRenderTarget = lambda *a, **k: None
    mod.SDL_GetRenderTarget = lambda *a, **k: None
    mod.SDL_RenderClear = lambda *a, **k: None
    mod.SDL_SetRenderDrawColor = lambda *a, **k: None
    mod.SDL_RenderFillRect = lambda *a, **k: None
    class DummySurf(ctypes.Structure):
        _fields_ = [('format', ctypes.c_void_p), ('pixels', ctypes.c_void_p)]
    def create_texture(*a, **k):
        calls["texture"] += 1
        return object()
    mod.SDL_CreateRGBSurfaceWithFormat = lambda *a, **k: ctypes.pointer(DummySurf())
    mod.SDL_MapRGBA = lambda fmt, r, g, b, a: 0
    mod.SDL_FillRect = lambda *a, **k: None
    mod.SDL_CreateTextureFromSurface = create_texture
    mod.SDL_FreeSurface = lambda s: None
    mod.SDL_CreateTexture = create_texture
    mod.SDL_SetTextureBlendMode = lambda *a, **k: None
    mod.SDL_UpdateTexture = lambda *a, **k: None
    mod.SDL_RenderCopy = lambda *a, **k: None
    mod.SDL_RenderPresent = lambda r: None
    mod.SDL_DestroyRenderer = lambda r: None
    mod.SDL_DestroyWindow = lambda w: None
    mod.SDL_Quit = lambda: None
    if 'engine.renderers.sdl_renderer' in sys.modules:
        del sys.modules['engine.renderers.sdl_renderer']
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
    monkeypatch.setitem(sys.modules, "sdl2", mod)
    # minimal Pillow stub
    pil = types.ModuleType('PIL')
    img_mod = types.ModuleType('PIL.Image')
    img_mod.Image = type('Image', (), {})
    img_mod.Transpose = types.SimpleNamespace(FLIP_TOP_BOTTOM=0)
    img_mod.open = lambda p: img_mod.Image()
    img_mod.frombytes = lambda *a, **k: img_mod.Image()
    img_mod.Image.transpose = lambda self, t: self
    monkeypatch.setitem(sys.modules, 'PIL', pil)
    monkeypatch.setitem(sys.modules, 'PIL.Image', img_mod)
    return calls


def test_tilemap_render_cached(monkeypatch):
    calls = _stub_sdl(monkeypatch)
    monkeypatch.setitem(sys.modules, 'engine.renderers.opengl.drawing', types.SimpleNamespace(parse_color=lambda c: (255,0,0,255)))
    from sage_engine.renderers.sdl_renderer import SDLRenderer

    tm = TileMap()
    tm.width = tm.height = 1
    tm.tile_width = tm.tile_height = 8
    tm.data = [1]
    tm.metadata = {'colors': {'1': '#ff0000'}}

    r = SDLRenderer(8, 8, 't')
    r._draw_map(tm)
    r._draw_map(tm)
    assert calls['texture'] == 1

