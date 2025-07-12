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


def test_physics_toggle_updates_object(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        "viewport", Path("src/sage_editor/plugins/viewport.py")
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    win.select_object(obj)

    win.properties.physics_enabled.setChecked(True)
    win.properties.body_combo.setCurrentIndex(1)

    assert obj.physics_enabled
    assert obj.body_type == "static"

    win.select_object(None)
    win.select_object(obj)

    assert obj.physics_enabled
    assert obj.body_type == "static"
