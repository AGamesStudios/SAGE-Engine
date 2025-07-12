import importlib.util
from pathlib import Path
from tests.test_viewport_sync import _setup_qt
from tests.test_opengl_tilemap import _stub_gl


def test_shape_and_sprite_groups(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        "viewport", Path("src/sage_editor/plugins/viewport.py")
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert "shape" in [win.properties.role_combo._items[i] for i in range(win.properties.role_combo.count())]
    assert hasattr(win.properties, "shape_group")
    assert hasattr(win.properties, "sprite_group")
