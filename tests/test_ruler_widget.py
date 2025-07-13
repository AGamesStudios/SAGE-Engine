import importlib
from tests.test_viewport_sync import _setup_qt


def test_ruler_set_transform(monkeypatch):
    _setup_qt(monkeypatch)
    mod = importlib.import_module('sage_editor.widgets.ruler_widget')
    r = mod.RulerWidget(getattr(mod.Qt.Orientation, 'Horizontal', 1))
    r.set_transform(5.0, 2.0, 3.0)
    assert r.offset == 5.0
    assert r.scale == 2.0
    assert r.cursor == 3.0
    assert r.sign == 1


def test_ruler_vertical_sign(monkeypatch):
    _setup_qt(monkeypatch)
    mod = importlib.import_module('sage_editor.widgets.ruler_widget')
    r = mod.RulerWidget(getattr(mod.Qt.Orientation, 'Vertical', 2))
    r.set_transform(-1.0, 1.5, None, sign=-1)
    assert r.offset == -1.0
    assert r.scale == 1.5
    assert r.cursor is None
    assert r.sign == -1
