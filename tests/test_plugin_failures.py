import logging
from importlib import metadata
import pytest

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
    with pytest.raises(RuntimeError):
        objects.load_object_plugins()
    assert 'Failed object plugins: bad1, bad2' in caplog.text

def test_input_plugin_error(monkeypatch, caplog):
    from engine import inputs
    caplog.set_level(logging.WARNING)
    eps = _EPS('sage_engine.inputs')
    monkeypatch.setattr(metadata, 'entry_points', lambda: eps)
    inputs._PLUGINS_LOADED = False
    with pytest.raises(RuntimeError):
        inputs._load_entry_points()
    assert 'Failed input plugins: bad1, bad2' in caplog.text

def test_renderer_plugin_error(monkeypatch, caplog):
    from engine import renderers
    caplog.set_level(logging.WARNING)
    eps = _EPS('sage_engine.renderers')
    monkeypatch.setattr(metadata, 'entry_points', lambda: eps)
    renderers.RENDERER_REGISTRY.clear()
    renderers._PLUGINS_LOADED = False
    with pytest.raises(RuntimeError):
        renderers._load_entry_points()
    assert 'Failed renderer plugins: bad1, bad2' in caplog.text


def test_engine_plugin_error(monkeypatch):
    from engine.core.scenes.scene import Scene
    from engine.core.engine import Engine
    from engine.renderers.null_renderer import NullRenderer
    from engine.inputs.null_input import NullInput

    def bad_loader(engine, paths=None):
        raise RuntimeError('boom')

    monkeypatch.setattr('engine.load_engine_plugins', bad_loader)

    with pytest.raises(RuntimeError):
        Engine(scene=Scene(with_defaults=False), renderer=NullRenderer,
               input_backend=NullInput)


def test_logic_plugin_error(monkeypatch):
    from engine.logic import base as logic_base
    import builtins

    orig_import = builtins.__import__

    def fake_import(name, *a, **k):
        if name == 'bad_logic':
            raise RuntimeError('boom')
        return orig_import(name, *a, **k)

    monkeypatch.setattr(builtins, '__import__', fake_import)
    with pytest.raises(RuntimeError):
        logic_base.load_logic_plugins('bad_logic')


def test_print_action_skips_none(caplog):
    from engine.logic.actions.print import Print
    from engine.core.scenes.scene import Scene
    from engine.core.engine import Engine
    from engine.renderers.null_renderer import NullRenderer
    from engine.inputs.null_input import NullInput

    scene = Scene(with_defaults=False)
    eng = Engine(scene=scene, renderer=NullRenderer, input_backend=NullInput)
    caplog.set_level(logging.INFO)
    caplog.clear()
    Print(None).execute(eng, scene, 0.0)
    assert caplog.text == ""
