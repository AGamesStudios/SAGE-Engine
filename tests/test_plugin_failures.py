import logging
from importlib import metadata

# helper entry point list
class _EP:
    def __init__(self, name):
        self.name = name
    def load(self):
        raise RuntimeError('boom')
class _EPS(list):
    def __init__(self, group):
        super().__init__([_EP('bad1'), _EP('bad2')])
        self.group = group
    def select(self, *, group):
        return self if group == self.group else []

def test_object_plugin_warning(monkeypatch, caplog):
    from engine.core import objects
    caplog.set_level(logging.WARNING)
    eps = _EPS('sage_engine.objects')
    monkeypatch.setattr(metadata, 'entry_points', lambda: eps)
    objects._PLUGINS_LOADED = False
    objects.load_object_plugins()
    assert 'Failed object plugins: bad1, bad2' in caplog.text

def test_input_plugin_warning(monkeypatch, caplog):
    from engine import inputs
    caplog.set_level(logging.WARNING)
    eps = _EPS('sage_engine.inputs')
    monkeypatch.setattr(metadata, 'entry_points', lambda: eps)
    inputs._PLUGINS_LOADED = False
    inputs._load_entry_points()
    assert 'Failed input plugins: bad1, bad2' in caplog.text

def test_renderer_plugin_warning(monkeypatch, caplog):
    from engine import renderers
    caplog.set_level(logging.WARNING)
    eps = _EPS('sage_engine.renderers')
    monkeypatch.setattr(metadata, 'entry_points', lambda: eps)
    renderers.RENDERER_REGISTRY.clear()
    renderers._PLUGINS_LOADED = False
    renderers._load_entry_points()
    assert 'Failed renderer plugins: bad1, bad2' in caplog.text
