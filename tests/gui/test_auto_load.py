import importlib

import sage_engine.gui as gui


def test_auto_load(monkeypatch, capsys):
    monkeypatch.setenv("SAGE_GUI", "tk")
    importlib.reload(gui)
    backend = gui.get_backend()
    captured = capsys.readouterr().out
    assert backend.__class__.__name__ == "TkBackend"
    assert "GUI backend: tk" in captured
