import types
import sys

# Stub heavy modules so engine imports
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))

qtcore = types.ModuleType('PyQt6.QtCore')
qtcore.Qt = object()
qtcore.QObject = type('QObject', (), {})
sys.modules.setdefault('PyQt6', types.ModuleType('PyQt6'))
sys.modules['PyQt6.QtCore'] = qtcore
sys.modules['PyQt6.QtWidgets'] = types.ModuleType('PyQt6.QtWidgets')

from engine.core.engine import Engine  # noqa: E402
from engine.core.scenes.scene import Scene  # noqa: E402
from engine.inputs import InputBackend  # noqa: E402


class DummyRenderer:
    def __init__(self, *a):
        self.widget = None
        self.closed = False
    def should_close(self):
        return True
    def draw_scene(self, *a, **k):
        pass
    def present(self):
        pass
    def close(self):
        self.closed = True


class DummyInput(InputBackend):
    def __init__(self, widget=None):
        self.shutdown_called = False
    def poll(self):
        pass
    def is_key_down(self, key):
        return False
    def is_button_down(self, button):
        return False
    def shutdown(self):
        self.shutdown_called = True


class DemoEngine(Engine):
    def __init__(self, *a, **kw):
        super().__init__(*a, **kw)
        self.cleaned = False
    def shutdown(self):
        super().shutdown()
        self.cleaned = True


def test_fallback_cleanup(monkeypatch):
    render_mod = types.ModuleType('engine.renderers')
    render_mod.Renderer = type('Renderer', (), {})
    render_mod.Shader = object
    render_mod.get_renderer = lambda name: None
    render_mod.__package__ = 'engine'
    render_mod.__spec__ = types.SimpleNamespace(loader=None)
    monkeypatch.setitem(sys.modules, 'engine.renderers', render_mod)
    monkeypatch.setitem(sys.modules, 'engine.renderers.shader', types.ModuleType('engine.renderers.shader'))
    monkeypatch.setitem(sys.modules, 'PyQt6.QtWidgets', None)
    scene = Scene(with_defaults=False)
    eng = DemoEngine(scene=scene, renderer=DummyRenderer, input_backend=DummyInput)
    eng.run()
    assert eng.cleaned
    assert eng.input.shutdown_called
    assert eng.renderer.closed


