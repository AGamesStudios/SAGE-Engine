import types
from pathlib import Path
import sys

from tests.test_viewport_sync import _setup_qt
from tests.test_opengl_tilemap import _stub_gl


def test_start_game_twice(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)
    import PyQt6.QtWidgets as qtwidgets
    qtwidgets.QMainWindow.closeEvent = lambda self, event: None
    monkeypatch.delitem(sys.modules, 'engine.renderers', raising=False)
    import importlib
    importlib.import_module('engine.renderers')
    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    engines = []
    class DummyEngine:
        def __init__(self):
            engines.append(self)
            self.renderer = types.SimpleNamespace(close=lambda: None)
        def shutdown(self):
            self.stopped = True

    windows = []
    class DummyGameWindow:
        def __init__(self, engine):
            self.engine = engine
            self.closed = types.SimpleNamespace(connect=lambda cb: setattr(self, 'cb', cb))
            windows.append(self)
        def show(self):
            pass
        def close(self):
            if hasattr(self, 'cb'):
                self.cb()
            windows.remove(self)

    monkeypatch.setattr('engine.core.engine.Engine', DummyEngine)
    monkeypatch.setattr('engine.game_window.GameWindow', DummyGameWindow)

    win = viewport.EditorWindow()
    win.start_game()
    assert len(windows) == 1
    first = win._engine
    win.start_game()
    assert len(windows) == 1
    assert win._engine is not first
    assert getattr(first, 'stopped', False)
