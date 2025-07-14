import types
import sys

# Stub heavy modules so engine imports without optional deps
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('engine.renderers', types.ModuleType('engine.renderers'))
sys.modules.setdefault('engine.renderers.shader', types.ModuleType('engine.renderers.shader'))
mesh_mod = types.ModuleType('engine.mesh_utils')
mesh_mod.Mesh = type('Mesh', (), {})
sys.modules.setdefault('engine.mesh_utils', mesh_mod)
dummy_renderers = sys.modules['engine.renderers']
class DummyRenderer:
    def __init__(self, w=640, h=480, t="SAGE"):
        self.width = w
        self.height = h
        self.title = t
        self.widget = None

    def clear(self, color=(0, 0, 0)):
        pass
    def draw_scene(self, scene, camera=None, **kw):
        pass
    def present(self):
        pass
    def close(self):
        pass

dummy_renderers.Renderer = DummyRenderer
dummy_renderers.Shader = object
dummy_renderers.get_renderer = lambda name: DummyRenderer
game_mod = types.ModuleType('engine.entities.game_object')
class GameObject:
    pass
game_mod.GameObject = GameObject
class SpriteCache:
    def __init__(self, limit=32):
        self.limit = limit
    def get(self, path):
        return None
    def put(self, path, img):
        pass
    def clear(self):
        pass
game_mod.SpriteCache = SpriteCache
def set_sprite_cache(cache):
    pass
game_mod.set_sprite_cache = set_sprite_cache
sys.modules.setdefault('engine.entities.game_object', game_mod)

cam_mod = types.ModuleType('engine.core.camera')
class Camera(GameObject):
    def __init__(self, *a, **k):
        pass
cam_mod.Camera = Camera
sys.modules.setdefault('engine.core.camera', cam_mod)

from engine.inputs import NullInput  # noqa: E402

from engine.core.engine import Engine, ENGINE_VERSION  # noqa: E402
from engine.core.scenes.scene import Scene  # noqa: E402


def test_engine_metadata_defaults():
    eng = Engine(scene=Scene(with_defaults=False), renderer=DummyRenderer, input_backend=NullInput)
    assert eng.metadata["version"] == ENGINE_VERSION


def test_engine_metadata_update():
    eng = Engine(scene=Scene(with_defaults=False), renderer=DummyRenderer, input_backend=NullInput, metadata={"level": 1})
    assert eng.metadata["version"] == ENGINE_VERSION
    assert eng.metadata["level"] == 1
