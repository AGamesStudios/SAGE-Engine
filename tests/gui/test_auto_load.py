import importlib

import sage_engine.gui as gui


def test_auto_load(monkeypatch):
    monkeypatch.setenv("SAGE_GUI", "tk")
    importlib.reload(gui)
    backend = gui.get_backend()
    assert backend.__class__.__name__ == "TkBackend"
