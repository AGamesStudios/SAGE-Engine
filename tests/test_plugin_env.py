import types

import engine.plugins


def test_plugin_dir_env(monkeypatch, tmp_path):
    plugin = tmp_path / "plg_mod.py"
    plugin.write_text("def init_engine(engine): engine.flag = True")
    monkeypatch.setenv("SAGE_PLUGIN_DIR", str(tmp_path))
    manager = engine.plugins.PluginManager("engine")
    obj = types.SimpleNamespace()
    manager.load(obj)
    assert getattr(obj, "flag", False)


def test_engine_plugins_env(monkeypatch, tmp_path):
    plugin = tmp_path / "env_engine.py"
    plugin.write_text("def init_engine(engine): engine.flag = True")
    monkeypatch.setenv("SAGE_ENGINE_PLUGINS", str(tmp_path))
    manager = engine.plugins.PluginManager("engine")
    ns = types.SimpleNamespace()
    manager.load(ns)
    assert getattr(ns, "flag", False)


def test_editor_plugins_env(monkeypatch, tmp_path):
    plugin = tmp_path / "env_editor.py"
    plugin.write_text("def init_editor(editor): editor.flag = True")
    monkeypatch.setenv("SAGE_EDITOR_PLUGINS", str(tmp_path))
    manager = engine.plugins.PluginManager("editor")
    ed = types.SimpleNamespace()
    manager.load(ed)
    assert getattr(ed, "flag", False)
