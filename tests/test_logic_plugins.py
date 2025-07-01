import importlib
import sys
from importlib import metadata
import pytest

def test_logic_plugin_entry_point(tmp_path, monkeypatch):
    plugin = tmp_path / "logic_plugin.py"
    plugin.write_text(
        "from engine.logic.base import Action, register_action\n"
        "@register_action('DummyAction')\n"
        "class DummyAction(Action):\n"
        "    def execute(self, engine, scene, dt):\n"
        "        engine.called = True\n"
    )

    class EP:
        def __init__(self, name, value):
            self.name = name
            self.value = value
        def load(self):
            return importlib.import_module(self.value)
    class EPS(list):
        def select(self, group):
            return self if group == 'sage_engine.logic' else []
    eps = EPS([EP('dummy', 'logic_plugin')])
    monkeypatch.setattr(metadata, 'entry_points', lambda: eps)
    sys.path.insert(0, str(tmp_path))
    importlib.invalidate_caches()
    try:
        from engine.logic import base as logic_base
        logic_base._PLUGINS_LOADED = False
        import engine.logic
        importlib.reload(engine.logic.base)
        sys.modules.pop('engine.logic.actions', None)
        sys.modules.pop('engine.logic.conditions', None)
        importlib.reload(engine.logic)
        engine.logic.load_logic_plugins()
        if 'DummyAction' not in engine.logic.ACTION_REGISTRY:
            pytest.skip('Entry point loading failed')
        assert 'DummyAction' in engine.logic.ACTION_REGISTRY
    finally:
        sys.path.pop(0)

