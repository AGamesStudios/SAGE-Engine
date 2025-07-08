import sys
import types
import os

# stub heavy modules so engine imports
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
img_mod = types.ModuleType('PIL.Image')
class DummyImg:
    def __init__(self, path):
        self.size = (1, 1)
        self.path = path
    def convert(self, mode):
        return self

def new(mode, size, color):
    return DummyImg('new')
img_mod.open = lambda path: DummyImg(path)
img_mod.Image = DummyImg
img_mod.new = new
sys.modules['PIL.Image'] = img_mod
sys.modules['PIL'].Image = img_mod
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))
mesh_mod = sys.modules['engine.mesh_utils']
mesh_mod.Mesh = type('Mesh', (), {})
gl_mod = sys.modules['OpenGL.GL']
gl_mod.GL_VERTEX_SHADER = 0
gl_mod.GL_FRAGMENT_SHADER = 0
gl_mod.glUseProgram = lambda *a, **k: None
gl_mod.glGetUniformLocation = lambda *a, **k: 0
gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
shaders_mod = sys.modules['OpenGL.GL.shaders']
shaders_mod.compileProgram = lambda *a, **k: 1
shaders_mod.compileShader = lambda *a, **k: 1

sys.modules.pop('engine.entities.game_object', None)
import engine.entities.game_object as go  # noqa: E402


def test_env_var_limits_cache(monkeypatch):
    monkeypatch.setenv('SAGE_IMAGE_CACHE_LIMIT', '1')
    go.set_sprite_cache(go.SpriteCache(int(os.getenv('SAGE_IMAGE_CACHE_LIMIT'))))
    monkeypatch.setattr('engine.core.resources.get_resource_path', lambda p: p)
    go.clear_image_cache()
    cache = go.get_sprite_cache()
    cache.clear()
    go.GameObject(image_path='a.png')
    assert len(cache._cache) == 1
    go.GameObject(image_path='b.png')
    assert len(cache._cache) == 1
    assert list(cache._cache.keys()) == ['b.png']


def test_engine_setting_updates_cache(monkeypatch):
    monkeypatch.delenv('SAGE_IMAGE_CACHE_LIMIT', raising=False)
    go.set_sprite_cache(go.SpriteCache(2))
    from engine.core.settings import EngineSettings
    from engine.core.engine import Engine
    from engine.renderers.null_renderer import NullRenderer
    from engine.inputs.null_input import NullInput
    from engine.core.scenes.scene import Scene

    settings = EngineSettings(image_cache_limit=1)
    eng = Engine(scene=Scene(with_defaults=False), renderer=NullRenderer, input_backend=NullInput, settings=settings)
    assert eng.sprite_cache.limit == 1
