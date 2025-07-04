import importlib
import types

import engine.plugins


def test_plugin_dir_env(monkeypatch, tmp_path):
    plugin = tmp_path / "plg_mod.py"
    plugin.write_text("def init_engine(engine): engine.flag = True")
    monkeypatch.setenv("SAGE_PLUGIN_DIR", str(tmp_path))
    importlib.reload(engine.plugins)
    manager = engine.plugins.PluginManager("engine")
    obj = types.SimpleNamespace()
    manager.load(obj)
    assert getattr(obj, "flag", False)
