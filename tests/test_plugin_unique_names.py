import types
from sage_engine.plugins import PluginManager


def test_plugins_same_filename(tmp_path):
    d1 = tmp_path / "d1"
    d2 = tmp_path / "d2"
    d1.mkdir()
    d2.mkdir()
    (d1 / "dup.py").write_text(
        "def init_engine(engine): engine.one = True"
    )
    (d2 / "dup.py").write_text(
        "def init_engine(engine): engine.two = True"
    )
    obj = types.SimpleNamespace()
    mgr = PluginManager("engine", plugin_dir=str(d1))
    mgr.load(obj, paths=[str(d2)])
    assert getattr(obj, "one", False) and getattr(obj, "two", False)


def test_plugin_hyphen_name(tmp_path):
    plugin = tmp_path / "my-plugin.py"
    plugin.write_text("def init_engine(engine): engine.hy = True")
    obj = types.SimpleNamespace()
    PluginManager("engine", plugin_dir=str(tmp_path)).load(obj)
    assert getattr(obj, "hy", False)

