import importlib
import pytest
from sage_engine.core.engine import Engine
from sage_engine.core.settings import EngineSettings
from sage_engine.inputs import InputBackend
from sage_engine.renderers import Renderer

missing = []
for name in ["PIL.Image", "PyQt6", "OpenGL.GL"]:
    try:
        importlib.import_module(name)
    except Exception:
        missing.append(name)
if missing:
    pytest.xfail("missing deps: " + ", ".join(missing))

class DummyRenderer(Renderer):
    def __init__(self, width, height, title):
        self.width = width
        self.height = height
        self.title = title
        self.widget = None
        self.keep_aspect = True
        self.background = (0, 0, 0)

    def clear(self, color=(0, 0, 0)):
        pass

    def draw_scene(self, scene, camera=None):
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


def test_engine_from_settings():
    settings = EngineSettings(width=320, height=240, fps=60,
                              renderer=DummyRenderer,
                              input_backend=DummyInput,
                              vsync=True)
    engine = Engine(settings=settings)
    assert engine.fps == 60
    assert engine.vsync is True
    assert isinstance(engine.renderer, DummyRenderer)
    assert isinstance(engine.input, DummyInput)
    assert engine.max_delta == settings.max_delta
