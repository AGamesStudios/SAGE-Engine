import importlib

from tests.test_viewport_sync import _setup_qt


def test_tag_field_basic(monkeypatch):
    _setup_qt(monkeypatch)
    tag_field = importlib.import_module('sage_editor.widgets.tag_field')

    w = tag_field.TagField()
    w.set_tags(['a', 'b'])
    assert w.tags() == ['a', 'b']
    w.add_tag('c')
    assert 'c' in w.tags()
    w.remove_tag('a')
    assert 'a' not in w.tags()
    w.add_tag('d')
    w.add_tag('e')
    w.add_tag('f')
    assert len(w.tags()) == 5

