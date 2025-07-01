import time
import types
import sys
import pytest

# Stub heavy modules so engine imports without optional deps
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('engine.renderers', types.ModuleType('engine.renderers'))
sys.modules.setdefault('engine.renderers.shader', types.ModuleType('engine.renderers.shader'))
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))
dummy_renderers = sys.modules['engine.renderers']
null_mod = types.ModuleType('engine.renderers.null_renderer')
class _Renderer: pass
dummy_renderers.Renderer = _Renderer
dummy_renderers.Shader = object
def get_renderer(name):
    return None
dummy_renderers.get_renderer = get_renderer

class NullRendererStub:
    def __init__(self, width=640, height=480, title="SAGE 2D"):
        self.width = width
        self.height = height
        self.title = title
        self.keep_aspect = True
        self.background = (0, 0, 0)
        self.widget = None
    def clear(self, color=(0, 0, 0)):
        pass
    def draw_scene(self, scene, camera=None, **kwargs):
        pass
    def present(self):
        pass
    def close(self):
        pass

dummy_renderers.null_renderer = null_mod
null_mod.NullRenderer = NullRendererStub

qtcore = types.ModuleType('PyQt6.QtCore')
qtcore.Qt = object()
class _QObj: pass
qtcore.QObject = _QObj
sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
sys.modules.setdefault('PyQt6.QtCore', qtcore)
sys.modules.setdefault('PyQt6.QtWidgets', types.ModuleType('PyQt6.QtWidgets'))
sys.modules.setdefault('PyQt6.QtOpenGLWidgets', types.ModuleType('PyQt6.QtOpenGLWidgets'))

game_mod = types.ModuleType('engine.entities.game_object')
class GameObject:
    def __init__(self, name="Obj"):
        self.name = name
        self.z = 0
game_mod.GameObject = GameObject
sys.modules.setdefault('engine.entities.game_object', game_mod)

cam_mod = types.ModuleType('engine.core.camera')
class Camera(GameObject):
    def __init__(self, x=0.0, y=0.0, width=640, height=480, active=False, name="Camera"):
        super().__init__(name)
        self.width = width
        self.height = height
        self.active = active
    def update(self, dt):
        pass
cam_mod.Camera = Camera
sys.modules.setdefault('engine.core.camera', cam_mod)

from engine.core.engine import Engine
from engine.core.scenes.scene import Scene
from engine.inputs import InputBackend

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

def test_engine_step(monkeypatch):
    calls = []
    class TestEngine(Engine):
        def update(self, dt):
            super().update(dt)
            calls.append(dt)
    times = iter([0.0, 0.016, 0.033, 0.049, 0.066])
    monkeypatch.setattr(time, "perf_counter", lambda: next(times))
    slept = []
    monkeypatch.setattr(time, "sleep", lambda t: slept.append(round(t, 3)))
    scene = Scene(with_defaults=False)
    eng = TestEngine(fps=30, scene=scene, renderer=NullRendererStub, input_backend=DummyInput)
    eng.step()
    eng.step()
    assert slept and all(s > 0 for s in slept)
    assert pytest.approx(calls[0], 1e-3) == 0.033
    assert pytest.approx(calls[1], 1e-3) == 0.033


def test_step_clamps_delta(monkeypatch):
    calls = []

    class TestEngine(Engine):
        def update(self, dt):
            calls.append(dt)

    times = iter([0.0, 1.0])
    monkeypatch.setattr(time, "perf_counter", lambda: next(times))
    monkeypatch.setattr(time, "sleep", lambda t: None)
    scene = Scene(with_defaults=False)
    eng = TestEngine(fps=0, scene=scene, renderer=NullRendererStub, input_backend=DummyInput, max_delta=0.2)
    eng.step()
    assert pytest.approx(calls[0], 1e-3) == 0.2


