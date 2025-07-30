from pathlib import Path
from sage_engine.plugins import load_plugins, register, unregister


class Dummy:
    def __init__(self):
        self.attached = False
        self.closed = False

    def on_attach(self, engine):
        self.attached = True

    def on_shutdown(self):
        self.closed = True


def test_load_plugins(tmp_path):
    plugin_file = tmp_path / "sample.py"
    plugin_file.write_text("x = 1", encoding='utf8')
    loaded = load_plugins(tmp_path)
    assert 'sample' in loaded


def test_register_unregister():
    d = Dummy()
    register(d)
    assert d.attached
    unregister(d)
    assert d.closed
