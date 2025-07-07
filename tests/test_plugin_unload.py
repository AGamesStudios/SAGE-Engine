import os
import types
from engine.plugins import PluginManager


def test_plugin_unload_and_reload(tmp_path):
    plugin = tmp_path / "mod.py"
    plugin.write_text("value=1\ndef init_engine(e): e.val=value")
    mgr = PluginManager("engine", plugin_dir=str(tmp_path))
    obj = types.SimpleNamespace()
    mgr.load(obj)
    assert obj.val == 1

    plugin.write_text("value=2\ndef init_engine(e): e.val=value")
    ts = os.stat(plugin).st_mtime + 2
    os.utime(plugin, (ts, ts))
    mgr.unload_all()
    obj2 = types.SimpleNamespace()
    mgr.load(obj2)
    assert obj2.val == 2
