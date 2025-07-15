import os
import types
from pathlib import Path
import shutil

import sage_engine as engine
import sage_engine.plugins
import sage_engine.utils.config as cfgmod


def test_plugin_dir_env(monkeypatch, tmp_path):
    plugin = tmp_path / "plg_mod.py"
    plugin.write_text("def init_engine(engine): engine.flag = True")
    monkeypatch.setenv("SAGE_PLUGIN_DIR", str(tmp_path))
    import sage_engine.utils.config as cfgmod
    cfgmod._config_cache = None
    manager = engine.plugins.PluginManager("engine")
    obj = types.SimpleNamespace()
    manager.load(obj)
    assert getattr(obj, "flag", False)


def test_engine_plugin_dir_env(monkeypatch, tmp_path):
    plugin = tmp_path / "env_dir.py"
    plugin.write_text("def init_engine(e): e.d=True")
    monkeypatch.setenv("SAGE_ENGINE_PLUGIN_DIR", str(tmp_path))
    cfgmod._config_cache = None
    mgr = engine.plugins.PluginManager("engine")
    obj = types.SimpleNamespace()
    mgr.load(obj)
    assert getattr(obj, "d", False)


def test_editor_plugin_dir_env(monkeypatch, tmp_path):
    plugin = tmp_path / "env_dir_ed.py"
    plugin.write_text("def init_editor(e): e.d=True")
    monkeypatch.setenv("SAGE_EDITOR_PLUGIN_DIR", str(tmp_path))
    cfgmod._config_cache = None
    mgr = engine.plugins.PluginManager("editor")
    obj = types.SimpleNamespace()
    mgr.load(obj)
    assert getattr(obj, "d", False)


def test_plugin_deduped_dirs(monkeypatch, tmp_path):
    plugin = tmp_path / "mod.py"
    plugin.write_text("def init_engine(e): e.count = getattr(e,'count',0) + 1")
    paths = os.pathsep.join([str(tmp_path), str(tmp_path)])
    monkeypatch.setenv("SAGE_ENGINE_PLUGINS", paths)
    mgr = engine.plugins.PluginManager("engine", plugin_dir=str(tmp_path))
    obj = types.SimpleNamespace()
    mgr.load(obj)
    assert getattr(obj, "count", 0) == 1


def test_engine_plugins_env(monkeypatch, tmp_path):
    plugin = tmp_path / "env_engine.py"
    plugin.write_text("def init_engine(engine): engine.flag = True")
    monkeypatch.setenv("SAGE_ENGINE_PLUGINS", str(tmp_path))
    import sage_engine.utils.config as cfgmod
    cfgmod._config_cache = None
    manager = engine.plugins.PluginManager("engine")
    ns = types.SimpleNamespace()
    manager.load(ns)
    assert getattr(ns, "flag", False)


def test_editor_plugins_env(monkeypatch, tmp_path):
    plugin = tmp_path / "env_editor.py"
    plugin.write_text("def init_editor(editor): editor.flag = True")
    monkeypatch.setenv("SAGE_EDITOR_PLUGINS", str(tmp_path))
    cfgmod._config_cache = None
    manager = engine.plugins.PluginManager("editor")
    ed = types.SimpleNamespace()
    manager.load(ed)
    assert getattr(ed, "flag", False)


def test_engine_plugins_override(monkeypatch, tmp_path):
    d1 = tmp_path / "d1"
    d2 = tmp_path / "d2"
    d1.mkdir()
    d2.mkdir()
    (d1 / "a.py").write_text("def init_engine(engine): engine.a=True")
    (d2 / "b.py").write_text("def init_engine(engine): engine.b=True")
    manager = engine.plugins.PluginManager("engine", plugin_dir=str(d1))
    monkeypatch.setenv("SAGE_ENGINE_PLUGINS", str(d2))
    cfgmod._config_cache = None
    ns = types.SimpleNamespace()
    manager.load(ns)
    assert hasattr(ns, "b") and not hasattr(ns, "a")


def test_expanduser_env(monkeypatch, tmp_path):
    home = Path.home()
    test_dir = home / "plug_home_test"
    test_dir.mkdir(exist_ok=True)
    (test_dir / "u.py").write_text("def init_engine(e): e.x=True")
    monkeypatch.setenv("SAGE_PLUGIN_DIR", f"~/{test_dir.name}")
    import sage_engine.utils.config as cfgmod
    cfgmod._config_cache = None
    manager = engine.plugins.PluginManager("engine")
    obj = types.SimpleNamespace()
    manager.load(obj)
    assert getattr(obj, "x", False)
    shutil.rmtree(test_dir)


def test_expanduser_engine_env(monkeypatch, tmp_path):
    home = Path.home()
    test_dir = home / "plug_home_test2"
    test_dir.mkdir(exist_ok=True)
    (test_dir / "u2.py").write_text("def init_engine(e): e.y=True")
    monkeypatch.setenv("SAGE_ENGINE_PLUGINS", f"~/{test_dir.name}")
    cfgmod._config_cache = None
    mgr = engine.plugins.PluginManager("engine")
    obj = types.SimpleNamespace()
    mgr.load(obj)
    assert getattr(obj, "y", False)
    shutil.rmtree(test_dir)
