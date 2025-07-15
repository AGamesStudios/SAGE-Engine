import importlib

import sage_engine.render as render


def test_auto_load(monkeypatch, capsys):
    monkeypatch.setenv("SAGE_RENDER", "headless")
    importlib.reload(render)
    backend = render.get_backend()
    captured = capsys.readouterr().out
    assert backend.__class__.__name__ == "HeadlessBackend"
    assert "Render backend: headless" in captured
