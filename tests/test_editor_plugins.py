from sage_editor import Editor
from engine.plugins import PluginManager


def test_editor_plugin_loading(tmp_path):
    plugin = tmp_path / "plg.py"
    plugin.write_text("def init_editor(editor): editor.loaded = True")
    ed = Editor()
    ed.plugins = PluginManager("editor", plugin_dir=str(tmp_path))
    ed.load_plugins()
    assert getattr(ed, "loaded", False)

