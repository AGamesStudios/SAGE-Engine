import types
from sage_sdk.plugins import PluginManager, PluginBase

def test_load_module_plugin(tmp_path):
    p = tmp_path / "p1.py"
    p.write_text("def init_engine(engine): engine.flag = True")
    manager = PluginManager('engine', plugin_dir=str(tmp_path))
    engine = types.SimpleNamespace()
    manager.load(engine)
    assert getattr(engine, 'flag', False)

class MyPlugin(PluginBase):
    def init_engine(self, engine):
        engine.value = 42


def test_load_object_plugin(tmp_path):
    p = tmp_path / "p2.py"
    p.write_text(
        "from sage_sdk.plugins import PluginBase\n"
        "class P(PluginBase):\n"
        "    def init_engine(self, engine):\n"
        "        engine.v = 1\n"
        "plugin = P()\n"
    )
    manager = PluginManager('engine', plugin_dir=str(tmp_path))
    obj = types.SimpleNamespace()
    manager.load(obj)
    assert getattr(obj, 'v', 0) == 1
