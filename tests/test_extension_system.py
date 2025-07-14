import types
import sys
import asyncio
import importlib.machinery

# Stub heavy modules so engine.core.engine imports without optional deps
qtcore = types.ModuleType('PyQt6.QtCore')
qtcore.Qt = object()


class _QObj:
    pass

qtcore.QObject = _QObj
sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
sys.modules.setdefault('PyQt6.QtCore', qtcore)
sys.modules.setdefault('PyQt6.QtWidgets', types.ModuleType('PyQt6.QtWidgets'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
render_mod = types.ModuleType('engine.renderers')


class _Renderer:
    pass
render_mod.Renderer = _Renderer
render_mod.Shader = object
def get_renderer(name):
    return None
render_mod.get_renderer = get_renderer
render_mod.__spec__ = importlib.machinery.ModuleSpec('engine.renderers', None)
render_mod.__package__ = 'engine'
sys.modules['engine.renderers'] = render_mod
sys.modules['engine.renderers.shader'] = types.ModuleType('engine.renderers.shader')
mesh_mod = types.ModuleType('engine.mesh_utils')


class _Mesh:
    pass
mesh_mod.Mesh = _Mesh
mesh_mod.create_square_mesh = lambda *a, **k: None
mesh_mod.create_triangle_mesh = lambda *a, **k: None
mesh_mod.create_circle_mesh = lambda *a, **k: None
mesh_mod.__package__ = 'engine'
mesh_mod.__spec__ = importlib.machinery.ModuleSpec('engine.mesh_utils', None)
sys.modules['engine.mesh_utils'] = mesh_mod
sys.modules['PIL'] = types.ModuleType('PIL')
sys.modules['PIL.Image'] = types.ModuleType('PIL.Image')

game_mod = types.ModuleType('engine.entities.game_object')
class GameObject:
    name: str = 'GameObject'
    z: int = 0
    def __init__(self, *args, **kwargs):
        for k, v in kwargs.items():
            setattr(self, k, v)
    def update(self, dt):
        pass
    def draw(self, surf):
        pass
sys.modules['engine.entities.game_object'] = game_mod
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

cam_mod = types.ModuleType('engine.core.camera')
class Camera(GameObject):
    x: float = 0.0
    y: float = 0.0
    width: int = 0
    height: int = 0
    active: bool = False
    def __init__(self, x=0.0, y=0.0, width=0, height=0, *, active=False, **kw):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.active = active
cam_mod.Camera = Camera
sys.modules['engine.core.camera'] = cam_mod

import importlib  # noqa: E402
if 'engine.core.scenes.scene' in sys.modules:
    importlib.reload(sys.modules['engine.core.scenes.scene'])

from engine.core.engine import Engine  # noqa: E402
from engine.core.extensions import EngineExtension  # noqa: E402
from engine.core.settings import EngineSettings  # noqa: E402
from engine.inputs import InputBackend  # noqa: E402
from engine.renderers import Renderer  # noqa: E402

class DummyRenderer(Renderer):
    def __init__(self, width, height, title):
        super().__init__()
        self.width = width
        self.height = height
        self.title = title
        self.widget = None
        self.keep_aspect = True
        self.background = (0, 0, 0)
    def clear(self, color=(0,0,0)):
        pass
    def draw_scene(self, scene, camera=None, **kwargs):
        pass
    def present(self):
        pass
    def close(self):
        pass

class DummyInput(InputBackend):
    def __init__(self, widget=None):
        pass
    def poll(self):
        pass
    def is_key_down(self, key):
        return False
    def is_button_down(self, button):
        return False
    def shutdown(self):
        pass

class DummyExt(EngineExtension):
    def __init__(self):
        self.started = False
        self.updated = []
        self.stopped = False

    def start(self, eng):
        self.started = True

    def update(self, eng, dt):
        self.updated.append(dt)

    def stop(self, eng):
        self.stopped = True


def test_extension_hooks():
    settings = EngineSettings(renderer=DummyRenderer, input_backend=DummyInput)
    eng = Engine(settings=settings)
    ext = DummyExt()
    eng.add_extension(ext)
    eng.update(0.1)
    eng.shutdown()
    assert ext.started
    assert ext.updated == [0.1]
    assert ext.stopped


class AsyncExt(EngineExtension):
    async def update(self, eng, dt):
        eng.flag = dt


def test_async_extension_update():
    settings = EngineSettings(renderer=DummyRenderer, input_backend=DummyInput)
    eng = Engine(settings=settings, asyncio_events=True)
    ext = AsyncExt()
    eng.flag = None
    eng.add_extension(ext)
    eng.logic_active = True
    eng.step()
    assert eng.flag is not None


class StartStopExt(EngineExtension):
    def __init__(self):
        self.started = False
        self.stopped = False

    async def start(self, eng):
        self.started = True

    async def stop(self, eng):
        self.stopped = True


def test_async_extension_start_stop():
    settings = EngineSettings(renderer=DummyRenderer, input_backend=DummyInput)
    eng = Engine(settings=settings, asyncio_events=True)
    ext = StartStopExt()
    eng.add_extension(ext)
    eng.remove_extension(ext)
    assert ext.started and ext.stopped


class AsyncObj(GameObject):
    async def update(self, dt):
        self.flag = dt


def test_async_object_update():
    from engine.core.scenes.scene import Scene

    scene = Scene(with_defaults=False)
    obj = AsyncObj()
    scene.add_object(obj)
    settings = EngineSettings(
        renderer=DummyRenderer,
        input_backend=DummyInput,
        scene=scene,
    )
    eng = Engine(settings=settings, asyncio_events=True)
    eng.logic_active = True
    asyncio.run(eng.step_async())
    assert getattr(obj, "flag", None) is not None


def teardown_module(module):
    import sys
    for name in [
        'engine.renderers',
        'engine.renderers.shader',
        'engine.mesh_utils',
        'engine.entities.game_object',
        'engine.core.camera',
        'PyQt6',
        'PyQt6.QtCore',
        'PyQt6.QtWidgets',
        'OpenGL',
        'OpenGL.GL',
        'PIL',
        'PIL.Image',
    ]:
        sys.modules.pop(name, None)
