from pathlib import Path
from sage_engine.plugins import load_plugins


def test_load_plugins(tmp_path):
    plugin_file = tmp_path / "sample.py"
    plugin_file.write_text("x = 1", encoding='utf8')
    loaded = load_plugins(tmp_path)
    assert 'sample' in loaded
