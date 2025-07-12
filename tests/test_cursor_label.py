import importlib
import importlib.util
import sys
from pathlib import Path

from tests.test_opengl_tilemap import _stub_gl
from tests.test_viewport_sync import _setup_qt


def test_cursor_label(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)
    monkeypatch.delitem(sys.modules, 'engine.renderers', raising=False)
    importlib.import_module('engine.renderers')
    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)
    win = viewport.EditorWindow()
    win.update_cursor(1.5, -2.5)
    label = win.cursor_label
    text = getattr(label, 'text', None)
    if callable(text):
        text = text()
    assert '1.5' in text and '-2.5' in text
