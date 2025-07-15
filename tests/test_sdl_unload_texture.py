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
    def create_texture(*a, **k):
        calls['create'] += 1
        return ctypes.c_void_p(1)
    mod.SDL_CreateTexture = create_texture
    mod.SDL_UpdateTexture = lambda *a, **k: None
    mod.SDL_SetTextureBlendMode = lambda *a, **k: None
    def destroy_texture(t):
        calls['destroy'] += 1
    mod.SDL_DestroyTexture = destroy_texture
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
    monkeypatch.setitem(sys.modules, 'sdl2', mod)
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


def test_unload_texture(monkeypatch):
    calls = {'create': 0, 'destroy': 0}
    _stub_sdl(monkeypatch, calls)
    from sage_engine.renderers.sdl_renderer import SDLRenderer

    class Img:
        width = 1
        height = 1
        def tobytes(self, *a, **k):
            return b"\xff" * 4
        def transpose(self, t):
            return self

    obj = types.SimpleNamespace(image=Img(), smooth=True)
    r = SDLRenderer(2, 2, 't')
    r._get_texture(obj)
    assert calls['create'] == 1
    r.unload_texture(obj)
    assert calls['destroy'] == 1
    assert r.textures == {}
