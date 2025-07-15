import importlib.util
from pathlib import Path
from tests.test_viewport_sync import _setup_qt
from tests.test_opengl_tilemap import _stub_gl


def test_shape_and_sprite_groups(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        "viewport", Path("src/sage_engine/editor/plugins/viewport.py")
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert "shape" in [win.properties.role_combo._items[i] for i in range(win.properties.role_combo.count())]
    assert hasattr(win.properties, "shape_group")
    assert hasattr(win.properties, "sprite_group")

def test_properties_blank_when_none(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        "viewport", Path("src/sage_engine/editor/plugins/viewport.py")
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    win.select_object(None)
    assert not win.properties.object_group.isVisible()
    assert not win.properties.transform_group.isVisible()

def test_role_change_updates_groups(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        "viewport", Path("src/sage_engine/editor/plugins/viewport.py")
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    obj.role = "shape"
    win.select_object(obj)
    assert win.properties.shape_group.isVisible()
    assert not win.properties.sprite_group.isVisible()

    # change role to sprite via combo box
    items = win.properties.role_combo._items
    idx = items.index("sprite") if "sprite" in items else 0
    win.properties.role_combo.setCurrentIndex(idx)
    assert win.properties.sprite_group.isVisible()
    assert not win.properties.shape_group.isVisible()

