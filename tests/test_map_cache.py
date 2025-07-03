import sys
import types

from engine.entities.tile_map import TileMap


def _stub_sdl(monkeypatch):
    mod = types.SimpleNamespace()
    mod.SDL_INIT_VIDEO = 0
    mod.SDL_BLENDMODE_BLEND = 1
    mod.SDL_TEXTUREACCESS_TARGET = 1
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
    def create_texture(*a, **k):
        calls["texture"] += 1
        return object()
    mod.SDL_CreateTexture = create_texture
    mod.SDL_SetTextureBlendMode = lambda *a, **k: None
    mod.SDL_RenderCopy = lambda *a, **k: None
    mod.SDL_RenderPresent = lambda r: None
    mod.SDL_DestroyRenderer = lambda r: None
    mod.SDL_DestroyWindow = lambda w: None
    mod.SDL_Quit = lambda: None
    monkeypatch.setitem(sys.modules, "sdl2", mod)
    return calls


def test_tilemap_render_cached(monkeypatch):
    calls = _stub_sdl(monkeypatch)
    monkeypatch.setitem(sys.modules, 'engine.renderers.opengl.drawing', types.SimpleNamespace(parse_color=lambda c: (255,0,0,255)))
    from engine.renderers.sdl_renderer import SDLRenderer

    tm = TileMap()
    tm.width = tm.height = 1
    tm.tile_width = tm.tile_height = 8
    tm.data = [1]
    tm.metadata = {'colors': {'1': '#ff0000'}}

    r = SDLRenderer(8, 8, 't')
    r._draw_map(tm)
    r._draw_map(tm)
    assert calls['texture'] == 1

