import importlib.util
from pathlib import Path
from tests.test_opengl_tilemap import _stub_gl
from tests.test_viewport_sync import _setup_qt
from sage_engine.renderers import register_renderer, Renderer

class DummySDL(Renderer):
    def __init__(self, width=0, height=0, title=None, widget=None, vsync=False, keep_aspect=False):
        super().__init__()
        self.width = width
        self.height = height
        self.widget = widget
        self.keep_aspect = keep_aspect
    def clear(self, color=(0,0,0)):
        pass
    def draw_scene(self, scene, camera=None):
        pass
    def present(self):
        pass
    def close(self):
        pass


def test_change_renderer(monkeypatch):
    calls = {}
    _stub_gl(monkeypatch, calls)
    _setup_qt(monkeypatch)

    register_renderer('sdl', DummySDL)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_engine/editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    monkeypatch.setattr('engine.renderers.get_renderer',
                        lambda name: DummySDL if name == 'sdl' else viewport.OpenGLRenderer)

    win = viewport.EditorWindow()
    assert win.renderer_backend == 'opengl'
    win.change_renderer('sdl')
    assert win.renderer_backend == 'sdl'
    assert isinstance(win.renderer, DummySDL)

