import types
from pathlib import Path
import sys

from tests.test_viewport_sync import _setup_qt
from tests.test_opengl_tilemap import _stub_gl


def test_close_event_calls_renderer(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)
    import PyQt6.QtWidgets as qtwidgets
    qtwidgets.QMainWindow.closeEvent = lambda self, event: None
    monkeypatch.delitem(sys.modules, 'engine.renderers', raising=False)
    import importlib
    importlib.import_module('engine.renderers')
    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_engine/editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    closed = []
    class DummyRenderer:
        def draw_scene(self, *a, **k):
            pass
        def close(self):
            closed.append(True)

    monkeypatch.setattr(viewport, 'OpenGLRenderer', lambda **k: DummyRenderer())
    monkeypatch.setattr(viewport.EditorWindow, 'close_game', lambda self: closed.append('game'))

    win = viewport.EditorWindow()
    event = types.SimpleNamespace(accept=lambda: None)
    win.closeEvent(event)
    assert True in closed
    assert 'game' in closed
