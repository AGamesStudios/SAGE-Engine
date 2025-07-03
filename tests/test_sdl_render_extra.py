import sys
import types
import ctypes
from engine.entities.game_object import GameObject


def _stub_sdl(monkeypatch, calls):
    mod = types.SimpleNamespace()
    mod.SDL_INIT_VIDEO = 0
    mod.SDL_BLENDMODE_BLEND = 1
    mod.SDL_TEXTUREACCESS_STATIC = 0
    mod.SDL_PIXELFORMAT_RGBA8888 = 0
    mod.SDL_FLIP_HORIZONTAL = 1
    mod.SDL_FLIP_VERTICAL = 2
    mod.SDL_WINDOWPOS_CENTERED = 0
    mod.SDL_RENDERER_ACCELERATED = 1
    mod.SDL_RENDERER_PRESENTVSYNC = 2
    mod.SDL_Rect = lambda x, y, w, h: (x, y, w, h)
    class P(ctypes.Structure):
        _fields_ = [('x', ctypes.c_int), ('y', ctypes.c_int)]
    mod.SDL_Point = P
    mod.SDL_GetError = lambda: b""
    mod.SDL_Init = lambda flag: 0
    mod.SDL_CreateWindow = lambda *a, **k: object()
    def create_renderer(window, index, flags):
        calls['flags'] = flags
        return object()
    mod.SDL_CreateRenderer = create_renderer
    mod.SDL_SetRenderDrawBlendMode = lambda *a, **k: None
    mod.SDL_SetRenderTarget = lambda *a, **k: None
    mod.SDL_GetRenderTarget = lambda *a, **k: None
    mod.SDL_RenderClear = lambda *a, **k: None
    mod.SDL_SetRenderDrawColor = lambda *a, **k: None
    mod.SDL_RenderFillRect = lambda *a, **k: None
    def create_tex(*a, **k):
        calls['tex'] += 1
        return object()
    mod.SDL_CreateTexture = create_tex
    mod.SDL_UpdateTexture = lambda *a, **k: None
    mod.SDL_SetTextureBlendMode = lambda *a, **k: None
    def copyex(*a, **k):
        calls['copy'] += 1
    mod.SDL_RenderCopyEx = copyex
    mod.SDL_UpdateTexture = lambda *a, **k: None
    mod.SDL_RenderCopy = lambda *a, **k: None
    def lines(r, pts, n):
        calls['lines'] += 1
    mod.SDL_RenderDrawLines = lines
    mod.SDL_RenderCopy = lambda *a, **k: None
    mod.SDL_RenderPresent = lambda r: None
    mod.SDL_DestroyTexture = lambda t: None
    mod.SDL_DestroyRenderer = lambda r: None
    mod.SDL_DestroyWindow = lambda w: None
    mod.SDL_Quit = lambda: None
    monkeypatch.setitem(sys.modules, "sdl2", mod)
    if 'engine.renderers.sdl_renderer' in sys.modules:
        del sys.modules['engine.renderers.sdl_renderer']
    pil = types.ModuleType('PIL')
    img_mod = types.ModuleType('PIL.Image')
    img_mod.Image = type('Image', (), {})
    img_mod.Transpose = types.SimpleNamespace(FLIP_TOP_BOTTOM=0)
    img_mod.open = lambda p: img_mod.Image()
    img_mod.frombytes = lambda *a, **k: img_mod.Image()
    img_mod.Image.transpose = lambda self, t: self
    monkeypatch.setitem(sys.modules, 'PIL', pil)
    monkeypatch.setitem(sys.modules, 'PIL.Image', img_mod)


def test_sprite_and_mesh(monkeypatch):
    calls = {'tex':0,'copy':0,'lines':0}
    _stub_sdl(monkeypatch, calls)
    from engine.renderers.sdl_renderer import SDLRenderer
    class Img:
        width = 1
        height = 1
        def tobytes(self, *a, **k):
            return b"\xff" * 4
        def transpose(self, t):
            return self
    img = Img()
    obj = GameObject(image_path="")
    obj.image = img
    obj.width = obj.height = 1
    r = SDLRenderer(2,2,'t')
    r._draw_sprite(obj)
    class DummyMesh:
        vertices = [(0,0),(1,0),(1,1),(0,1)]
        indices = []
    obj.mesh = DummyMesh()
    r._draw_mesh(obj, obj.mesh)
    assert calls['copy'] == 1 and calls['lines'] == 1 and calls['tex'] >= 1


def test_vsync_flag(monkeypatch):
    calls = {'tex': 0, 'copy': 0, 'lines': 0, 'flags': 0}
    _stub_sdl(monkeypatch, calls)
    from engine.renderers.sdl_renderer import SDLRenderer
    SDLRenderer(2, 2, 't', vsync=True)
    assert calls['flags'] & 2
