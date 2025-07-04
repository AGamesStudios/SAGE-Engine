import types
import pytest
from engine.plugins import PluginManager


def test_plugin_rejects_outside_relative(tmp_path):
    outside = tmp_path / "evil.py"
    outside.write_text("def init_engine(engine): engine.bad = True")
    plugins = tmp_path / "plugins"
    plugins.mkdir()
    link = plugins / "rel.py"
    try:
        link.symlink_to("../evil.py")
    except (OSError, NotImplementedError):
        pytest.skip("symlink unsupported")

    obj = types.SimpleNamespace()
    PluginManager("engine", plugin_dir=str(plugins)).load(obj)
    assert not hasattr(obj, "bad")

