import importlib.util
from pathlib import Path

from tests.test_viewport_sync import _setup_qt
from tests.test_opengl_tilemap import _stub_gl


def test_properties_physics_group(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        "viewport", Path("src/sage_editor/plugins/viewport.py")
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert hasattr(win.properties, "physics_group")
    assert win.properties.body_combo.count() == 2
