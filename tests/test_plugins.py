import types
import pytest
from sage_engine.plugins import PluginManager, PluginBase

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
        "from sage_engine.plugins import PluginBase\n"
        "class P(PluginBase):\n"
        "    def init_engine(self, engine):\n"
        "        engine.v = 1\n"
        "plugin = P()\n"
    )
    manager = PluginManager('engine', plugin_dir=str(tmp_path))
    obj = types.SimpleNamespace()
    manager.load(obj)
    assert getattr(obj, 'v', 0) == 1


def test_symlink_outside_dir(tmp_path):
    outside = tmp_path / "outside.py"
    outside.write_text("flag=True")
    plugins = tmp_path / "pdir"
    plugins.mkdir()
    link = plugins / "evil.py"
    try:
        link.symlink_to(outside)
    except (OSError, NotImplementedError):
        pytest.xfail("symlink unsupported")
    manager = PluginManager('engine', plugin_dir=str(plugins))
    ns = types.SimpleNamespace()
    manager.load(ns)
    assert not hasattr(ns, 'flag')


def test_async_module_plugin(tmp_path):
    p = tmp_path / "async_mod.py"
    p.write_text(
        "import asyncio\n"
        "async def init_engine(engine):\n"
        "    engine.async_flag = True\n"
    )
    manager = PluginManager('engine', plugin_dir=str(tmp_path))
    engine = types.SimpleNamespace()
    manager.load(engine)
    assert getattr(engine, 'async_flag', False)


def test_async_object_plugin(tmp_path):
    p = tmp_path / "async_obj.py"
    p.write_text(
        "from sage_engine.plugins import PluginBase\n"
        "class P(PluginBase):\n"
        "    async def init_engine(self, engine):\n"
        "        engine.async_val = 5\n"
        "plugin = P()\n"
    )
    manager = PluginManager('engine', plugin_dir=str(tmp_path))
    engine = types.SimpleNamespace()
    manager.load(engine)
    assert getattr(engine, 'async_val', 0) == 5
